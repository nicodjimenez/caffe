#!/usr/bin/env sh
# This script converts the mnist data into leveldb format.

UTIL=../../build/tools
USE_CAFFE=../../../hand2tex/use_caffe

echo "Deleting old leveldb..."

rm -rf omni-train-leveldb
rm -rf omni-test-leveldb

#./update_config.sh
echo "Creating leveldb for testing set..."
$UTIL/convert_imageset.bin / $USE_CAFFE/listfile_test.txt omni-test-leveldb 0 1

echo "Creating leveldb for training set..."
$UTIL/convert_imageset.bin / $USE_CAFFE/listfile_train.txt omni-train-leveldb 0 0

echo "Done."
