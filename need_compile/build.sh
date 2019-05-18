#!/bin/bash

DIR=`dirname ${BASH_SOURCE[0]}`

pushd $PWD
cd $DIR
echo "Building in path: $PWD"
CMD="g++ need_compile.cpp -lstdc++fs"
echo "$CMD"
$CMD
if [ $? -ne 0 ]
then
	echo "FAILED"
else
	echo "SUCCESS"
fi
popd

