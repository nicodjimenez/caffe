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

  char* format_str = "%08d_%s";
  if (argv[5][0] == '1') {
    // change key string formatting
    LOG(ERROR) << "Adding testing tag to key strings.";
    format_str = "T%08d_%s";
  }
  else{
    LOG(ERROR) << "Not testing tag to key strings.";
  }
  
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
  for (int line_id = 0; line_id < lines.size(); ++line_id) {
    if (!ReadRawImageToDatum(root_folder + lines[line_id].first,
         lines[line_id].second, &datum)) {
      continue;
    }
    // sequential
    snprintf(key_cstr, kMaxKeyLength, format_str, line_id,
        lines[line_id].first.c_str());
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
