USE_CAFFE=$(cat ./use_caffe.txt)
# copy json files to current directory
cp $USE_CAFFE/*.json ./
echo "Copied JSON files to current directory."
