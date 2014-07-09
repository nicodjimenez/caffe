#!/usr/bin/env sh
# This script converts the mnist data into leveldb format.

UTIL=../../build/tools
USE_CAFFE=../../../hand2tex/use_caffe

echo "Creating leveldb..."

rm -rf omni-train-leveldb
rm -rf omni-test-leveldb

$USE_CAFFE/update_level_db.py
num_output=$(./echo_num_output.py $USE_CAFFE/caffe_sym_to_num.json) 
for file in $(ls omni*.prototxt); do
    tail -r $file | gsed -r "0,/(.*)()(num_output: )([0-9]+)/s//\1\2\3$num_output/g" | tail -r > tmp
    mv tmp $file
done

$UTIL/convert_imageset.bin -g / $USE_CAFFE/listfile_train.txt omni-train-leveldb 0  
$UTIL/convert_imageset.bin -g / $USE_CAFFE/listfile_test.txt omni-test-leveldb 0 
echo "Done."
