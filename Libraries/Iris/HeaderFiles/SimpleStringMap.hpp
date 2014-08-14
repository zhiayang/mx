// Map.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once
#include <stdint.h>

#include "List.hpp"
#include "String.hpp"
#include "Utility.hpp"

namespace Library
{
	class SimpleStringMap
	{
		private:
			class SSMNode
			{
				public:
					SSMNode(Library::string* k, Library::string* v);

					Library::string* key;
					Library::string* value;
			};

			LinkedList<SSMNode>* NodeList;



		public:
			SimpleStringMap();
			~SimpleStringMap();
			bool Put(Library::string* key, Library::string* value);
			Library::string* Get(Library::string* key);
			Library::string* Get(const char* key);

			LinkedList<Library::string>* Values();

			uint64_t Size();
	};
}
