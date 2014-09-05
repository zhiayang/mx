// List.hpp
// Copyright (c) 2013 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#pragma once

#include "Memory.hpp"
#include "Iterator.hpp"


namespace Library
{
	template <class data>
	class LinkedList
	{
		private:
			class SingleNode
			{
				public:
					SingleNode(data* o = 0, SingleNode* n = 0)
					{
						this->object = reinterpret_cast<data*>(o);
						this->next = n;
					}

					SingleNode* next;
					data* object;
			};

			uint64_t length;
			SingleNode* head;
			SingleNode* tail;




			SingleNode* Traverse(uint64_t i) const
			{
				SingleNode* h = this->head;

				if(this->length == 1 || i == 0)
				{
					return this->head;
				}
				else if(this->length == 2 && i == 1)
				{
					return this->tail;
				}
				for(uint64_t l = 0; l < i; l++)
				{
					h = h->next;
				}
				return h;
			}



		public:
			// this needs to be already allocated; give a head pointer.
			LinkedList()
			{
				this->head = 0;
				this->tail = 0;
				this->length = 0;
			}
			~LinkedList()
			{
				// delete all nodes
				for(uint64_t i = 0, m = this->length; i < m; i++)
				{
					SingleNode* s = this->head;
					this->head = this->head->next;

					delete s;
				}
			}


			uint64_t size(){ return this->length; }
			bool IsEmpty(){ return this->length == 0; }
			data* get(uint64_t i) const
			{
				if(this->length == 1 || i == 0)
				{
					return this->head->object;
				}
				else if(i == this->length - 1)
				{
					return this->tail->object;
				}

				return this->Traverse(i)->object;
			}






			void push_front(data* obj)
			{
				SingleNode* f = new SingleNode(obj, this->head);


				if(this->IsEmpty())
				{
					this->head = f;
					this->tail = f;
				}
				else
				{
					this->head = f;
				}

				this->length++;
			}

			void push_back(data* obj)
			{
				SingleNode* f = new SingleNode(obj, 0);

				if(this->IsEmpty())
				{
					this->head = f;
				}
				else
				{
					this->tail->next = f;
				}

				this->tail = f;
				this->length++;
			}

			void AddAll(LinkedList<data>* l)
			{
				for(uint64_t i = 0; i < l->size(); i++)
				{
					this->InsertBack(l->get(i));
				}
			}

			data* pop_front()
			{
				// if(this->head == 0)
				// 	HALT("Removing from empty list");

				data* obj = this->head->object;
				SingleNode* old_head = this->head;

				if(this->length == 1)
				{
					this->head = 0;
					this->tail = 0;
				}
				else
				{
					this->head = this->head->next;
				}

				delete old_head;
				this->length--;
				return obj;
			}

			data* pop_back()
			{
				// if(this->tail == 0)
				// 	HALT("Removing from empty list!");

				SingleNode* old_tail = this->tail;
				data* obj = this->tail->object;


				if(this->length == 1)
				{
					this->head = 0;
					this->tail = 0;
				}
				else
				{
					SingleNode* kr = this->head;
					while(kr->next != this->tail)
						kr = kr->next;

					kr->next = 0;
					this->tail = kr;
				}

				delete old_tail;
				this->length--;
				return obj;
			}

			data* back()
			{
				return this->tail->object;
			}

			data* front()
			{
				return this->head->object;
			}

			void Clear()
			{
				for(uint64_t m = 0, g = this->length; m < g; m++)
				{
					this->RemoveFront();
				}
			}

			int64_t IndexOf(data* p)
			{
				for(int64_t d = 0; static_cast<uint64_t>(d) < this->length; d++)
				{
					if(p == this->get(static_cast<uint64_t>(d)))
						return d;
				}

				return -1;
			}

			data* RemoveAt(uint64_t i)
			{
				if(i == 0)
					return this->pop_front();

				if(i == this->length - 1)
					return this->pop_back();


				data* ret = this->get(i);
				SingleNode* k = this->Traverse(i);
				SingleNode* p = this->Traverse(i - 1);

				p->next = k->next;
				delete k;
				this->length--;

				return ret;
			}

			void InsertAt(uint64_t i, data* d)
			{
				if(i >= this->length)
					this->InsertBack(d);

				else if(i == 0)
					this->InsertFront(d);

				else
				{
					SingleNode* p = this->Traverse(i - 1);
					SingleNode* t = this->Traverse(i);
					SingleNode* n = this->Traverse(i + 1);
					p->next = new SingleNode(d, t);
					t->next = n;
					this->length++;
				}
			}
	};
}




