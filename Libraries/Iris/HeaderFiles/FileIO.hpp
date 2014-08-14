// FileIO.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <stdint.h>
#include "List.hpp"
#include "String.hpp"
#include "SystemCall.hpp"

namespace Library {
namespace FileIO
{
	enum class FSObjectTypes
	{
		File = 1,
		Folder = 2
	};

	class File
	{
		public:
			File(const char* n, uint8_t mode);
			~File();

			void Open(uint8_t mode);
			void Close();
			bool Exists();
			uint8_t* Read(uint8_t* buffer, uint64_t length);

			const char* Name();
			uint64_t Size();

		private:
			uint64_t fd;
			uint64_t size;
			char* name;
			bool exists;
	};

	class Folder
	{
	};
}
}
