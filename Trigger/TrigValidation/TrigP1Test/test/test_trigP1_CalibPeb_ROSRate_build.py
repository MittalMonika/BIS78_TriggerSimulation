#!/usr/bin/env python

# art-description: A version of the CalibPeb test including ROS rate simulation
# art-type: build
# art-include: master/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Step

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TrigExPartialEB/MTCalibPeb.py'
ex.input = 'data'
ex.args = '--ros2rob /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/ATLASros2rob2018-r22format.py'
ex.perfmon = False # perfmon with athenaHLT doesn't work at the moment

ros2json = CheckSteps.InputDependentStep("RosRateToJson")
ros2json.executable = 'ros-hitstats-to-json.py'
ros2json.input_file = 'ros_hitstats_reject.txt'
ros2json.output_stream = Step.Step.OutputStream.STDOUT_ONLY

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)
test.check_steps.append(ros2json)

import sys
sys.exit(test.run())