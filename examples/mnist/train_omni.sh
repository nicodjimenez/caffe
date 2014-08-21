#!/usr/bin/env sh
echo "Working directory is $PWD"
module load cuda55/blas/5.5.22
module load cuda55/toolkit/5.5.22
module load cuda55/tdk/5.319.43
echo "Loaded modules."
echo "Hardware: "
lspci -nn | grep NVIDIA

TOOLS=../../build/tools
#GLOG_logtostderr=1 $TOOLS/train_net.bin omni_solver.prototxt 2> omni.log
GLOG_logtostderr=1 $TOOLS/train_net.bin omni_solver.prototxt 
cat omni.log | mail -s "Omni training" njimene2@stanford.edu
