######################################################################################
#
# Run runHLT_standalone.py from TriggerJobOpts offline to compare with results
# from AthenaMT/PT: run on RDO
#
######################################################################################


# does not use RecExCommon_flags!

if not ('EvtMax' in dir()):
   EvtMax = 10



ReadBS=False

include("TriggerJobOpts/runHLT_standalone.py")

MessageSvc.Format = "% F%48W%S%7W%R%T %0W%M"

print AlgSequence
print ServiceMgr

del EvtMax


