// BinarySearchTree.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <stdint.h>
#include "Comparable.hpp"
#include "List.hpp"
#include "Loki/TypeManip.h"

#pragma once

namespace Library
{
	template<class K, class V>
	class BinarySearchTree
	{
		protected:
			class Node
			{
				public:
					Node(K k, V v)
					{
						this->key = k;
						this->value = v;

						this->left = nullptr;
						this->right = nullptr;
						this->parent = nullptr;
					}

					K key;
					V value;

					Node* left;
					Node* right;
					Node* parent;
			};

			// void RotateLeft(Node*& node)
			// {
			// 	Node* pivot = node->right;
			// 	node->right = pivot->left;
			// 	pivot->left = node;

			// 	node->parent = pivot;
			// 	node = pivot;
			// }

			// void RotateRight(Node*& node)
			// {
			// 	Node* pivot = node->left;
			// 	node->left = pivot->right;
			// 	pivot->right = node;

			// 	node->parent = pivot;
			// 	node = pivot;
			// }



			V* SearchRecursive(Node* node, K k)
			{
				if(node == nullptr)
					return nullptr;

				if(k == node->key)
					return &node->value;

				else if(k < node->key)
					return SearchRecursive(node->left, k);

				else
					return SearchRecursive(node->right, k);
			}

			virtual void InsertRecursive(Node*& parent, Node* target)
			{
				if(parent == nullptr)
					parent = target;

				else if(target->key < parent->key)
				{
					target->parent = parent;
					InsertRecursive(parent->left, target);
				}

				else if(target->key > parent->key)
				{
					target->parent = parent;
					InsertRecursive(parent->right, target);
				}
			}

			Node* Minimum(Node* sub)
			{
				Node* current = sub;
				while(current->left != nullptr)
					current = current->left;

				return current;
			}

			Node* Maximum(Node* sub)
			{
				Node* current = sub;
				while(current->right != nullptr)
					current = current->right;

				return current;
			}

			void TraverseRecursive(LinkedList<V>*& list, Node* node)
			{
				if(node == nullptr)
					return;

				this->TraverseRecursive(list, node->left);
				list->InsertBack(&node->value);
				this->TraverseRecursive(list, node->right);
			}

			void ReplaceNode(Node* node, Node* newnode)
			{
				if(node->parent != nullptr)
				{
					if(node == node->parent->left)
					{
						// we are the left.
						node->parent->left = newnode;
					}
					else
					{
						// we are the right
						node->parent->right = newnode;
					}
				}

				if(newnode != nullptr)
					newnode->parent = node->parent;
			}

			void RemoveRecursive(Node* sub, K k)
			{
				if(k < sub->key)
					this->RemoveRecursive(sub->left, k);

				else if(k > sub->key)
					this->RemoveRecursive(sub->right, k);

				else
				{
					// delete it
					if(sub->left != nullptr && sub->right != nullptr)
					{
						// we have two children
						Node* next = 0;
						if(this->removedsuccessor)
							next = this->Minimum(sub->right);

						else
							next = this->Minimum(sub->left);

						this->removedsuccessor = !this->removedsuccessor;

						sub->key = next->key;
						sub->value = next->value;

						RemoveRecursive(next, next->key);
					}
					else if(sub->left != nullptr)
					{
						// only left child
						this->ReplaceNode(sub, sub->left);
					}
					else if(sub->right != nullptr)
					{
						// only right child
						this->ReplaceNode(sub, sub->right);
					}
					else
					{
						// no children
						this->ReplaceNode(sub, nullptr);
					}
				}
			}


			bool removedsuccessor = false;
			Node* root;
			uint64_t items;
		public:

			BinarySearchTree()
			{
				this->root = nullptr;
				this->items = 0;
			}

			virtual ~BinarySearchTree()
			{
			}

			V* Search(K k)
			{
				return this->SearchRecursive(this->root, k);
			}

			virtual void Insert(K k, V v)
			{
				Node* n = new Node(k, v);
				InsertRecursive(this->root, n);
				this->items++;
			}

			virtual void Remove(K k)
			{
				this->RemoveRecursive(this->root, k);
			}

			virtual V* Fetch(K k)
			{
				V* ret = this->Search(k);
				this->RemoveRecursive(this->root, k);

				return ret;
			}

			V* Minimum()
			{
				return &this->Minimum(this->root)->value;
			}

			V* Maximum()
			{
				return &this->Maximum(this->root)->value;
			}

			LinkedList<V>* Items()
			{
				LinkedList<V>* ret = new LinkedList<V>();
				this->TraverseRecursive(ret, this->root);

				return ret;
			}

			uint64_t Size()
			{
				return this->items;
			}
	};
}














