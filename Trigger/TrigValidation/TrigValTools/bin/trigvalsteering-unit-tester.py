#!/usr/bin/env python
#
# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

import sys
import os
import argparse
import subprocess
import logging


def main():
    logging.basicConfig(stream=sys.stdout,
                        format='TrigValSteeringUT %(levelname)-8s %(message)s',
                        level=logging.INFO)

    parser = argparse.ArgumentParser(usage='%(prog)s PATH [PATH] ...')
    parser.add_argument('paths',
                        metavar='PATH',
                        nargs='+',
                        help='Path containing test python scripts to test')
    args = parser.parse_args()

    n_failed = 0
    for p in args.paths:
        if not os.path.exists(p):
            logging.error('Path %s does not exist', p)
            return 1
        tests = [f for f in os.listdir(p) if f.startswith('test_') and f.endswith('.py')]
        logging.info('Testing %d test scripts from path %s', len(tests), p)
        for test in tests:
            cmd = 'TRIGVALSTEERING_DRY_RUN=1 {:s}'.format(p+'/'+test)
            log_file = '{:s}.unitTest.log'.format(test)
            cmd += ' >{:s} 2>&1'.format(log_file)
            ret_code = subprocess.call(cmd, shell=True)
            status_str = 'OK'
            if ret_code != 0:
                status_str = 'FAILED WITH CODE {:d}'.format(ret_code)
            # Grep for error messages but ignore missing reference errors
            grep_cmd = 'grep ERROR {:s} | grep -v "Missing reference" >/dev/null 2>&1'.format(log_file)
            grep_code = subprocess.call(grep_cmd, shell=True)
            if grep_code == 0:
                status_str = 'ERROR IN LOG {:s}'.format(log_file)
            if status_str != 'OK':
                n_failed += 1
            logging.info('---- %s ---- %s', test, status_str)
    return n_failed


if "__main__" in __name__:
    sys.exit(main())
