#!/bin/bash

# updates the build number in root/.build.h
string=$(cat .build.h)
tokens=($string)
buildnum=${tokens[2]}

let "buildnum += 1"
echo "#define X_BUILD_NUMBER $buildnum" > .build.h
