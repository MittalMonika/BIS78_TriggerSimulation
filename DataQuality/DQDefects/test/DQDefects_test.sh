#!/bin/bash

NOSEPATH=`python -c 'from __future__ import print_function; import nose; f=nose.__file__; i=f.find("/lib/"); print (f[0:i])'`
$NOSEPATH/bin/nosetests -v DQDefects