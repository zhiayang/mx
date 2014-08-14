// AVLTree.hpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.


#include <stdint.h>
#include "BinarySearchTree.hpp"
#pragma once

namespace Library
{
	template<class K, class V>
	class AVLTree : public BinarySearchTree<K, V>
	{
		public:
			typedef typename BinarySearchTree<K, V>::Node Node;
			AVLTree()
			{
				this->root = nullptr;
				this->items = 0;
			}

		private:
	};
}
