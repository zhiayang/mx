// userspace/String.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/String.hpp"
#include "HeaderFiles/StandardIO.hpp"
#include "HeaderFiles/Heap.hpp"
#include <string.h>

using namespace Library::String;
using namespace Library::StandardIO;



namespace Library
{
	// string::string(const char* str)
	// {
	// 	this->length = String::Compare(str, "") ? 0 : String::Length(str);
	// 	this->TheString = new char[this->length > 32 ? this->length : 32];

	// 	if(this->length > 0)
	// 		Library::String::Copy(this->TheString, str);

	// 	// Because the dynamic memory manager may return a block of larger size.
	// 	this->AllocatedLength = Heap::QuerySize((void*) this->TheString);
	// 	// this->AllocatedLength = 32;
	// }

	// string::string(string* str)
	// {
	// 	this->length = String::Compare(str->CString(), "") ? 0 : String::Length(str->CString());
	// 	this->TheString = new char[this->length > 32 ? this->length : 32];

	// 	if(this->length > 0)
	// 		Library::String::Copy(this->TheString, str->CString());

	// 	// Because the dynamic memory manager may return a block of larger size.
	// 	this->AllocatedLength = Heap::QuerySize((void*) this->TheString);
	// 	// this->AllocatedLength = this->length;
	// }

	// string::string()
	// {
	// 	this->length = 0;
	// 	this->TheString = new char[32];

	// 	// Because the dynamic memory manager may return a block of larger size.
	// 	this->AllocatedLength = Heap::QuerySize((void*) this->TheString);
	// 	// this->AllocatedLength = 32;
	// }

	// string::string(const string& obj)
	// {
	// 	this->length = obj.length;

	// 	this->TheString = new char[obj.AllocatedLength];
	// 	this->AllocatedLength = obj.AllocatedLength;

	// 	String::Copy(this->TheString, obj.TheString);
	// }

	// string::~string()
	// {
	// 	delete[] this->TheString;
	// }

	// char* string::CString() const
	// {
	// 	return this->TheString;
	// }

	// int string::Length() const
	// {
	// 	return (int) this->length;
	// }

	// void string::Clear()
	// {
	// 	this->length = 0;
	// 	Memory::Set((void*) this->TheString, 0x0, this->AllocatedLength);
	// }

	// void string::Truncate(int newlength)
	// {
	// 	if((uint64_t) newlength > this->length)
	// 		return;

	// 	this->TheString[newlength] = 0;
	// 	this->length -= this->length - newlength;
	// }

	// void string::SubString(int s, int l)
	// {
	// 	char* f = new char[this->length];
	// 	char* tmp = f;
	// 	CopyLength(tmp, this->TheString + s, l);

	// 	delete[] this->TheString;

	// 	this->TheString = f;
	// 	this->length = String::Length(this->TheString);
	// }

	// void string::SubString(int start)
	// {
	// 	this->SubString(start, ((int) this->length - start));
	// }

	// uint64_t string::Hash()
	// {
	// 	uint64_t hash = 0;
	// 	uint64_t actual = String::Length(this->TheString);

	// 	for(uint64_t i = 0; i < actual; i++)
	// 		hash = ((hash << 3) | (hash >> 29)) ^ this->TheString[i];

	// 	return hash;
	// }

	// LinkedList<string>* string::Split(const char delim, LinkedList<string>* Tokens)
	// {
	// 	uint64_t pi = 0, i = 0;
	// 	if(String::Length(this->TheString) == 1 && this->TheString[0] == delim)
	// 		return Tokens;

	// 	for(i = 0; i < String::Length(this->TheString); i++)
	// 	{
	// 		if(this->TheString[i] == delim && (i + 1) < this->length)
	// 		{
	// 			if(i == 0)
	// 			{
	// 				pi = 1;
	// 				continue;
	// 			}

	// 			if(i - pi == 0)
	// 			{
	// 				pi++;
	// 				continue;
	// 			}

	// 			char* f = Library::String::SubString(this->TheString, pi, i - pi);
	// 			Tokens->InsertBack(new string(f));

	// 			delete[] f;
	// 			pi = i + 1;
	// 		}
	// 	}

	// 	// add the last token
	// 	if(pi < i)
	// 	{
	// 		if(this->length - pi == 0)
	// 		{
	// 			return Tokens;
	// 		}

	// 		char* f = Library::String::SubString(this->TheString, pi, this->length - pi);

	// 		if(f[Library::String::Length(f) - 1] == delim)
	// 			f[Library::String::Length(f) - 1] = 0;

	// 		string* k = new string(f);
	// 		Tokens->InsertBack(k);
	// 		delete[] f;
	// 	}

	// 	return Tokens;
	// }


	// string& string::operator=(const string &rhs)
	// {
	// 	// check for self-assignment.
	// 	if(this == &rhs)
	// 		return *this;

	// 	// do a string copy, but handle a string larger than currently allocated.
	// 	if(Library::String::Length((&rhs)->TheString) > this->AllocatedLength)
	// 	{
	// 		RelocateString(Library::String::Length((&rhs)->TheString));
	// 	}
	// 	else
	// 	{
	// 		Memory::Copy(this->TheString, (&rhs)->TheString, (&rhs)->length);
	// 	}

	// 	return *this;
	// }

	// string& string::operator+=(const string &rhs)
	// {
	// 	this->Append(&rhs);
	// 	return *this;
	// }

	// string& string::operator+=(const char &rhs)
	// {
	// 	this->Append(rhs);
	// 	return *this;
	// }

	// const string string::operator+(const string &other) const
	// {
	// 	return string((*this).TheString) += other;
	// }

	// bool string::operator==(const string &other) const
	// {
	// 	return Library::String::Compare(this->TheString, other.TheString);
	// }

	// char& string::operator[](const int index)
	// {
	// 	return this->TheString[index];
	// }

	// string::operator const char*() const
	// {
	// 	return (const char*) this->TheString;
	// }

	// void string::Strip(const char c)
	// {
	// 	char *p1 = this->TheString, *p2 = this->TheString;

	// 	do
	// 	{
	// 		while(*p2 == c)
	// 			p2++;

	// 	} while((*p1++ = *p2++));

	// 	this->length = String::Length(this->TheString);
	// }

	// char* string::RelocateString(uint64_t NewSize)
	// {
	// 	char* pointer = new char[NewSize];
	// 	uint64_t k = Heap::QuerySize((void*) pointer);
	// 	this->AllocatedLength = k;
	// 	// this->AllocatedLength = NewSize;


	// 	String::CopyLength(pointer, this->TheString, this->length);
	// 	delete[] this->TheString;

	// 	this->TheString = pointer;
	// 	return pointer;
	// }

	// bool string::Contains(const char c)
	// {
	// 	return this->IndexOf(c) >= 0;
	// }
	// bool string::Contains(const string* s)
	// {
	// 	return this->IndexOf(s) >= 0;
	// }
	// bool string::Contains(const char* s)
	// {
	// 	return this->IndexOf(s) >= 0;
	// }

	// int string::IndexOf(const char c)
	// {
	// 	// iterate by char
	// 	for(uint64_t i = 0; i < this->length; i++)
	// 	{
	// 		if(this->TheString[i] == c)
	// 			return (int) i;
	// 	}

	// 	return -1;
	// }

	// int string::IndexOf(const string* s)
	// {
	// 	return this->IndexOf(s->TheString);
	// }

	// int string::IndexOf(const char* s)
	// {
	// 	uint64_t l = String::Length(s);
	// 	char* f;

	// 	for(uint64_t i = 0; i < this->length; i++)
	// 	{
	// 		f = String::SubString((char*) this->TheString, i, l);

	// 		if(Compare(s, f))
	// 		{
	// 			delete[] f;
	// 			return (int) i;
	// 		}
	// 		delete[] f;
	// 	}

	// 	return -1;
	// }

	// bool string::EndsWith(const char c)
	// {
	// 	return this->TheString[this->length - 1] == c;
	// }


	// bool string::StartsWith(const char c)
	// {
	// 	return this->TheString[0] == c;
	// }

	// bool string::StartsWith(const string* s)
	// {
	// 	return this->StartsWith(s->TheString);
	// }

	// bool string::StartsWith(const char* s)
	// {
	// 	uint64_t l = String::Length(s);
	// 	char* f;

	// 	f = String::SubString((char*) this->TheString, 0, l);

	// 	bool r = Compare(s, f);
	// 	delete[] f;
	// 	return r;
	// }

	// void string::Append(const char c)
	// {
	// 	// check if our current block will accomodate the new size.x

	// 	if(this->AllocatedLength < this->length + 2)
	// 	{
	// 		RelocateString(this->length + 2);
	// 	}

	// 	this->TheString[this->length] = c;
	// 	this->TheString[this->length + 1] = 0;

	// 	this->length++;
	// }

	// void string::Append(const string* s)
	// {
	// 	this->Append(s->CString());
	// }
	// void string::Append(const char* s)
	// {
	// 	// check if our current block will accomodate the new size.
	// 	if(this->AllocatedLength <= String::Length(s) + this->length + 1)
	// 	{
	// 		RelocateString(String::Length(s) + this->length + 1);
	// 	}

	// 	this->TheString = Library::String::Concatenate(this->TheString, s);
	// 	this->length += String::Length(s);
	// }

	// void string::Set(string* s)
	// {
	// 	this->Set(s->CString());
	// }

	// void string::Set(const char* s)
	// {
	// 	if(this->AllocatedLength <= String::Length(s))
	// 	{
	// 		RelocateString(String::Length(s) + 1);
	// 	}

	// 	String::CopyLength(this->TheString, s, String::Length(s));
	// 	this->length = String::Length(s);
	// }

	// void string::Prepend(const char c)
	// {
	// 	// check if our current block will accomodate the new size.
	// 	if(this->AllocatedLength <= this->length + 2)
	// 	{
	// 		RelocateString(this->length + 2);
	// 	}

	// 	char* copy = new char[this->AllocatedLength];
	// 	String::ConcatenateChar(copy, c);
	// 	String::Concatenate(copy, this->TheString);

	// 	String::Copy(this->TheString, copy);
	// 	delete[] copy;

	// 	this->length++;
	// }

	// void string::Prepend(const string* s)
	// {
	// 	this->Prepend(s->CString());
	// }

	// void string::Prepend(const char* s)
	// {
	// 	// check if our current block will accomodate the new size.
	// 	if(this->AllocatedLength <= String::Length(s) + this->length)
	// 	{
	// 		RelocateString(String::Length(s) + this->length);
	// 	}

	// 	char* copy = new char[this->AllocatedLength];
	// 	String::Copy(copy, this->TheString);

	// 	String::Clear(this->TheString);
	// 	String::Concatenate(this->TheString, s);
	// 	String::Concatenate(this->TheString, copy);
	// 	delete[] copy;

	// 	this->length = String::Length(this->TheString);
	// }

	// void string::Insert(int index, const char c)
	// {
	// 	if(this->AllocatedLength <= 1 + this->length)
	// 	{
	// 		RelocateString(4 + this->length);
	// 	}

	// 	string front = this->TheString;
	// 	if(index > front.Length())
	// 		index = front.Length();

	// 	front.SubString(0, index);

	// 	string back = this->TheString;
	// 	back.SubString(index, back.Length() - index);

	// 	front.Append(c);
	// 	front.Append(back);

	// 	String::Clear(this->TheString);
	// 	String::CopyLength(this->TheString, front.TheString, front.length);
	// 	this->length++;
	// }

	// void string::Remove(int index)
	// {
	// 	string front = this->TheString;
	// 	if(index > front.Length())
	// 		index = front.Length() - 1;

	// 	front.SubString(0, index);

	// 	string back = this->TheString;
	// 	back.SubString(index + 1, back.Length() - index);
	// 	front.Append(back);

	// 	String::Clear(this->TheString);
	// 	String::CopyLength(this->TheString, front.TheString, front.length);
	// 	this->length--;
	// }
















	namespace String
	{
		// uint64_t Length(const char* str)
		// {
		// 	uint64_t ret = 0;
		// 	while(str[ret] != 0)
		// 		ret++;

		// 	return ret;
		// }

		// char* Copy(char* dest, const char* src)
		// {
		// 	char* origdest = dest;
		// 	while(*src)
		// 	{
		// 		*dest++ = *src++;
		// 	}

		// 	*dest = '\0';
		// 	return origdest;
		// }

		// char* CopyLength(char* dst, const char* src, uint64_t Length)
		// {
		// 	char* dest = dst;
		// 	char* source = (char*) src;

		// 	while(Length)
		// 	{
		// 		*dest++ = *source++;
		// 		Length--;
		// 	}

		// 	*dest = '\0';
		// 	return dst;
		// }

		// char* Concatenate(char* dest, const char* src)
		// {
		// 	return ConcatenateLength(dest, src, Length(dest) > Length(src) ? Length(dest) : Length(src));
		// }

		// char* ConcatenateLength(char* dest, const char* src, unsigned long n)
		// {
		// 	uint64_t dest_len = Length(dest);
		// 	uint64_t i;
		// 	for(i = 0; i < n && src[i] != '\0'; i++)
		// 	{
		// 		dest[dest_len + i] = src[i];
		// 	}

		// 	dest[dest_len + i] = '\0';
		// 	return dest;
		// }

		// char* Clear(char* src)
		// {
		// 	for(uint64_t i = 0; i < Length(src); i++)
		// 	{
		// 		src[i] = 0;
		// 	}
		// 	return src;
		// }

		// char* ConcatenateChar(char* dest, const char c)
		// {
		// 	uint64_t dest_len = Length(dest);

		// 	dest[dest_len] = c;
		// 	dest[dest_len + 1] = '\0';
		// 	return dest;
		// }

		// bool Compare(const char* a, const char* b)
		// {
		// 	return CompareLength(a, b, (Length(a) > Length(b)) ? Length(a) : Length(b));
		// }

		// bool CompareLength(const char* a, const char* b, uint64_t length)
		// {
		// 	if(Length(a) != Length(b))
		// 	{
		// 		return false;
		// 	}

		// 	// clamp the value of length
		// 	if(length > Length(a) || length > Length(b))
		// 		length = Length(a) > Length(b) ? Length(a) : Length(b);

		// 	for(uint64_t l = length; l > 0; l--)
		// 	{
		// 		if(*(a + l) != *(b + l))
		// 			return false;
		// 	}
		// 	return true;
		// }

		// char* Reverse(char* str, uint64_t length)
		// {
		// 	uint64_t i = 0, j = length - 1;
		// 	char tmp;
		// 	while(i < j)
		// 	{
		// 		tmp = str[i];
		// 		str[i] = str[j];
		// 		str[j] = tmp;
		// 		i++;
		// 		j--;
		// 	}

		// 	return str;
		// }


		char* TrimWhitespace(char *str)
		{
			char *end;

			// Trim leading space
			while(*str == ' ')
				str++;

			if(*str == 0)  // All spaces?
				return str;

			// Trim trailing space
			end = str + strlen(str) - 1;
			while(end > str && *end == ' ')
				end--;

			// Write new null terminator
			*(end + 1) = 0;

			return str;
		}

		// char* SubString(const char* src, uint64_t offset, uint64_t length)
		// {
		// 	if(length == 0 || Length(src) - offset < length)
		// 	{
		// 		length = Length(src) - offset;
		// 	}

		// 	char* dest = new char[length + 1];
		// 	dest = CopyLength(dest, src + offset, length);

		// 	return dest;
		// }

		// char* Truncate(char* str, uint64_t length)
		// {
		// 	if(Length(str) <= length)
		// 		return str;

		// 	str[length] = 0;
		// 	return str;
		// }
	}
}
