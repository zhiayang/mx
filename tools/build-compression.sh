#!/bin/bash

git clone https://github.com/atomicobject/heatshrink ../build/heatshrink

cd ../build/heatshrink
make heatshrink

mv heatshrink ../../tools/heatshrink
cd ../../tools
