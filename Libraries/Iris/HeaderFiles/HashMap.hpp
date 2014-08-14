// HashMap.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0


#include <stdint.h>
#include "List.hpp"
#include "StandardIO.hpp"
#include "Hashable.hpp"
#include "Loki/TypeManip.h"
#pragma once

namespace Library
{
	template <class K, class V>
	class HashMap
	{
		private:
			class HashBucket
			{
				public:
					HashBucket(K k, V v) : key(k), value(v)
					{
					}
					K key;
					V value;
			};

			uint64_t GetHashCode(K key)
			{
				if(Loki::SuperSubclass<Hashable, K>::value)
				{
					return ((Hashable*) (&key))->Hash();
				}
				else
				{
					uint8_t* data = (uint8_t*) (&key);
					uint64_t length = sizeof(K);

					uint64_t hash = 0;
					for(uint64_t i = 0; i < length; i++)
						hash = (hash << 3) | (hash >> (29)) ^ data[i];

					return hash;
				}
			}

			bool CompareKeys(K one, K two)
			{
				return this->GetHashCode(one) == this->GetHashCode(two);
			}


		public:
			HashMap(uint64_t s = 32)
			{
				this->size = s;
				this->backingstore = new LinkedList<HashBucket>*[this->size + 1];
			}

			void Put(K key, V value)
			{
				// get the hash
				uint64_t hash = this->GetHashCode(key);
				hash %= this->size;

				HashBucket* bucket = new HashBucket(key, value);

				if(!backingstore[hash])
					backingstore[hash] = new LinkedList<HashBucket>();

				backingstore[hash]->InsertBack(bucket);
			}

			V* Get(K key)
			{
				// get the hash
				uint64_t hash = this->GetHashCode(key);
				hash %= this->size;

				if(!this->backingstore[hash])
					return nullptr;

				for(uint64_t i = 0; i < this->backingstore[hash]->Size(); i++)
				{
					if(this->CompareKeys(this->backingstore[hash]->Get(i)->key, key))
						return &this->backingstore[hash]->Get(i)->value;
				}

				return nullptr;
			}

			void Remove(K key)
			{
				// does nothing if it doesn't exist.

				// get the hash
				uint64_t hash = this->GetHashCode(key);
				hash %= this->size;

				for(uint64_t i = 0; i < this->backingstore[hash]->Size(); i++)
				{
					if(this->CompareKeys(this->backingstore[hash]->Get(i)->key, key))
					{
						HashBucket* b = this->backingstore[hash]->Get(i);
						this->backingstore[hash]->RemoveAt(i);

						delete b;
						return;
					}
				}
			}

			bool ContainsKey(K key)
			{
				uint64_t hash = this->GetHashCode(key);
				hash %= this->size;

				return backingstore[hash]->Size() > 0;
			}

			LinkedList<K>* KeyList()
			{
				LinkedList<K>* list = new LinkedList<K>();
				for(uint64_t i = 0; i < this->size; i++)
				{
					if(this->backingstore[i])
					{
						for(uint64_t k = 0; k < this->backingstore[i]->Size(); k++)
						{
							if(this->backingstore[i]->Get(k))
								list->InsertBack(&(this->backingstore[i]->Get(k)->key));
						}
					}
				}

				return list;
			}

		private:
			uint64_t size;
			LinkedList<HashBucket>** backingstore;
	};

}

