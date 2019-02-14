#!/bin/sh

athena --config-only=bootstrap_test.pkl AthenaConfiguration/zeroJO.py &> /dev/null
get_files -remove -data bootstrap.pkl
confTool.py --diff bootstrap.pkl bootstrap_test.pkl | grep -v identical
athena --threads=1 --config-only=bootstrap_threaded_test.pkl AthenaConfiguration/zeroJO.py &> /dev/null
get_files -remove -data bootstrap_threaded.pkl
confTool.py --diff bootstrap_threaded.pkl bootstrap_threaded_test.pkl | grep -v identical
