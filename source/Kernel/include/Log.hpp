// // Log.hpp
// // Copyright (c) 2014 - 2016, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.

// #pragma once
// #include <StandardIO.hpp>
// #include <HardwareAbstraction/Devices/SerialPort.hpp>

// namespace Kernel
// {
// 	void Log(uint8_t level, const char* str)
// 	{
// 		astl::string s = StdIO::Format("[%]: %\n", (level == 0 ? "INFO" : (level == 1 ? "WARN" : (level == 2 ? "SEVR" : "CRIT"))), str);

// 		Kernel::HardwareAbstraction::Devices::SerialPort::WriteString(s.c_str());
// 		if(level > 1) StdIO::PrintStr(s.c_str(), s.length());
// 	}

// 	void Log(const char* str)
// 	{
// 		Log((uint8_t) 0, str);
// 	}

// 	template <typename T, typename... Args>
// 	void Log(uint8_t level, const char* str, T&& head, Args&&... rest)
// 	{
// 		astl::string s = StdIO::Format("[%]: ", (level == 0 ? "INFO" : (level == 1 ? "WARN" : (level == 2 ? "SEVR" : "CRIT"))));
// 		astl::string f = StdIO::Format(str, head, rest...);

// 		s += f + "\n";

// 		Kernel::HardwareAbstraction::Devices::SerialPort::WriteString(s.c_str());
// 		if(level > 1) StdIO::PrintStr(s.c_str(), s.length());
// 	}

// 	template <typename T, typename... Args>
// 	void Log(const char* str, T&& head, Args&&... rest)
// 	{
// 		Log((uint8_t) 0, str, head, rest...);
// 	}
// }
