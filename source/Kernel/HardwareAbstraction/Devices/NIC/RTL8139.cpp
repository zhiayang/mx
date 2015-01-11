// // RTL8139.cpp
// // Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under Creative Commons 3.0 Unported.

// #include <Kernel.hpp>
// #include <HardwareAbstraction/Devices/NIC.hpp>
// #include <HardwareAbstraction/Network.hpp>
// #include <HardwareAbstraction/Interrupts.hpp>
// #include <HardwareAbstraction/Devices/IOPort.hpp>

// using namespace Kernel::HardwareAbstraction::Devices::PCI;
// using namespace Library::StandardIO;
// using namespace Kernel::HardwareAbstraction::Network;

// namespace Kernel {
// namespace HardwareAbstraction {
// namespace Devices {
// namespace NIC
// {
// 	enum Registers
// 	{
// 		MAC0			= 0x00,		// Ethernet hardware address
// 		MAR0			= 0x08,		// Multicast filter
// 		TxStatus0		= 0x10,		// Transmit status (Four 32bit registers)
// 		TxStatus1		= 0x14,
// 		TxStatus2		= 0x18,
// 		TxStatus3		= 0x1C,
// 		TxAddr0		= 0x20,		// Tx descriptors (also four 32bit)
// 		TxAddr1		= 0x24,
// 		TxAddr2		= 0x28,
// 		TxAddr3		= 0x2C,
// 		RxBuf			= 0x30,
// 		RxEarlyCnt		= 0x34,
// 		RxEarlyStatus		= 0x36,
// 		Command		= 0x37,
// 		RxBufPtr		= 0x38,
// 		RxBufAddr		= 0x3A,
// 		IntrMask		= 0x3C,
// 		IntrStatus		= 0x3E,
// 		TxConfig		= 0x40,
// 		RxConfig		= 0x44,
// 		Timer			= 0x48,		// A general-purpose counter
// 		RxMissed         		= 0x4C,		// 24 bits valid, write clears
// 		Cfg9346		= 0x50,
// 		Config0			= 0x51,
// 		Config1			= 0x52,
// 		FlashReg		= 0x54,
// 		GPPinData		= 0x58,
// 		GPPinDir		= 0x59,
// 		MII_SMI		= 0x5A,
// 		HltClk			= 0x5B,
// 		MultiIntr		= 0x5C,
// 		TxSummary		= 0x60,
// 		MII_BMCR		= 0x62,
// 		MII_BMSR		= 0x64,
// 		NWayAdvert		= 0x66,
// 		NWayLPAR		= 0x68,
// 		NWayExpansion	= 0x6A,

// 		// Undocumented registers, but required for proper operation
// 		FIFOTMS		= 0x70,		// FIFO Control and test
// 		CSCR			= 0x74,		// Chip Status and Configuration Register
// 		PARA78		= 0x78,
// 		PARA7c			= 0x7c,		// Magic transceiver parameter register
// 	};

// 	static const uint64_t RxBufferSize = 0x4000 * 3;

// 	void StaticHandleInterrupt()
// 	{
// 		Kernel::KernelNIC->HandleInterrupt();
// 	}

// 	RTL8139::RTL8139(PCIDevice* dev) : GenericNIC(this)
// 	{
// 		this->pcidev = dev;

// 		// enable bus mastering
// 		uint32_t f = this->pcidev->GetRegisterData(0x4, 0, 2);
// 		this->pcidev->WriteRegisterData(0x4, 0, 2, (f | 0x4) & ~0x400);

// 		uint32_t mmio = (uint32_t) this->pcidev->GetBAR(0);
// 		assert(this->pcidev->IsBARIOPort(0));
// 		Log("Initialised Busmastering RTL8139 NIC with BaseIO %x", mmio);

// 		this->ioaddr = (uint16_t) mmio;

// 		Interrupts::InstallIRQHandler(this->pcidev->GetInterruptLine(), StaticHandleInterrupt);


// 		// power on
// 		IOPort::WriteByte(this->ioaddr + Registers::Config1, 0x0);

// 		this->Reset();
// 		Log("RTL8139 Software reset complete.");

// 		// get mac address
// 		Library::Memory::Set(this->MAC, 0, sizeof(this->MAC));
// 		for(uint8_t i = 0; i < 6; i++)
// 		{
// 			this->MAC[i] = IOPort::ReadByte(this->ioaddr + Registers::MAC0 + i);
// 		}

// 		// Log("Raw register: %x, %x", IOPort::Read32(this->ioaddr + Registers::MAC0), IOPort::Read32(this->ioaddr + Registers::MAC0 + 4));
// 		Log("Initialised RTL8139 NIC with MAC address %#02x:%#02x:%#02x:%#02x:%#02x:%#02x", this->MAC[0], this->MAC[1], this->MAC[2], this->MAC[3], this->MAC[4], this->MAC[5]);

// 		Log("Using IRQ number %d and Interrupt Pin #%c", this->pcidev->GetInterruptLine(), this->pcidev->GetInterruptPin() + 'A' - 1);

// 		// init rx buffer.
// 		// setup 8K + 1500 + 16
// 		// 3 pages contiguous

// 		// this->ReceiveBuffer = (uint8_t*) MemoryManager::Physical::AllocatePage_Contiguous(3);
// 		this->ReceiveBuffer = (uint8_t*) MemoryManager::Physical::AllocateDMA(3);
// 		Log("Configured 12kb buffer for RX at %x", this->ReceiveBuffer);

// 		IOPort::Write32(this->ioaddr + Registers::RxBuf, (uint32_t) (uint64_t) this->ReceiveBuffer);
// 		IOPort::Write16(this->ioaddr + Registers::RxBufPtr, 0);
// 		IOPort::Write16(this->ioaddr + Registers::RxBufAddr, 0);
// 		IOPort::Write16(this->ioaddr + Registers::IntrMask, 0xF);


// 		// setup transmit buffers.
// 		// 1500 bytes each, 4 buffers
// 		// 2 pages, non contiguous.

// 		this->TransmitBuffers[0] = MemoryManager::Physical::AllocateDMA(1);
// 		this->TransmitBuffers[1] = this->TransmitBuffers[0] + 0x800;

// 		this->TransmitBuffers[2] = MemoryManager::Physical::AllocateDMA(1);
// 		this->TransmitBuffers[3] = this->TransmitBuffers[2] + 0x800;

// 		Log("Tx[0] at %x, Tx[1] at %x, Tx[2] at %x, Tx[3] at %x", this->TransmitBuffers[0], this->TransmitBuffers[1], this->TransmitBuffers[2], this->TransmitBuffers[3]);

// 		// send data to the card
// 		IOPort::Write32(this->ioaddr + Registers::TxAddr0, (uint32_t) this->TransmitBuffers[0]);
// 		IOPort::Write32(this->ioaddr + Registers::TxAddr1, (uint32_t) this->TransmitBuffers[1]);
// 		IOPort::Write32(this->ioaddr + Registers::TxAddr2, (uint32_t) this->TransmitBuffers[2]);
// 		IOPort::Write32(this->ioaddr + Registers::TxAddr3, (uint32_t) this->TransmitBuffers[3]);


// 		// set size (max, 8K + 15 + WRAP = 1)
// 		IOPort::Write32(this->ioaddr + Registers::RxConfig, 0x8F);


// 		// enable TransmitOK, ReceiveOK, TransmitErr, ReceiveErr, SystemErr interrupts

// 		// enable things.
// 		IOPort::WriteByte(this->ioaddr + Registers::Command, 0x0C);

// 		this->transmitbuffermtx[0] = new Mutex();
// 		this->transmitbuffermtx[1] = new Mutex();
// 		this->transmitbuffermtx[2] = new Mutex();
// 		this->transmitbuffermtx[3] = new Mutex();
// 	}

// 	void RTL8139::Reset()
// 	{
// 		IOPort::WriteByte(this->ioaddr + Registers::Command, 0x10);

// 		// wait for reset.
// 		while(IOPort::ReadByte(this->ioaddr + Registers::Command) & 0x10);
// 	}

// 	void RTL8139::SendData(uint8_t *data, uint64_t bytes)
// 	{
// 		if(bytes > 1500)
// 		{
// 			Log(1, "Tried to transmit packet larger than 1500 bytes long, exceeds MTU -- aborting transmit");
// 			return;
// 		}


// 		int usebuf = this->CurrentTxBuffer;
// 		this->CurrentTxBuffer++;
// 		this->CurrentTxBuffer %= 4;

// 		auto m = AutoMutex(this->transmitbuffermtx[usebuf]);

// 		// copy the data to the transmit buffer.
// 		Library::Memory::Copy((void*) (this->TransmitBuffers[usebuf]), data, bytes);

// 		uint32_t status = 0;
// 		status |= bytes & 0x1FFF;	// 0-12: Length
// 		status |= 0 << 13;		// 13: OWN bit
// 		status |= (0 & 0x3F) << 16;	// 16-21: Early TX threshold (zero, transmit immediately)

// 		// PrintFormatted("sent\n");
// 		IOPort::Write32(this->ioaddr + Registers::TxStatus0 + (uint16_t) usebuf * 4, status);
// 	}

// 	uint8_t* RTL8139::ReceiveData(uint8_t *data, uint64_t bytes)
// 	{
// 		HALT("packet received");

// 		UNUSED(bytes);
// 		return data;
// 	}

// 	uint64_t RTL8139::GetHardwareType()
// 	{
// 		return 0x1;
// 	}

// 	void RTL8139::HandlePacket()
// 	{
// 		// depending on the contents of the packet, we stuff it into a queue of the appropriate type.

// 		uint64_t ReadOffset = 0;
// 		uint64_t EndOffset = 0;
// 		uint64_t PacketCount = 0;
// 		uint64_t length = 0;

// 		EndOffset = IOPort::Read16(this->ioaddr + Registers::RxBufAddr);
// 		ReadOffset = this->SeenOfs;

// 		if(ReadOffset > EndOffset)
// 		{
// 			while(ReadOffset < RxBufferSize)
// 			{
// 				PacketCount++;
// 				length = *(uint16_t*) &this->ReceiveBuffer[ReadOffset + 2];
// 				Ethernet::HandlePacket(this, &this->ReceiveBuffer[ReadOffset + 4], length - 4);

// 				if(length > 2000)
// 					HALT("What");

// 				ReadOffset += length + 4;
// 				ReadOffset = (ReadOffset + 3) & ~3;	// align to 4 bytes
// 			}

// 			ReadOffset -= RxBufferSize;
// 		}
// 		while(ReadOffset < EndOffset)
// 		{
// 			PacketCount++;
// 			Ethernet::HandlePacket(this, &this->ReceiveBuffer[ReadOffset + 4], *(uint16_t*) &this->ReceiveBuffer[ReadOffset + 2] - 4);

// 			ReadOffset += *(uint16_t*) &this->ReceiveBuffer[ReadOffset + 2] + 4;
// 			ReadOffset = (ReadOffset + 3) & ~3;	// align to 4 bytes

// 		}
// 		this->SeenOfs = ReadOffset;
// 	}

// 	void RTL8139::HandleRxOk()
// 	{
// 		this->HandlePacket();
// 		IOPort::Write16(this->ioaddr + Registers::IntrStatus, 0x1);
// 	}

// 	void RTL8139::HandleRxErr()
// 	{
// 		PrintFormatted("Rx Err\n");
// 		IOPort::Write16(this->ioaddr + Registers::IntrStatus, 0x2);
// 	}

// 	void RTL8139::HandleTxOk()
// 	{
// 		// PrintFormatted("Tx Ok\n");

// 		for(int i = 0; i < 4; i++)
// 		{
// 			if(IOPort::Read32((this->ioaddr + Registers::TxStatus0 + ((uint16_t) i * 4)) & 0x8000))
// 			{
// 				// PrintFormatted("Released Tx Buffer %d\n", i);
// 				this->TxBufferInUse[i] = false;
// 			}
// 		}

// 		IOPort::Write16(this->ioaddr + Registers::IntrStatus, 0x4);
// 	}

// 	void RTL8139::HandleTxErr()
// 	{
// 		PrintFormatted("Tx Err\n");
// 		IOPort::Write16(this->ioaddr + Registers::IntrStatus, 0x8);
// 	}

// 	void RTL8139::HandleSysErr()
// 	{
// 		PrintFormatted("Error motherfucker\n");
// 		IOPort::Write16(this->ioaddr + Registers::IntrStatus, 0x8000);
// 	}

// 	void RTL8139::HandleInterrupt()
// 	{
// 		// check if the card fired the interrupt
// 		uint16_t status = IOPort::Read16(this->ioaddr + Registers::IntrStatus);

// 		if(status & 0x1)
// 			this->HandleRxOk();

// 		if(status & 0x2)
// 			this->HandleRxErr();

// 		if(status & 0x4)
// 			this->HandleTxOk();

// 		if(status & 0x8)
// 			this->HandleTxErr();

// 		if(status & 0x8000)
// 			this->HandleSysErr();
// 	}


// }
// }
// }
// }
