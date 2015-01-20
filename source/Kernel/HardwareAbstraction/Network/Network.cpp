// Network.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include <Kernel.hpp>
#include <HardwareAbstraction/Network.hpp>

using namespace Library;

namespace Kernel {
namespace HardwareAbstraction {
namespace Network
{
	void Initialise()
	{
		// initialise all the socket maps
		// IP::ipv4socketmap = new HashMap<SocketIPv4Mapping, Network::Socket::SocketObj>();
		// UDP::udpsocketmapv4 = new HashMap<SocketFullMappingv4, Network::Socket::SocketObj>();
		// UDP::udpsocketmapv6 = new HashMap<SocketFullMappingv6, Network::Socket::SocketObj>();

		// TCP::tcpsocketmapv4 = new HashMap<SocketFullMappingv4, Network::Socket::SocketObj>();
		// TCP::tcpsocketmapv6 = new HashMap<SocketFullMappingv6, Network::Socket::SocketObj>();

		// // initialise DHCP requests.
		// DHCP::Initialise();
		// DNS::Initialise();
	}
}
}
}
