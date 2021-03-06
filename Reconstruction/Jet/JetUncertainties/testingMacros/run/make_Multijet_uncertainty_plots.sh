#!/bin/sh
exec=bin/multi
code=code/MakeMultijetPlots.C
mkdir -p bin

flags=$($ROOTSYS/bin/root-config --cflags --glibs)
includes="-I../ -L../StandAlone -lJetUncertainties"

#echo ; echo "Will compile the JESUncertaintyProvider"
cd ../cmt
make -f Makefile.StandAlone
cd -

export LD_LIBRARY_PATH=../StandAlone:$LD_LIBRARY_PATH

echo ; echo "Will compile $code"
g++ -o $exec $code $flags $includes && {
    echo "Compilation successful!"
    echo
    echo "Will now run..."
    ./$exec 
} || {
    echo "Compilation failed :("
    echo
}
