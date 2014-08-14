// FileIO.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "../HeaderFiles/FileIO.hpp"
#include "../HeaderFiles/String.hpp"
#include "../HeaderFiles/List.hpp"
#include "../HeaderFiles/SystemCall.hpp"


namespace Library {
namespace FileIO
{
	File::File(const char* n, uint8_t mode)
	{
		this->name = new char[String::Length(n)];
		String::Copy(this->name, n);
		this->fd = SystemCall::OpenFile(n, mode);
		this->size = SystemCall::GetFileSize(this->fd);
		this->exists = false;
	}

	File::~File()
	{
		SystemCall::CloseFile(this->fd);
		delete this->name;
	}

	void File::Open(uint8_t mode)
	{
		this->fd = SystemCall::OpenFile(this->name, mode);
		this->size = SystemCall::GetFileSize(this->fd);
	}

	void File::Close()
	{
		SystemCall::CloseFile(this->fd);
	}

	uint8_t* File::Read(uint8_t* buffer, uint64_t length)
	{
		return SystemCall::ReadFile(this->fd, buffer, length);
	}

	const char* File::Name()
	{
		return this->name;
	}

	uint64_t File::Size()
	{
		return this->size;
	}
}
}











