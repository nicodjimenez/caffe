#!/usr/bin/env sh
# This script converts the mnist data into leveldb format.

UTIL=../../build/tools
USE_CAFFE=../../../hand2tex/use_caffe

echo "Deleting old leveldb..."

#rm -rf omni-train-leveldb
#rm -rf omni-test-leveldb
rm -rf db_omni 
mkdir db_omni 

echo "Creating leveldb for testing set..."
$UTIL/convert_imageset.bin / $USE_CAFFE/listfile_test.txt db_omni/omni-test-leveldb 0 0

echo "Creating leveldb for training set..."
$UTIL/convert_imageset.bin / $USE_CAFFE/listfile_train.txt db_omni/omni-train-leveldb 0 1

echo "Done."
