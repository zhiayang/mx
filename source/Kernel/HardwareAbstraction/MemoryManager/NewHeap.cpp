// NewHeap.cpp
// Copyright (c) 2014 - The Foreseeable Future, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

// With reference to "github.com/thePowersGang/rust_os/Kernel/Core/memory/heap.rs"
// Originally by John Hodge (thePowersGang)

#include <Kernel.hpp>
#include <Memory.hpp>
#include <StandardIO.hpp>
#include <String.hpp>

using namespace Kernel;
using namespace Library;
using namespace Kernel::HardwareAbstraction::MemoryManager;


