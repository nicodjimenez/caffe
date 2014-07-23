#!/usr/bin/env python
import json 
import sys

file_name = "leveldb_stats.json"
with open(file_name,'r') as f:
	leveldb_stats = json.load(f)
	label_ct = leveldb_stats["label_ct"]
	print label_ct
