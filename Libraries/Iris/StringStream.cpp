// StringStream.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/String.hpp"
#include "HeaderFiles/StringStream.hpp"

namespace Library
{
	// StringStream::StringStream()
	// {
	// 	this->backingstore = new string();
	// }

	StringStream::StringStream(string& str)
	{
		this->backingstore = new char[str.Length()];
		String::Copy(this->backingstore, str.CString());

		this->origlength = str.Length();
	}

	StringStream::StringStream(const char* str)
	{
		this->backingstore = new char[String::Length(str)];
		String::Copy(this->backingstore, str);

		this->origlength = (int) String::Length(str);
	}



	void StringStream::write(string& str)
	{
		// (*this->backingstore) += str;
		(void) str;
	}

	string* StringStream::read(char stop)
	{
		string* ret = new string();
		int length = (int) String::Length(this->backingstore);

		if(this->backingstore[0] == stop)
			this->backingstore++;

		for(int i = 0; i < length && stop == ' ' ? (this->backingstore[i] != ' ' && this->backingstore[i] != '\n') : this->backingstore[i] != stop; i++)
			*ret += this->backingstore[i];

		this->backingstore += ret->Length();
		return ret;
	}

	string* StringStream::readline()
	{
		return this->read('\n');
	}
}


