####################
## File configLumi_run305000.py: autogenerated configuration file from command
##../../RunDependentSimComps/share/RunDepProfileGenerator.py -r 305000 -s 1525000000 -c 2000 -o configLumi_run305000.py -e {0.25:0.031,0.75:0.051,1.25:0.153,1.75:0.303,2.25:0.261,2.75:0.077,3.25:0.054,3.75:0.039,4.25:0.024,4.75:0.008,5.25:0.001}
## Created on Tue Dec 12 16:13:30 2017
####################
#Run-dependent digi job configuration file.
#RunDependentSimData/OverrideRunLBLumiDigitConfig.py

#We need to be able to adjust for different dataset sizes.
if not 'ScaleTaskLength' in dir():   ScaleTaskLength = 1
_evts = lambda x: int(ScaleTaskLength * x)

if not 'logging' in dir(): import logging
digilog = logging.getLogger('Digi_trf')
digilog.info('doing RunLumiOverride configuration from file.')
JobMaker=[
  {'run':305000, 'lb':1, 'starttstamp':1525000000, 'dt':0.000, 'evts':_evts(62), 'mu':0.250, 'force_new':False},
  {'run':305000, 'lb':2, 'starttstamp':1525000060, 'dt':0.000, 'evts':_evts(102), 'mu':0.750, 'force_new':False},
  {'run':305000, 'lb':3, 'starttstamp':1525000120, 'dt':0.000, 'evts':_evts(305), 'mu':1.250, 'force_new':False},
  {'run':305000, 'lb':4, 'starttstamp':1525000180, 'dt':0.000, 'evts':_evts(604), 'mu':1.750, 'force_new':False},
  {'run':305000, 'lb':5, 'starttstamp':1525000240, 'dt':0.000, 'evts':_evts(521), 'mu':2.250, 'force_new':False},
  {'run':305000, 'lb':6, 'starttstamp':1525000300, 'dt':0.000, 'evts':_evts(154), 'mu':2.750, 'force_new':False},
  {'run':305000, 'lb':7, 'starttstamp':1525000360, 'dt':0.000, 'evts':_evts(108), 'mu':3.250, 'force_new':False},
  {'run':305000, 'lb':8, 'starttstamp':1525000420, 'dt':0.000, 'evts':_evts(78), 'mu':3.750, 'force_new':False},
  {'run':305000, 'lb':9, 'starttstamp':1525000480, 'dt':0.000, 'evts':_evts(48), 'mu':4.250, 'force_new':False},
  {'run':305000, 'lb':10, 'starttstamp':1525000540, 'dt':0.000, 'evts':_evts(16), 'mu':4.750, 'force_new':False},
  {'run':305000, 'lb':11, 'starttstamp':1525000600, 'dt':0.000, 'evts':_evts(2), 'mu':5.250, 'force_new':False},
#--> end hiding
]

include('RunDependentSimData/configCommon.py')

#cleanup python memory
if not "RunDMC_testing_configuration" in dir():
    del JobMaker
