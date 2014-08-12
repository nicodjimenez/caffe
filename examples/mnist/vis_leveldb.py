#!/usr/bin/env python 
import cv2 
import numpy as np 
import leveldb 
import sys
import json

from hand2tex.common.util import show_img
from hand2tex.data.ocr_data.manager.data_manager import DataManager
from hand2tex.data.ocr_data.manager.process_char import get_portrait
from caffe.io import caffe_pb2

LABEL_TO_IND_JSON = "label_to_ind.json" 

def vis_db(db_dir):
    # note: assumes square images only
    with open(LABEL_TO_IND_JSON,'r') as f:
        label_to_ind = json.load(f)
	sym_list = label_to_ind.keys()
        
    h = leveldb.LevelDB(db_dir)
    datum = caffe_pb2.Datum()
    for key_val,ser_str in h.RangeIter():
        datum.ParseFromString(ser_str)
        #print "Label int: ", datum.label
	#print "Sym list: ", sym_list
        #print "\nKey val: ", key_val
	print "Label: ", sym_list[int(datum.label)]
        img_pre = np.fromstring(datum.data,dtype=np.uint8)
	img_dim = np.sqrt(len(img_pre))
        img = img_pre.reshape(img_dim,img_dim)
        show_img(img,"img")

def main():
    try:    
        db_dir = sys.argv[1]
        vis_db(db_dir)
    except KeyboardInterrupt:
        print "Bye bye!"

if __name__ == "__main__":
    main()
