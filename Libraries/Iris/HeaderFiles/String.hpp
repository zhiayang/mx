// userspace/String.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once
#include <stdint.h>
#include "List.hpp"
#include "Hashable.hpp"

namespace Library
{
	namespace String
	{
		uint64_t Length(const char* str);
		char* Copy(char* dest, const char* src);
		char* CopyLength(char* dest, const char* src, uint64_t Length);
		char* Concatenate(char* dest, const char* src);
		char* ConcatenateLength(char* dest, const char* src, unsigned long n);
		char* ConcatenateChar(char* dest, const char c);
		bool Compare(const char* a, const char* b);
		bool CompareLength(const char* a, const char* b, uint64_t length);
		char* Reverse(char* str, uint64_t length);

		char* TrimWhitespace(char* str);
		char* SubString(const char* src, uint64_t offset, uint64_t length = 0);
		char* Clear(char* src);

		char* Truncate(char* str, uint64_t Length);
	}

	class string : public Hashable
	{
		public:
			string(const char* str);
			string(string* f);
			string(const string &obj);
			string();
			~string();

			uint64_t Hash();

			char* CString() const;
			int Length() const;
			uint64_t AllocLength() const { return this->AllocatedLength; }
			LinkedList<string>* Split(const char delim, LinkedList<string>* list);
			void Strip(const char c);
			bool Contains(const char c);
			bool Contains(const string* s);
			bool Contains(const char* s);

			int IndexOf(const char c);
			int IndexOf(const string* s);
			int IndexOf(const char* s);

			bool StartsWith(const char c);
			bool StartsWith(const string* s);
			bool StartsWith(const char* s);

			bool EndsWith(const char c);
			bool EndsWith(const string* s);
			bool EndsWith(const char* s);

			void Append(const char c);
			void Append(const string* s);
			void Append(const char* s);

			void Prepend(const char c);
			void Prepend(const string* s);
			void Prepend(const char* s);

			void Set(string* s);
			void Set(const char* s);
			void Clear();

			void Insert(int index, const char c);
			void Remove(int index);

			void SubString(int start);
			void SubString(int start, int length);
			void Truncate(int newlength);

			string& operator=(const string &rhs);
			string& operator+=(const string &rhs);
			string& operator+=(const char &rhs);

			const string operator+(const string &other) const;
			bool operator==(const string &other) const;
			bool operator==(const char* other) const { return *this == string(other); }

			bool operator!=(const char* other) const { return !(*this == string(other)); }
			bool operator!=(const string &other) const { return !(*this == other); }
			char& operator[](const int index);
			operator const char*() const;



		private:
			char* RelocateString(uint64_t NewSize);

			char* TheString;
			uint64_t AllocatedLength = 0;
			uint64_t length = 0;
	};
}

