#!/bin/bash

# Available options
CC=clang
CXX=clang++
BUILDTESTS=OFF

# Get all options
for i in "$@"
do
case $i in
    --cc=*)
    CC="${i#*=}"
    shift # past argument=value
    ;;
    --cxx=*)
    CXX="${i#*=}"
    shift # past argument=value
    ;;
    --build-tests)
    BUILDTESTS=ON
    shift # past argument with no value
    ;;
    *)
          # unknown option
    ;;
esac
done
echo "CC COMPILER  = ${CC}"
echo "CXX COMPILER = ${CXX}"
echo "BUILD TESTS  = ${BUILDTESTS}"

# The directory location of this script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

# Make the build directory
mkdir -p $DIR/build
cd $DIR/build

# Build with the specified options
cmake -DBUILD_TESTS=$BUILDTESTS -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX ..
CORES=`grep -c ^processor /proc/cpuinfo`
make -j$CORES