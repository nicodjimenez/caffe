TOOLS=../../build/tools
latest_file=$(ls -t omni*.solverstate | head -1)
GLOG_logtostderr=1 $TOOLS/train_net.bin omni_solver.prototxt $latest_file
