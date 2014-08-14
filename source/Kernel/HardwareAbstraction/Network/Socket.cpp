// Socket.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <String.hpp>
#include <StandardIO.hpp>
#include <HardwareAbstraction/Network.hpp>

using namespace Library;
using namespace Kernel::HardwareAbstraction::Filesystems::VFS;

namespace Kernel {
namespace HardwareAbstraction {
namespace Network {
namespace Socket
{
	SocketObj* VerfiySocket(uint64_t ds)
	{
		Multitasking::Process* proc = Multitasking::GetCurrentProcess();
		if(ds >= MaxDescriptors ||  proc->FileDescriptors[ds].Pointer == 0)
		{
			Log(1, "Error: Descriptor %d does not exist, or has not been opened.", ds);
			return 0;
		}

		FSObject* fso = proc->FileDescriptors[ds].Pointer;
		if(fso->Type != FSObjectTypes::Socket)
		{
			Log(1, "Error: Object %s(%d) is not a socket.", fso->Name(), ds);
			return 0;
		}

		return (SocketObj*) fso;
	}



	uint64_t OpenSocket(uint16_t destport, Library::SocketProtocol prot)
	{
		return OpenSocket(IPv4Address { 0 }, IPv4Address { 0xFFFFFFFF }, 0, destport, prot);
	}

	uint64_t OpenSocket(uint16_t sourceport, uint16_t destport, Library::SocketProtocol prot)
	{
		return OpenSocket(IPv4Address { 0 }, IPv4Address { 0xFFFFFFFF }, sourceport, destport, prot);
	}

	uint64_t OpenSocket(IPv4Address dest, uint16_t destport, SocketProtocol prot)
	{
		return OpenSocket(*IP::GetIPv4Address(), dest, 0, destport, prot);
	}

	uint64_t OpenSocket(IPv4Address dest, uint16_t sourceport, uint16_t destport, SocketProtocol prot)
	{
		return OpenSocket(*IP::GetIPv4Address(), dest, sourceport, destport, prot);
	}

	// file descriptor stuff.
	// TODO: clean up duplication
	uint64_t OpenSocket(IPv4Address source, IPv4Address dest, uint16_t sourceport, uint16_t destport, SocketProtocol prot)
	{
		if(prot == SocketProtocol::RawIPv6)
			return 0;

		// make sure the port isn't open somewhere else.
		switch(prot)
		{
			case SocketProtocol::RawIPv4:
				if(IP::ipv4socketmap->Get(SocketIPv4Mapping { source, dest }) != nullptr)
				{
					Log(1, "Socket (%d.%d.%d.%d : %d) already opened by another process.", source.bytes[0], source.bytes[1], source.bytes[2], source.bytes[3], sourceport);
					return 0;
				}
				break;

			case SocketProtocol::TCP:
				if(TCP::tcpsocketmapv4->Get(SocketFullMappingv4 { IPv4PortAddress { source, sourceport }, IPv4PortAddress { dest, destport } }) != nullptr)
				{
					Log(1, "Socket (%d.%d.%d.%d:%d) already opened by another process.", source.bytes[0], source.bytes[1], source.bytes[2], source.bytes[3], sourceport);
					return 0;
				}
				break;

			case SocketProtocol::UDP:
				if(UDP::udpsocketmapv4->Get(SocketFullMappingv4 { IPv4PortAddress { source, sourceport }, IPv4PortAddress { dest, destport } }) != nullptr)
				{
					Log(1, "Socket (%d.%d.%d.%d:%d) already opened by another process.", source.bytes[0], source.bytes[1], source.bytes[2], source.bytes[3], sourceport);
					return 0;
				}
				break;

			default:
				break;
		}


		Multitasking::Process* proc = Multitasking::GetCurrentProcess();
		SocketObj* socket = new SocketObj(source, dest, sourceport, destport, prot);

		proc->FileDescriptors[proc->CurrentFDIndex].Pointer = socket;
		uint64_t ret = proc->CurrentFDIndex;

		// find the next currentfdindex.
		for(uint64_t i = ReservedStreams; i < MaxDescriptors; i++)
		{
			if(proc->FileDescriptors[i].Pointer == 0)
			{
				proc->CurrentFDIndex = i;
				break;
			}
		}

		// ipv4
		switch(prot)
		{
			case Library::SocketProtocol::RawIPv4:
				IP::ipv4socketmap->Put(SocketIPv4Mapping { source, dest }, *socket);
				break;

			case Library::SocketProtocol::TCP:
				TCP::tcpsocketmapv4->Put(SocketFullMappingv4 { IPv4PortAddress { source, sourceport }, IPv4PortAddress { dest, destport } }, *socket);
				break;

			case Library::SocketProtocol::UDP:
				UDP::udpsocketmapv4->Put(SocketFullMappingv4 { IPv4PortAddress { source, sourceport }, IPv4PortAddress { dest, destport } }, *socket);
				break;

			default:
				break;
		}

		Log("Opened socket (%d.%d.%.d.%d : %d => %d.%d.%d.%d : %d) with PID (%d)", source.b1, source.b2, source.b3, source.b4, sourceport, dest.b1, dest.b2,
			dest.b3, dest.b4, destport, proc->ProcessID);
		return ret;
	}





	void CloseSocket(uint64_t ds)
	{
		Multitasking::Process* proc = Multitasking::GetCurrentProcess();
		SocketObj* socket = VerfiySocket(ds);

		switch(socket->protocol)
		{
			case Library::SocketProtocol::RawIPv4:
			{
				IP::ipv4socketmap->Remove(SocketIPv4Mapping { socket->ip4source, socket->ip4dest });
				break;
			}

			case Library::SocketProtocol::RawIPv6:
				break;

			case Library::SocketProtocol::TCP:
			{
				if(socket->ip4source.raw != 0)
					TCP::tcpsocketmapv4->Remove(SocketFullMappingv4 { IPv4PortAddress { socket->ip4source, (uint16_t) socket->clientport },
						IPv4PortAddress { socket->ip4dest, (uint16_t)  socket->serverport } });
				else
					TCP::tcpsocketmapv6->Remove(SocketFullMappingv6 { IPv6PortAddress { socket->ip6source, (uint16_t) socket->clientport },
						IPv6PortAddress { socket->ip6dest, (uint16_t)  socket->serverport } });

				// delete the socket's tcpconnection
				delete socket->tcpconnection;

				break;
			}

			case Library::SocketProtocol::UDP:
			{
				if(socket->ip4source.raw != 0)
					UDP::udpsocketmapv4->Remove(SocketFullMappingv4 { IPv4PortAddress { socket->ip4source, (uint16_t) socket->clientport },
						IPv4PortAddress { socket->ip4dest, (uint16_t)  socket->serverport } });

				else
					UDP::udpsocketmapv6->Remove(SocketFullMappingv6 { IPv6PortAddress { socket->ip6source, (uint16_t) socket->clientport },
						IPv6PortAddress { socket->ip6dest, (uint16_t)  socket->serverport } });

				break;
			}
		}

		// delete the object.
		proc->CurrentFDIndex = ds;
		delete (SocketObj*) proc->FileDescriptors[ds].Pointer;
	}

	uint64_t ReadFromSocket(uint64_t ds, uint8_t* buf, uint64_t bytes)
	{
		SocketObj* skt = VerfiySocket(ds);

		if(skt)
			return skt->Read(buf, bytes);

		else
			return 0;
	}

	uint64_t WriteToSocket(uint64_t ds, uint8_t* buf, uint64_t bytes)
	{
		SocketObj* skt = VerfiySocket(ds);

		if(skt)
			return skt->Write(buf, bytes);

		else
			return 0;
	}

	uint64_t BytesInSocket(uint64_t ds)
	{
		SocketObj* skt = VerfiySocket(ds);

		if(skt)
			return skt->recvbuffer->ByteCount();

		else
			return 0;
	}

	uint64_t PacketsInSocket(uint64_t ds)
	{
		SocketObj* skt = VerfiySocket(ds);

		if(skt)
			return skt->packetcount;

		else
			return 0;
	}

	uint64_t NextPacketSize(uint64_t ds)
	{
		SocketObj* skt = VerfiySocket(ds);

		if(!skt)
			return 0;

		uint64_t psize = 0;
		psize = skt->packetsizes->Read();
		skt->packetsizes->Write(psize);
		return psize;
	}

	void ConnectSocket(uint64_t ds)
	{
		SocketObj* skt = VerfiySocket(ds);

		if(skt)
			skt->tcpconnection->Connect();
	}

	void DisconnectSocket(uint64_t ds)
	{
		SocketObj* skt = VerfiySocket(ds);

		if(skt)
			skt->tcpconnection->Disconnect();
	}














	// actual class implementation
	SocketObj::SocketObj(IPv4Address source, IPv4Address dest, uint64_t spt, uint64_t dpt, Library::SocketProtocol prot) : FSObject(FSObjectTypes::Socket)
	{
		this->ip4source = source;
		this->ip4dest = dest;

		this->ip6source.high = 0;
		this->ip6source.low = 0;
		this->ip6dest.high = 0;
		this->ip6dest.low = 0;

		this->clientport = spt;
		this->serverport = dpt;
		this->protocol = prot;

		this->recvbuffer = new CircularMemoryBuffer(GLOBAL_MTU * 8);
		this->packetsizes = new CircularBuffer<uint64_t>();

		if(prot == SocketProtocol::TCP)
			this->tcpconnection = new TCP::TCPConnection(this, dest, (uint16_t) spt, (uint16_t) dpt);

		this->Type = FSObjectTypes::Socket;
	}

	SocketObj::SocketObj(IPv6Address source, IPv6Address dest, uint64_t spt, uint64_t dpt, Library::SocketProtocol prot) : FSObject(FSObjectTypes::Socket)
	{
		this->ip6source = source;
		this->ip6dest = dest;

		this->ip4source.raw = 0;
		this->ip4dest.raw = 0;

		this->clientport = spt;
		this->serverport = dpt;
		this->protocol = prot;

		this->recvbuffer = new CircularMemoryBuffer(GLOBAL_MTU * 8);
		this->packetsizes = new CircularBuffer<uint64_t>();

		if(prot == SocketProtocol::TCP)
			this->tcpconnection = new TCP::TCPConnection(this, dest, (uint16_t) spt, (uint16_t) dpt);


		this->Type = FSObjectTypes::Socket;
	}

	const char* SocketObj::Name()
	{
		string* s = new string;

		if(this->ip6source.high == 0 && this->ip6source.low == 0)
			StandardIO::PrintToString(s, "%d.%d.%d.%d : %d", this->ip4source.bytes[0], this->ip4source.bytes[1], this->ip4source.bytes[2], this->ip4source.bytes[3], this->clientport);

		else
			StandardIO::PrintToString(s, "unknown (IPv6?)");

		char* str = new char[s->Length()];
		String::Copy(str, s->CString());

		delete s;
		return str;
	}

	const char* SocketObj::Path()
	{
		return this->Name();
	}

	FSObject* SocketObj::Parent()
	{
		return nullptr;
	}

	Filesystem* SocketObj::RootFS()
	{
		return nullptr;
	}

	uint64_t SocketObj::Read(uint8_t* buf, uint64_t bytes)
	{
		uint64_t ret = math::min(this->recvbuffer->ByteCount(), bytes);
		buf = this->recvbuffer->Read(buf, bytes);
		return ret;
	}

	uint64_t SocketObj::Write(uint8_t *buf, uint64_t bytes)
	{
		// tcp is special because we need to do stupid things in the TCPConnection class.
		if(this->protocol == SocketProtocol::TCP)
		{
			if(this->ip6source.high == 0 && this->ip6dest.low == 0 && this->ip6dest.high == 0 && this->ip6dest.low == 0)
			{
				this->tcpconnection->SendUserPacket(buf, (uint16_t) bytes);
			}
		}
		else if(this->protocol == SocketProtocol::UDP)
		{
			// udp is simpler.
			if(bytes > GLOBAL_MTU)
				HALT("can't do that");

			if(this->ip6source.high == 0 && this->ip6dest.low == 0 && this->ip6dest.high == 0 && this->ip6dest.low == 0)
			{
				UDP::SendIPv4Packet(Kernel::KernelNIC, buf, (uint16_t) bytes, this->ip4dest, (uint16_t) this->clientport, (uint16_t) this->serverport);
			}
		}
		return bytes;
	}

	uint8_t SocketObj::Attributes()
	{
		return 0;
	}

	bool SocketObj::Exists()
	{
		return true;
	}
}
}
}
}
