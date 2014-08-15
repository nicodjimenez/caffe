#!/bin/bash
qsub -d /home/njimene2/caffe/examples/mnist -o myOut -e myErr train_omni.sh
 
#cd $HOME/caffe/examples/mnist
#./train_omni.sh
