#!/usr/bin/env python 
'''
Created on Jul 16, 2014

@author: nicodjimenez
'''
import numpy as np 
import leveldb 
from hand2tex.data.ocr_data.manager.data_manager import DataManager
from hand2tex.data.ocr_data.manager.process_char import get_portrait
from caffe.io import caffe_pb2

def save_mnist(db_dir_list=["mnist-test-leveldb","mnist-train-leveldb"]):
    # takes all mnist data and inserts it into binarized character directory 
    data_manager = DataManager()
    for db_dir in db_dir_list:
        h = leveldb.LevelDB(db_dir)
        datum = caffe_pb2.Datum()
        for key_val,ser_str in h.RangeIter():
            datum.ParseFromString(ser_str)
            char_str = str(datum.label)
            img_pre = np.fromstring(datum.data,dtype=np.uint8)
            img = img_pre.reshape(28,28)
            img = 255 - img
            portrait_img = get_portrait(img,expand_factor=2)
            file_name = "mnist_" + str(key_val) + ".png"
            data_manager.save_char_img(portrait_img, file_name, char_str, phase=1)
            #show_img(img,"img")
            
if __name__ == "__main__": 
    save_mnist()