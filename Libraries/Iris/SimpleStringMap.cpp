// Map.cpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "HeaderFiles/SimpleStringMap.hpp"
#include "HeaderFiles/List.hpp"
#include "HeaderFiles/String.hpp"

namespace Library
{
	SimpleStringMap::SSMNode::SSMNode(string* k, string* v)
	{
		key = k;
		value = v;
	}

	SimpleStringMap::SimpleStringMap()
	{
		this->NodeList = new LinkedList<SSMNode>();
	}
	SimpleStringMap::~SimpleStringMap()
	{
		delete this->NodeList;
	}

	bool SimpleStringMap::Put(string* key, string* value)
	{
		this->NodeList->InsertBack(new SSMNode(key, value));
		return true;
	}

	string* SimpleStringMap::Get(string* key)
	{
		for(uint64_t i = 0; i < this->NodeList->Size(); i++)
		{
			if(Library::String::Compare(key->CString(), this->NodeList->Get(i)->key->CString()))
			{
				return this->NodeList->Get(i)->value;
			}
		}

		return 0;
	}

	string* SimpleStringMap::Get(const char* key)
	{
		string* d = new string(key);
		string* r = this->Get(d);

		delete d;
		return r;
	}

	LinkedList<string>* SimpleStringMap::Values()
	{
		LinkedList<string>* ret = new LinkedList<string>();
		for(uint64_t k = 0; k < this->NodeList->Size(); k++)
		{
			ret->InsertBack(this->NodeList->Get(k)->value);
		}

		return ret;
	}

	uint64_t SimpleStringMap::Size()
	{
		return this->NodeList->Size();
	}
}

