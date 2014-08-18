// Copyright 2014 BVLC and contributors.
// This program converts a set of images to a leveldb by storing them as Datum
// proto buffers.
// Usage:
//   convert_imageset [-g] ROOTFOLDER/ LISTFILE DB_NAME RANDOM_SHUFFLE[0 or 1] \
//                     [imSize] [charSize]
// where ROOTFOLDER is the root folder that holds all the images, and LISTFILE
// should be a list of files as well as their labels, in the format as
//   subfolder1/file1.JPEG 7
//   ....
// if RANDOM_SHUFFLE is 1, a random shuffle will be carried out before we
// process the file lines.
// Optional flag -g indicates the images should be read as
// single-channel grayscale. If omitted, grayscale images will be
// converted to color.

#include <glog/logging.h>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include <algorithm>
#include <fstream>  // NOLINT(readability/streams)
#include <string>
#include <utility>
#include <vector>

#include "caffe/proto/caffe.pb.h"
#include "caffe/util/io.hpp"

using namespace caffe;  // NOLINT(build/namespaces)
using std::pair;
using std::string;

int main(int argc, char** argv) {
  ::google::InitGoogleLogging(argv[0]);
  if (argc != 6) {
    printf("Convert a set of images to the leveldb format used\n"
        "as input for Caffe.\n"
        "Usage:\n"
        "    convert_imageset ROOTFOLDER/ LISTFILE DB_NAME"
        " RANDOM_SHUFFLE_DATA[0 or 1] ADD_TEST_TAG[0 or 1]\n");
    return 1;
  }
  
  std::ifstream infile(argv[2]);
  std::vector<std::pair<string, int> > lines;
  string filename;
  int label;
  while (infile >> filename >> label) {
    lines.push_back(std::make_pair(filename, label));
  }
  if (argv[4][0] == '1') {
    // randomly shuffle data
    LOG(INFO) << "Shuffling data.";
    std::random_shuffle(lines.begin(), lines.end());
  }

  bool is_train = false;
  if (argv[5][0] == '1') 
    is_train = true;
  LOG(ERROR) << "Is training data: " << is_train;
  LOG(INFO) << "A total of " << lines.size() << " images.";
  leveldb::DB* db;
  leveldb::Options options;
  options.error_if_exists = true;
  options.create_if_missing = true;
  options.write_buffer_size = 268435456;
  LOG(INFO) << "Opening leveldb " << argv[3];
  leveldb::Status status = leveldb::DB::Open(
      options, argv[3], &db);
  CHECK(status.ok()) << "Failed to open leveldb " << argv[3];

  string root_folder(argv[1]);
  Datum datum;
  int count = 0;
  const int kMaxKeyLength = 256;
  char key_cstr[kMaxKeyLength];
  leveldb::WriteBatch* batch = new leveldb::WriteBatch();
  // some substrings that we want to look for to instruct how to augment dataset
  string inkml_substr = "inkml";
  string mnist_substr = "mnist";
  string file_name;
  for (int line_id = 0; line_id < lines.size(); ++line_id) {
    file_name = lines[line_id].first;
    if (!ReadRawImageToDatum(root_folder + file_name, lines[line_id].second, &datum)) {
      continue;
    }

  // set optional parameters
  if (is_train)
    datum.set_is_train(true);
  else
    datum.set_is_train(false);

  if (file_name.find(inkml_substr) != string::npos)
    datum.set_is_inkml(true);
  else
    datum.set_is_inkml(false);
    
  if (file_name.find(mnist_substr) != string::npos) 
    datum.set_is_mnist(true);
  else
    datum.set_is_mnist(false);
    
    // sequential
    snprintf(key_cstr, kMaxKeyLength, "%08d_%s", line_id, lines[line_id].first.c_str());
    string value;
    // get the value
    datum.SerializeToString(&value);
    batch->Put(string(key_cstr), value);
    if (++count % 1000 == 0) {
      db->Write(leveldb::WriteOptions(), batch);
      LOG(ERROR) << "Processed " << count << " files.";
      delete batch;
      batch = new leveldb::WriteBatch();
    }
  }
  // write the last batch
  if (count % 1000 != 0) {
    db->Write(leveldb::WriteOptions(), batch);
    LOG(ERROR) << "Processed " << count << " files.";
  }

  delete batch;
  delete db;
  return 0;
}
