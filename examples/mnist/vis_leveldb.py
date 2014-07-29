#!/usr/bin/env python 
import cv2 
import numpy as np 
import leveldb 
import sys
from hand2tex.common.util import show_img
from hand2tex.data.ocr_data.manager.data_manager import DataManager
from hand2tex.data.ocr_data.manager.process_char import get_portrait
from caffe.io import caffe_pb2

def vis_db(db_dir,img_dim):
    h = leveldb.LevelDB(db_dir)
    datum = caffe_pb2.Datum()
    for key_val,ser_str in h.RangeIter():
        print "Key val: ", key_val
        datum.ParseFromString(ser_str)
        print "Label: "
        print datum.label
        img_pre = np.fromstring(datum.data,dtype=np.uint8)
        img = img_pre.reshape(img_dim,img_dim)
        show_img(img,"img")

def main():
    try:    
        db_dir = sys.argv[1]
        img_dim = int(sys.argv[2])
        vis_db(db_dir,img_dim)
    except KeyboardInterrupt:
        print "Bye bye!"

def debug():
    vis_db("omni-train-leveldb",28)
    
if __name__ == "__main__":
    main()