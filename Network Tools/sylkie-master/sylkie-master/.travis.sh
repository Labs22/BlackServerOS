#!/bin/bash

if [ -z "${GEN}" -o -z "${BUILD_TYPE}" ]
then
    echo "Must define GEN and BUILD_TYPE"
    exit 1
fi

mkdir ./build
cd build

if [ -d /usr/src/gtest ]
then
    cp -r /usr/src/gtest ./gtest
    cd ./gtest
    cmake -DBUILD_SHARED_LIBS=ON .
    make
    cd ..
    ln -svf ./gtest/libgtest.so ./libgtest.so
else
    echo "libgtest-dev must be installed to use this script"
    exit 1
fi

cmake -G "${GEN}" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DENABLE_TESTS=ON ..

if [[ "${GEN}" == "Ninja" ]]
then
    ninja
else
    make
fi

if [[ ${BUILD_TYPE} == "ASAN" ]]
then
    LD_PRELOAD=libasan.so ./test_runner
else
    ./test_runner
fi
