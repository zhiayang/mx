// Colours.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once

#include <stdint.h>
#include "Defines.hpp"
#include "SystemCall.hpp"


namespace Library
{
	namespace Colours
	{
		// Colours
		// B/W
		constexpr uint32_t White			= 0xFFFFFFFF;
		constexpr uint32_t Black			= 0xFF000000;

		// Greys
		constexpr uint32_t LightGrey		= 0xFFD3D3D3;
		constexpr uint32_t Silver			= 0xFFC0C0C0;
		constexpr uint32_t DarkGrey			= 0xFFA9A9A9;
		constexpr uint32_t Grey				= 0xFF808080;
		constexpr uint32_t DimGrey			= 0xFF696969;


		// R/G/B
		constexpr uint32_t Red				= 0xFFFF0000;
		constexpr uint32_t Green			= 0xFF00FF00;
		constexpr uint32_t Blue				= 0xFF0000FF;


		// C/Y/M
		constexpr uint32_t Cyan				= Green | Blue;
		constexpr uint32_t Yellow			= Green | Red;
		constexpr uint32_t Magenta			= Blue | Red;


		// Tertiary colours -- A/V/R/O/C/SG
		constexpr uint32_t Azure			= 0x0007FFFF;
		constexpr uint32_t Violet			= 0xFF7F00FF;
		constexpr uint32_t Rose				= 0xFFFF007F;
		constexpr uint32_t Orange			= 0xFFFF7F00;
		constexpr uint32_t Chartreuse		= 0xFF7FFF00;
		constexpr uint32_t SpringGreen		= 0xFF00FF7F;


		// VGA 16-colour palette
		constexpr uint32_t DarkCyan			= 0xFF00AAAA;
		constexpr uint32_t Brown			= 0xFFAA5500;
		constexpr uint32_t DarkRed			= 0xFFAA3333;

		// Common web colours
		constexpr uint32_t Pink				= 0xFFFF69B4;
		constexpr uint32_t Purple			= 0xFF9B30FF;
	}
}





