// // TCPConnection.cpp
// // Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.

// #include <Kernel.hpp>
// #include <HardwareAbstraction/Network.hpp>
// #include <HardwareAbstraction/Devices.hpp>
// #include <Utility.hpp>

// using namespace Library::StandardIO;

// // in ms
// #define TCP_TIMEOUT		1000
// #define TCP_MAXWINDOW	8 * 1024

// namespace Kernel {
// namespace HardwareAbstraction {
// namespace Network {
// namespace TCP
// {
// 	enum class ConnectionState
// 	{
// 		// connection
// 		Disconnected,
// 		WatingSYNReply,
// 		WaitingConnectionACK,

// 		// in operation
// 		Connected,


// 		// disconnection
// 		WaitingDisconnectFINACK,
// 		WaitingDisconnectACK
// 	};


// 	void TCPAckMonitor(TCPConnection* tcp)
// 	{
// 		uint64_t ppt = tcp->lastpackettime;
// 		// uint8_t* buf = new uint8_t[GLOBAL_MTU];
// 		while(true)
// 		{
// 			while(tcp->lastpackettime == 0  || ppt == tcp->lastpackettime || Time::Now() < tcp->lastpackettime + TCP_TIMEOUT)
// 				YieldCPU();

// 			ppt = tcp->lastpackettime;
// 			// tcp->AlreadyAcked = true;

// 			// flush tcp to application.
// 			tcp->SendPacket(0, 0, 0x10);

// 			// Library::Memory::Set(buf, 0, GLOBAL_MTU);
// 			// tcp->packetbuffer->Read(buf, GLOBAL_MTU);
// 			// tcp->socket->recvbuffer->Write(buf, GLOBAL_MTU);
// 		}
// 	}

// 	TCPConnection::TCPConnection(Socket::SocketObj* skt, uint16_t srcport, uint16_t destport)
// 	{
// 		this->socket = skt;
// 		// this->destip4 = dest;
// 		// this->sourceip4 = *IP::GetIPv4Address();

// 		this->destip6 = {{ 0 }};
// 		this->sourceip6 = {{ 0 }};
// 		this->clientport = srcport;
// 		this->serverport = destport;

// 		// generate unique UUIDs and sequence numbers.
// 		this->uuid = Kernel::KernelRandom->Generate64();
// 		this->localsequence = Kernel::KernelRandom->Generate16();
// 		this->localcumlsequence = 0;
// 		this->servercumlsequence = 0;
// 		this->nextack = 0;
// 		this->maxsegsize = 1000;
// 		this->PacketReceived = false;
// 		this->packetbuffer = new Library::CircularMemoryBuffer(TCP_MAXWINDOW);
// 		this->bufferfill = 0;
// 		this->lastpackettime = 0;
// 		this->AlreadyAcked = false;
// 		this->state = ConnectionState::Disconnected;

// 		Multitasking::Thread* th = Multitasking::CreateKernelThread((void (*)()) (TCPAckMonitor), 1, (void*) (uint64_t) this);
// 		Multitasking::AddToQueue(th);

// 		this->workertid = th->ThreadID;
// 	}

// 	TCPConnection::TCPConnection(Socket::SocketObj* skt, Library::IPv4Address dest, uint16_t srcport, uint16_t destport) : TCPConnection(skt, srcport, destport)
// 	{
// 		this->destip4 = dest;
// 		this->sourceip4 = *IP::GetIPv4Address();
// 	}

// 	TCPConnection::TCPConnection(Socket::SocketObj* skt, Library::IPv6Address dest, uint16_t srcport, uint16_t destport) : TCPConnection(skt, srcport, destport)
// 	{
// 		this->destip6 = dest;
// 		// this->sourceip6 = 0;
// 	}



// 	TCPConnection::~TCPConnection()
// 	{
// 		if(this->state != ConnectionState::Disconnected)
// 			this->Disconnect();

// 		// flush tcp to application.
// 		uint8_t* buf = new uint8_t[GLOBAL_MTU];
// 		this->packetbuffer->Read(buf, GLOBAL_MTU);
// 		this->socket->recvbuffer->Write(buf, GLOBAL_MTU);

// 		delete[] buf;
// 		delete this->packetbuffer;
// 		Multitasking::Kill(Multitasking::GetThread(this->workertid));
// 	}




// 	ConnectionError TCPConnection::Connect()
// 	{
// 		// stage 1: send the opening packet.
// 		// wait for reply for TCP_TIMEOUT miliseconds.
// 		{
// 			// flags: SYN.
// 			uint8_t flags = 0x02;

// 			this->state = ConnectionState::WatingSYNReply;
// 			this->SendPacket(0, 0, flags);

// 			Log("Trying to connect to %d.%d.%d.%d : %d", this->destip4.b1, this->destip4.b2, this->destip4.b3, this->destip4.b4, this->serverport);

// 			// wait for reply
// 			volatile uint64_t future = Time::Now() + TCP_TIMEOUT;
// 			while(this->serversequence == 0)
// 			{
// 				if(Time::Now() > future)
// 				{
// 					this->error = ConnectionError::Timeout;
// 					Log(1, "TCP connection to %d.%d.%d.%d : %d timed out", this->destip4.b1, this->destip4.b2, this->destip4.b3, this->destip4.b4, this->serverport);
// 					return ConnectionError::Timeout;
// 				}
// 			}
// 		}
// 		{
// 			// stage 2: server responded. HandleIncoming() would have set the server's seqnum in our object,
// 			// so we send a corresponding ACK.

// 			uint8_t flags = 0x10;
// 			this->nextack = this->serversequence + 1;

// 			this->state = ConnectionState::WaitingConnectionACK;
// 			this->SendPacket(0, 0, flags);

// 			// make sure we get the ack.
// 			volatile uint64_t future = Time::Now() + TCP_TIMEOUT;
// 			while(!this->PacketReceived)
// 			{
// 				if(Time::Now() > future)
// 				{
// 					this->error = ConnectionError::Timeout;
// 					Log(1, "TCP connection to %d.%d.%d.%d : %d timed out", this->destip4.b1, this->destip4.b2, this->destip4.b3, this->destip4.b4, this->serverport);
// 					return ConnectionError::Timeout;
// 				}
// 			}

// 			this->PacketReceived = false;
// 		}

// 		return ConnectionError::NoError;
// 	}

// 	void TCPConnection::Disconnect()
// 	{
// 		this->state = ConnectionState::WaitingDisconnectACK;
// 		this->SendPacket(0, 0, 0x11);

// 		this->localcumlsequence++;
// 	}







// 	void TCPConnection::HandleIncoming(uint8_t* packet, uint64_t bytes, uint64_t HeaderSize)
// 	{
// 		// at this stage the checksum should be verified (and set to zero), so we can ignore that.
// 		TCPPacket* tcp = (TCPPacket*) packet;
// 		HeaderSize *= 4;

// 		// check if we got options.
// 		if(HeaderSize > sizeof(TCPPacket))
// 		{
// 			uint64_t optsize = HeaderSize - sizeof(TCPPacket);
// 			uint64_t offset = sizeof(TCPPacket);
// 			while(offset < optsize)
// 			{
// 				switch(packet[offset])
// 				{
// 					case 2:
// 						this->maxsegsize = SwapEndian16(*(packet + offset + 2));
// 						offset += 4;
// 						break;
// 				}
// 			}
// 		}




// 		// check if ack.
// 		if(!(tcp->Flags & 0x10))
// 		{
// 			Log("TCP ACK flag not set, discarding.");
// 			return;
// 		}

// 		uint64_t datalength = bytes - HeaderSize;
// 		this->lastpackettime = Time::Now();
// 		this->nextack = SwapEndian32(tcp->sequence) + (uint32_t) datalength;
// 		this->servercumlsequence += math::max(datalength, 1);
// 		this->PacketReceived = true;

// 		switch(this->state)
// 		{
// 			case ConnectionState::Disconnected:
// 			{
// 				Log(1, "TCP Stack error: Received packet from closed connection");
// 				return;
// 			}

// 			case ConnectionState::WatingSYNReply:
// 			{
// 				// check if we're synchronising
// 				if(tcp->Flags & 0x2)
// 				{
// 					this->serversequence = SwapEndian32(tcp->sequence);
// 					this->servercumlsequence = 1;
// 					this->localcumlsequence = 1;
// 					this->nextack = 1;
// 				}
// 				break;
// 			}

// 			case ConnectionState::WaitingConnectionACK:
// 			{
// 				// handshake complete.
// 				Log("Established connection to %d.%d.%d.%d : %d; Relative local seq: %d, Relative server seq: %d", this->destip4.b1, this->destip4.b2, this->destip4.b3, this->destip4.b4, this->serverport, this->localcumlsequence, this->servercumlsequence);

// 				this->state = ConnectionState::Connected;
// 				break;
// 			}



// 			// disconnections:
// 			case ConnectionState::WaitingDisconnectACK:
// 			{
// 				// servers will either send an 'ACK', followed by a 'FIN, then ACK', or a 'FIN, ACK' directly.
// 				// handle both cases.
// 				if(tcp->Flags & 0x1 && tcp->Flags & 0x10)
// 				{
// 					// this is a FIN, ACK
// 					// don't respond.
// 					this->state = ConnectionState::Disconnected;
// 					return;
// 				}
// 				else if(tcp->Flags & 0x10)
// 				{
// 					// server sent an ACK, expect stuff later
// 					this->state = ConnectionState::WaitingDisconnectFINACK;
// 					return;
// 				}
// 				else
// 				{
// 					Log(1, "Invalid TCP response to FIN, ACK; state machine broken.");
// 				}
// 				break;
// 			}

// 			case ConnectionState::WaitingDisconnectFINACK:
// 			{
// 				// well we're expecting a FIN, ACK.
// 				if(tcp->Flags & 0x1 && tcp->Flags & 0x10)
// 				{
// 					this->nextack++;
// 					this->SendPacket(0, 0, 0x10);
// 					this->state = ConnectionState::Disconnected;
// 					Log("Disconnected from %d.%d.%d.%d : %d.", this->destip4.b1, this->destip4.b2, this->destip4.b3, this->destip4.b4, this->serverport);
// 					return;
// 				}
// 				else
// 				{
// 					Log(1, "Invalid TCP response to FIN, ACK; state machine broken.");
// 				}
// 				break;
// 			}





// 			case ConnectionState::Connected:
// 			{
// 				// first check if the server wants us to D/C
// 				if(tcp->Flags & 0x1)
// 				{
// 					// damn it, that fucker
// 					// send an ACK to that.
// 					this->nextack++;
// 					this->SendPacket(0, 0, 0x10);
// 					this->state = ConnectionState::WaitingDisconnectACK;
// 					return;
// 				}

// 				this->AlreadyAcked = false;

// 				// copy the packet data to the correct place.
// 				if(datalength > 0 && tcp->Flags & 0x8)
// 				{
// 					// push.
// 					// fine, bastard.
// 					uint64_t offset = SwapEndian32(tcp->sequence) - this->serversequence - 1;
// 					this->socket->recvbuffer->MoveWritePointer(offset);
// 					this->socket->recvbuffer->Write(packet + HeaderSize, datalength);

// 					Log("pushed %d bytes to offset %d", datalength, offset);
// 				}
// 				else if(datalength > 0 && (SwapEndian32(tcp->sequence) >= this->servercumlsequence) && (this->bufferfill + datalength) < TCP_MAXWINDOW)
// 				{
// 					uint64_t offset = SwapEndian32(tcp->sequence) - this->serversequence - 1;
// 					// uint64_t prp = this->packetbuffer->GetWritePointer();

// 					Log("Copying %d bytes to offset %d", datalength, offset);
// 					// this->packetbuffer->MoveWritePointer(offset);
// 					// this->packetbuffer->Write(packet + HeaderSize, datalength);
// 					this->socket->recvbuffer->MoveWritePointer(offset);
// 					this->socket->recvbuffer->Write(packet + HeaderSize, datalength);

// 					this->bufferfill += datalength;
// 				}
// 				else if((this->bufferfill + (bytes - HeaderSize)) >= TCP_MAXWINDOW)
// 				{
// 					// shit.
// 					// push data to application,
// 					// send collective ACK.

// 					HALT("");

// 					// flush tcp to application.
// 					uint8_t* buf = new uint8_t[GLOBAL_MTU];
// 					this->packetbuffer->Read(buf, GLOBAL_MTU);
// 					this->socket->recvbuffer->Write(buf, GLOBAL_MTU);

// 					delete[] buf;
// 				}
// 				break;
// 			}
// 		}
// 	}

// 	void TCPConnection::SendPacket(uint8_t* packet, uint64_t bytes)
// 	{
// 		uint8_t flags = 0x10;
// 		this->SendPacket(packet, bytes, flags);
// 	}


// 	void TCPConnection::SendPacket(uint8_t* packet, uint64_t length, uint8_t flags)
// 	{
// 		if(this->sourceip6.high == 0 && this->sourceip6.low == 0 && this->destip6.high == 0 && this->destip6.low == 0)
// 			this->SendIPv4Packet(packet, length, flags);

// 		else
// 			this->SendIPv6Packet(packet, length, flags);
// 	}


// 	void TCPConnection::SendUserPacket(uint8_t* packet, uint64_t bytes)
// 	{
// 		Log("Sending %d total bytes to %d.%d.%d.%d : %d", bytes, this->destip4.b1, this->destip4.b2, this->destip4.b3, this->destip4.b4, this->serverport);
// 		uint64_t bytesleft = bytes;
// 		uint64_t offset = 0;
// 		while(bytesleft > 0)
// 		{
// 			uint64_t min = math::min(this->maxsegsize, bytesleft);
// 			this->SendPacket(packet + offset, min);
// 			bytesleft -= min;
// 			offset += min;

// 			// Log("Sent %d bytes", min);
// 		}
// 	}

// 	void TCPConnection::SendIPv4Packet(uint8_t* packet, uint64_t length, uint8_t flags)
// 	{
// 		/*
// 			SYN: The active open is performed by the client sending a SYN to the server. The client sets the segment's sequence number to a random value A.
// 			SYN-ACK: In response, the server replies with a SYN-ACK. The acknowledgment number is set to one more than the received sequence number i.e. A+1, and the sequence number that the server chooses for the packet is another random number, B.
// 			ACK: Finally, the client sends an ACK back to the server. The sequence number is set to the received acknowledgement value i.e. A+1, and the acknowledgement number is set to one more than the received sequence number i.e. B+1.
// 		*/

// 		if(this->state == ConnectionState::Disconnected)
// 		{
// 			Log(1, "Error: cannot send packet through a disconnected socket.");
// 			return;
// 		}

// 		// setup a fake IPv4 header.
// 		IP::PseudoIPv4Header* pseudo = new IP::PseudoIPv4Header;
// 		pseudo->source = this->sourceip4;
// 		pseudo->dest = this->destip4;

// 		pseudo->zeroes = 0;
// 		pseudo->protocol = (uint8_t) IP::ProtocolType::TCP;
// 		pseudo->length = SwapEndian16(sizeof(TCPPacket) + (uint16_t) length);

// 		// calculate the pseudo header's checksum separately.
// 		uint16_t pseudocheck = IP::CalculateIPChecksum_Partial(0, pseudo, sizeof(IP::PseudoIPv4Header));

// 		uint8_t* raw = new uint8_t[sizeof(TCPPacket) + length];
// 		TCPPacket* tcp = (TCPPacket*) raw;
// 		tcp->clientport = SwapEndian16(this->clientport);
// 		tcp->serverport = SwapEndian16(this->serverport);

// 		tcp->sequence = SwapEndian32(this->localsequence + this->localcumlsequence);
// 		tcp->ackid = SwapEndian32(this->nextack);
// 		this->localcumlsequence += length;

// 		tcp->HeaderLength = 0x50;
// 		tcp->Flags = flags;

// 		// 32k
// 		tcp->WindowSize = SwapEndian16(TCP_MAXWINDOW);

// 		tcp->Checksum = 0;
// 		tcp->UrgentPointer = 0;

// 		Library::Memory::Copy(raw + sizeof(TCPPacket), packet, length);
// 		uint16_t tcpcheck = IP::CalculateIPChecksum_Partial(pseudocheck, tcp, sizeof(TCPPacket) + length);

// 		tcp->Checksum = SwapEndian16(IP::CalculateIPChecksum_Finalise(tcpcheck));

// 		// this->socket->WriteTCP(raw, sizeof(TCPPacket) + length);
// 		IP::SendIPv4Packet(Kernel::KernelNIC, raw, (uint16_t) (sizeof(TCPPacket) + length), 48131, this->destip4, IP::ProtocolType::TCP);


// 		this->AlreadyAcked = true;

// 		delete pseudo;
// 		delete tcp;
// 	}

// 	void TCPConnection::SendIPv6Packet(uint8_t* packet, uint64_t length, uint8_t flags)
// 	{
// 		UNUSED(packet);
// 		UNUSED(length);
// 		UNUSED(flags);
// 	}
// }
// }
// }
// }












