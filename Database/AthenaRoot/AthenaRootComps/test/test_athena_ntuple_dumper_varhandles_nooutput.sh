#!/bin/bash

set -e

testname=test_athena_ntuple_dumper_varhandles_nooutput
rm -rf $testname
mkdir $testname
cd $testname

IIMAX=5
if [[ "$CMTCONFIG" == *-dbg* ]]; then
    IIMAX=1
    echo "::: setting IIMAX=1 (debug mode is slower)"
else
    IIMAX=5
    echo "::: setting IIMAX=5 (opt mode is faster)"
fi


ATLAS_REFERENCE_TAG=AthenaRootComps/AthenaRootCompsReference-01-00-01
refbase=ref.d3pd.ascii
chkfile=d3pd.ascii

reffile=$refbase
get_files -data -symlink $reffile > /dev/null
if [ ! -r $reffile ]; then
  # Look for the file in DATAPATH.
  # We have to look for the directory, not the file itself,
  # since get_files is hardcoded not to look more than two
  # levels down.
  get_files -data -symlink $ATLAS_REFERENCE_TAG > /dev/null
  reffile=`basename $ATLAS_REFERENCE_TAG`/$refbase
  if [ ! -r $reffile ]; then
      testdata=$ATLAS_REFERENCE_DATA
      if [ "$testdata" = "" ]; then
          testdata=/afs/cern.ch/atlas/maxidisk/d33/referencefiles
      fi
      reffile=$testdata/$ATLAS_REFERENCE_TAG/$refbase
  fi
fi

echo "::::::::::::::::::::::::::::::::::::::::::::::"
echo "::: run athena-ntuple-dumper... (w/ varhandles, w/o output)"
time_cmd="/usr/bin/time -a -o d3pd.rw.timing.log"
/bin/rm -rf d3pd.rw.timing.log >& /dev/null
for ii in `seq $IIMAX`; do
    echo " - iter $ii/$IIMAX..."
    ($time_cmd athena.py -c'USEVARHANDLE=1; DOWRITE=0' -lERROR AthenaRootComps/test_athena_ntuple_dumper.py >& d3pd.001.$ii.log.txt) || exit 1
done
grep "user" d3pd.rw.timing.log
echo "::: comparing ascii dumps..."
/bin/rm -f d3pd.ascii.diff
diff -urN $reffile $chkfile >& d3pd.ascii.diff
sc=$?
echo "::: comparing ascii dumps...[$sc]"

if [ $sc -ne 0 ]; then
    exit $sc
fi
