// TCPConnection.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Network.hpp>
#include <HardwareAbstraction/Devices.hpp>
#include <Utility.hpp>

// in ms
#define TCP_TIMEOUT		1000
#define TCP_MAXWINDOW	8 * 1024


#define FLAG_ACK		0x10
#define FLAG_PUSH		0x08
#define FLAG_RESET		0x04
#define FLAG_SYN		0x02
#define FLAG_FIN		0x01



namespace Kernel {
namespace HardwareAbstraction {
namespace Network {
namespace TCP
{
	enum class ConnectionState
	{
		// connection
		Disconnected,
		WatingSYNReply,

		// in operation
		Connected,


		// disconnection
		DisconnectingWaitForFirstACK,
		DisconnectingCloseWait,
		DisconnectingWaitForLastACK,
		DisconnectingWaitForLastFIN
	};

	TCPConnection::TCPConnection(Socket* skt, uint16_t srcport, uint16_t destport) : packetbuffer(TCP_MAXWINDOW)
	{
		this->socket = skt;
		this->destip6 = {{ 0 }};
		this->sourceip6 = {{ 0 }};
		this->clientport = srcport;
		this->serverport = destport;

		// generate unique UUIDs and sequence numbers.
		this->uuid = Kernel::KernelRandom->Generate64();
		this->localsequence = Kernel::KernelRandom->Generate16();
		this->localcumlsequence = 0;
		this->servercumlsequence = 0;
		this->nextack = 0;
		this->maxsegsize = 1000;
		this->PacketReceived = false;
		this->bufferfill = 0;
		this->lastpackettime = 0;
		this->AlreadyAcked = false;
		this->state = ConnectionState::Disconnected;
	}

	TCPConnection::TCPConnection(Socket* skt, Library::IPv4Address dest, uint16_t srcport, uint16_t destport) : TCPConnection(skt, srcport, destport)
	{
		this->destip4 = dest;
		this->sourceip4 = IP::GetIPv4Address();
	}

	TCPConnection::TCPConnection(Socket* skt, Library::IPv6Address dest, uint16_t srcport, uint16_t destport) : TCPConnection(skt, srcport, destport)
	{
		this->destip6 = dest;
		// this->sourceip6 = 0;
	}



	TCPConnection::~TCPConnection()
	{
		if(this->state != ConnectionState::Disconnected)
			this->Disconnect();

		// flush tcp to application.
		uint8_t* buf = new uint8_t[GLOBAL_MTU];
		this->packetbuffer.Read(buf, GLOBAL_MTU);
		this->socket->recvbuffer.Write(buf, GLOBAL_MTU);

		delete[] buf;
	}










	ConnectionError TCPConnection::Connect()
	{
		// stage 1: send the opening packet.
		// wait for reply for TCP_TIMEOUT miliseconds.
		{
			// flags: SYN.
			uint8_t flags = FLAG_SYN;

			this->state = ConnectionState::WatingSYNReply;
			this->SendPacket(0, 0, flags);

			Log("Trying to connect to %d.%d.%d.%d : %d", this->destip4.b1, this->destip4.b2, this->destip4.b3, this->destip4.b4, this->serverport);

			// wait for reply
			volatile uint64_t future = Time::Now() + TCP_TIMEOUT;
			while(this->serversequence == 0)
			{
				if(Time::Now() > future)
				{
					this->error = ConnectionError::Timeout;
					Log(1, "TCP connection to %d.%d.%d.%d : %d timed out", this->destip4.b1, this->destip4.b2, this->destip4.b3, this->destip4.b4, this->serverport);
					return ConnectionError::Timeout;
				}
			}
		}

		Log("Stage 1 TCP connect complete -- sent SYN, received SYN + ACK");


		// stage 2: server responded. HandleIncoming() would have set the server's seqnum in our object,
		// so we send a corresponding ACK.
		{
			uint8_t flags = FLAG_ACK;
			this->nextack = this->serversequence + 1;

			this->SendPacket(0, 0, flags);

			this->state = ConnectionState::Connected;

			Log("Established connection to %d.%d.%d.%d : %d; Relative local seq: %d, Relative server seq: %d", this->destip4.b1,
				this->destip4.b2, this->destip4.b3, this->destip4.b4, this->serverport, this->localcumlsequence, this->servercumlsequence);
		}

		return ConnectionError::NoError;
	}

	void TCPConnection::Disconnect()
	{
		if(this->state != ConnectionState::Disconnected)
		{
			this->state = ConnectionState::DisconnectingWaitForFirstACK;
			this->SendPacket(0, 0, FLAG_ACK | FLAG_FIN);

			this->localcumlsequence++;
		}
	}







	void TCPConnection::HandleIncoming(uint8_t* packet, uint64_t bytes, uint64_t HeaderSize)
	{
		// the state machine reference:
		// http://www.tcpipguide.com/free/t_TCPOperationalOverviewandtheTCPFiniteStateMachineF-2.htm



		// at this stage the checksum should be verified (and set to zero), so we can ignore that.
		TCPPacket* tcp = (TCPPacket*) packet;
		HeaderSize *= 4;

		// check if we got options.
		if(HeaderSize > sizeof(TCPPacket))
		{
			uint64_t optsize = HeaderSize - sizeof(TCPPacket);
			uint64_t offset = sizeof(TCPPacket);
			while(offset < optsize)
			{
				switch(packet[offset])
				{
					case 2:
						this->maxsegsize = SwapEndian16(*(packet + offset + 2));
						offset += 4;
						break;
				}
			}
		}




		// check if ack.
		if(!(tcp->Flags & FLAG_ACK))
		{
			Log(1, "TCP ACK flag not set, discarding.");
			return;
		}

		uint64_t datalength = bytes - HeaderSize;
		this->lastpackettime = Time::Now();
		this->nextack = SwapEndian32(tcp->sequence) + (uint32_t) datalength;
		this->servercumlsequence += __max(datalength, 1);
		this->PacketReceived = true;

		Log("**** Received packet, datalen %d", datalength);
		switch(this->state)
		{
			case ConnectionState::Disconnected:
			{
				Log(1, "TCP Stack error: Received packet from closed connection");
				break;
			}

			case ConnectionState::WatingSYNReply:
			{
				// check if we're synchronising
				if(tcp->Flags & FLAG_SYN)
				{
					this->serversequence = SwapEndian32(tcp->sequence);
					this->servercumlsequence = 1;
					this->localcumlsequence = 1;
					this->nextack = 1;
				}
				break;
			}




			// next 2 states: if the other end wanted to disconnect
			case ConnectionState::DisconnectingCloseWait:
			{
				Log(1, "Should not be receiving packets while in CLOSE-WAIT state, ignoring!");
				break;
			}

			case ConnectionState::DisconnectingWaitForLastACK:
			{
				// we're done.
				this->state = ConnectionState::Disconnected;
				break;
			}



			// next 2 states: if we initiated the disconnect
			case ConnectionState::DisconnectingWaitForFirstACK:
			{
				// we just sent the initiating FIN, server replies with ACK... or FIN.

				// if we get a FIN, it's the "simultaneous close" scenario: remote end wants to
				// GTFO as much as we do.

				if(tcp->Flags & FLAG_FIN)
				{
					// let's all GTFO.
					// send an ACK to that.
					this->nextack++;
					this->SendPacket(0, 0, FLAG_ACK);
					this->state = ConnectionState::Disconnected;
				}
				else
				{
					// normal, remote end just responded with 'ACK'.
					// give it some time to finish the CLOSE-WAIT process, and expect a FIN.
					this->state = ConnectionState::DisconnectingWaitForLastFIN;
				}

				break;
			}

			case ConnectionState::DisconnectingWaitForLastFIN:
			{
				if(!(tcp->Flags & FLAG_FIN))
				{
					Log(1, "Expected FIN flag to be set while waiting for last FIN. State machine broken, treating as FIN.");
				}

				// ack, and gtfo.
				this->nextack++;
				this->SendPacket(0, 0, FLAG_ACK);
				this->state = ConnectionState::Disconnected;
				break;
			}





			case ConnectionState::Connected:
			{
				// first check if the server wants us to D/C
				if(tcp->Flags & FLAG_FIN)
				{
					Log("Remote end requesting disconnect, sent FIN");

					// damn it, that fucker
					// send an ACK to that.
					this->nextack++;
					this->SendPacket(0, 0, FLAG_ACK);
					this->state = ConnectionState::DisconnectingCloseWait;

					auto timeoutCloser = [](TCPConnection* tcon)
					{
						// todo: use another value?
						SLEEP(TCP_TIMEOUT);

						tcon->SendPacket(0, 0, FLAG_FIN);
						tcon->state = ConnectionState::DisconnectingWaitForLastACK;
					};

					// ugh, casting
					void (*fnTimeoutCloser)(TCPConnection*) = timeoutCloser;
					Multitasking::AddToQueue(Multitasking::CreateKernelThread((void (*)()) fnTimeoutCloser, 1, this));

					break;
				}




				// todo: make this more optimal. set a timer that will automatically send an ACK-only packet
				// to the server if the user does not send their own packet. if they do within this timeframe
				// (like 1 second or whatever the retransmit timer on the server is), then bundle the ACK with that
				// packet.

				// for now, just ACK everything individually.
				// but only if the packet contains actual data.
				if(datalength > 0)
				{
					this->SendPacket(0, 0, FLAG_ACK);
				}



				this->AlreadyAcked = false;

				// copy the packet data to the correct place.
				if(datalength > 0 && tcp->Flags & FLAG_PUSH)
				{
					// push.
					uint64_t offset = SwapEndian32(tcp->sequence) - this->serversequence - 1;
					this->socket->recvbuffer.MoveWritePointer(offset);
					this->socket->recvbuffer.Write(packet + HeaderSize, datalength);

					Log("pushed %d bytes to offset %d", datalength, offset);
				}
				else if(datalength > 0 && (SwapEndian32(tcp->sequence) >= this->servercumlsequence)
					&& (this->bufferfill + datalength) < TCP_MAXWINDOW)
				{
					uint64_t offset = SwapEndian32(tcp->sequence) - this->serversequence - 1;

					Log("Copying %d bytes to offset %d", datalength, offset);
					this->socket->recvbuffer.MoveWritePointer(offset);
					this->socket->recvbuffer.Write(packet + HeaderSize, datalength);

					this->bufferfill += datalength;
				}
				else if((this->bufferfill + (bytes - HeaderSize)) >= TCP_MAXWINDOW)
				{
					// shit.
					// push data to application,
					// send collective ACK.

					HALT("TCP BUFFER FILLED UP???");

					// flush tcp to application.
					uint8_t* buf = new uint8_t[GLOBAL_MTU];
					this->packetbuffer.Read(buf, GLOBAL_MTU);
					this->socket->recvbuffer.Write(buf, GLOBAL_MTU);

					delete[] buf;
				}
				break;
			}
		}
	}

	void TCPConnection::SendPacket(uint8_t* packet, uint64_t bytes)
	{
		uint8_t flags = FLAG_ACK;
		this->SendPacket(packet, bytes, flags);
	}


	void TCPConnection::SendPacket(uint8_t* packet, uint64_t length, uint8_t flags)
	{
		if(this->sourceip6.high == 0 && this->sourceip6.low == 0 && this->destip6.high == 0 && this->destip6.low == 0)
			this->SendIPv4Packet(packet, length, flags);

		else
			this->SendIPv6Packet(packet, length, flags);
	}


	void TCPConnection::SendUserPacket(uint8_t* packet, uint64_t bytes)
	{
		Log("Sending %d total bytes to %d.%d.%d.%d : %d", bytes, this->destip4.b1, this->destip4.b2, this->destip4.b3, this->destip4.b4, this->serverport);
		uint64_t bytesleft = bytes;
		uint64_t offset = 0;
		while(bytesleft > 0)
		{
			uint64_t min = __min(this->maxsegsize, bytesleft);
			this->SendPacket(packet + offset, min);
			bytesleft -= min;
			offset += min;

			// Log("Sent %d bytes", min);
		}
	}

	void TCPConnection::SendIPv4Packet(uint8_t* packet, uint64_t length, uint8_t flags)
	{
		if(this->state == ConnectionState::Disconnected)
		{
			Log(1, "Error: cannot send packet through a disconnected socket.");
			return;
		}

		// setup a fake IPv4 header.
		IP::PseudoIPv4Header* pseudo = new IP::PseudoIPv4Header;
		pseudo->source = this->sourceip4;
		pseudo->dest = this->destip4;

		pseudo->zeroes = 0;
		pseudo->protocol = (uint8_t) IP::ProtocolType::TCP;
		pseudo->length = SwapEndian16(sizeof(TCPPacket) + (uint16_t) length);

		// calculate the pseudo header's checksum separately.
		uint16_t pseudocheck = IP::CalculateIPChecksum_Partial(0, pseudo, sizeof(IP::PseudoIPv4Header));

		uint8_t* raw = new uint8_t[sizeof(TCPPacket) + length];
		TCPPacket* tcp = (TCPPacket*) raw;
		tcp->clientport = SwapEndian16(this->clientport);
		tcp->serverport = SwapEndian16(this->serverport);

		tcp->sequence = SwapEndian32(this->localsequence + this->localcumlsequence);
		tcp->ackid = SwapEndian32(this->nextack);
		this->localcumlsequence += length;

		tcp->HeaderLength = 0x50;
		tcp->Flags = flags;

		// 32k
		tcp->WindowSize = SwapEndian16(TCP_MAXWINDOW);

		tcp->Checksum = 0;
		tcp->UrgentPointer = 0;

		Memory::Copy(raw + sizeof(TCPPacket), packet, length);
		uint16_t tcpcheck = IP::CalculateIPChecksum_Partial(pseudocheck, tcp, sizeof(TCPPacket) + length);

		tcp->Checksum = SwapEndian16(IP::CalculateIPChecksum_Finalise(tcpcheck));

		IP::SendIPv4Packet(this->socket->interface, raw, (uint16_t) (sizeof(TCPPacket) + length), 48131, this->destip4, IP::ProtocolType::TCP);
		this->AlreadyAcked = true;

		delete pseudo;
		delete tcp;
	}

	void TCPConnection::SendIPv6Packet(uint8_t* packet, uint64_t length, uint8_t flags)
	{
		UNUSED(packet);
		UNUSED(length);
		UNUSED(flags);
	}
}
}
}
}












