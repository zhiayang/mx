// // ConfigFile.cpp
// // Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// // Licensed under the Apache License Version 2.0.

// #include <Kernel.hpp>
// #include <String.hpp>
// #include <StandardIO.hpp>
// #include <SimpleStringMap.hpp>

// #define RAPIDXML_NO_STDLIB
// #define RAPIDXML_NO_EXCEPTIONS
// #include <RapidXML/rapidxml.hpp>

// using Library::string;
// using Library::HashMap;

// namespace Kernel {
// namespace ConfigFile
// {
// 	void Initialise()
// 	{
// 		using Kernel::HardwareAbstraction::Filesystems::VFS::File;
// 		using Library::LinkedList;

// 		using namespace Library::StandardIO;
// 		using namespace rapidxml;
// 		Kernel::KernelConfigFile = new HashMap<string, string>();

// 		xml_document<> document;

// 		File* file = new File("/System/Library/Preferences/CorePreferences.plist");
// 		uint8_t* buf = new uint8_t[file->FileSize()];
// 		file->Read(buf);

// 		document.parse<0>((char*) buf);
// 		// make sure we're reading a plist.
// 		assert(Library::String::Compare(document.first_node()->name(), "plist"));
// 		xml_node<>* plist = document.first_node("plist");

// 		// read the dict entry.
// 		xml_node<>* dict = plist->first_node("dict");

// 		for(xml_node<>* entry = dict->first_node(); entry; entry = entry->next_sibling())
// 		{
// 			if(!entry->next_sibling())
// 			{
// 				PrintFormatted("Key %s missing value, ignoring...\n", entry->value());
// 				continue;
// 			}

// 			if(!Library::String::Compare(entry->name(), "key"))
// 			{
// 				PrintFormatted("Malformed plist, expected <key>...</key> but got <%s> instead\n", entry->name());
// 				continue;
// 			}


// 			string keyval = entry->value();
// 			string valval = entry->next_sibling()->value();

// 			// check if we got a boolean.
// 			if(valval.Length() == 0)
// 			{
// 				string valname = entry->next_sibling()->name();
// 				if(valname == string("true") || valname == string("false"))
// 					valval = valname;

// 				else
// 				{
// 					PrintFormatted("Malformed plist, expected boolean with either <true/> or <false/> but got <%s> instead\n", valname.CString());
// 					continue;
// 				}
// 			}

// 			Kernel::KernelConfigFile->Put(string(entry->value()), string(entry->next_sibling()->value()));
// 			entry = entry->next_sibling();
// 		}
// 	}

// 	int64_t ReadInteger(const char* key)
// 	{
// 		string* r = Kernel::KernelConfigFile->get(string(key));

// 		if(!r)
// 			return 0;

// 		else
// 		{
// 			return Library::Utility::ConvertToInt(r->CString());
// 		}
// 	}

// 	Library::string* ReadString(const char* key)
// 	{
// 		return Kernel::KernelConfigFile->get(string(key));
// 	}

// 	bool ReadBoolean(const char* key)
// 	{
// 		using Library::string;
// 		using Library::SimpleStringMap;

// 		string* r = Kernel::KernelConfigFile->get(string(key));

// 		if(!r)
// 			return false;

// 		else
// 		{
// 			string str = *r;

// 			if(str == string("true") || str == string("1"))
// 				return true;

// 			else
// 				return false;
// 		}
// 	}
// }
// }


















