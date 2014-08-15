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

#include "caffe/util/normalize.hpp"

using namespace caffe;  // NOLINT(build/namespaces)
using std::pair;
using std::string;

int main(int argc, char** argv) {
  ::google::InitGoogleLogging(argv[0]);
  
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  LOG(INFO) << "Opening leveldb " << argv[1];
  leveldb::Status status = leveldb::DB::Open(
      options, argv[1], &db);
  CHECK(status.ok()) << "Failed to open leveldb " << argv[1];
  Datum datum;

  leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    //cout << it->key().ToString() << ": "  << it->value().ToString() << endl;
    datum.ParseFromString(it->value().ToString());
    const string key_str = it->key().ToString();
    NormalizeDatumImage( &datum, key_str, 50, 25);
    display_datum( &datum);
  }
  assert(it->status().ok());  // Check for any errors found during the scan

  delete it;
  delete db;
  return 0;
}
