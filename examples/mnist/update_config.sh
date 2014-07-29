#!/usr/bin/env sh
USE_CAFFE=$(cat ./use_caffe.txt)
$USE_CAFFE/update_level_db.py
# copy json files to current directory
cp $USE_CAFFE/*.json ./
echo "Copied JSON files to current directory."

num_output=$(./echo_num_output.py)
for file in $(ls omni*.prototxt); do
    tail -r $file | gsed -r "0,/(.*)()(num_output: )([0-9]+)/s//\1\2\3$num_output/g" | tail -r > tmp
    mv tmp $file
done
echo "Edited config files to match label ct."
echo "Done."
