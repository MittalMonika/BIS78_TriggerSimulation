####################
## File configLumi_Run183003_9LB.py: autogenerated configuration file from command
##/afs/cern.ch/user/a/ayana/scratch0/devval/InstallArea/share/bin/RunDepTaskMaker.py --externalDict={3.75:.0071, 4.25:.0922, 4.75:.1328, 5.25:.1997, 5.75:.2001, 6.25:.1667, 6.75:.1314, 7.25:.0561, 7.75:.0139} --lumimethod=EXTERNAL --outfile=configLumi_Run183003_9LB.py nofeb.xml
## Created on Thu Jun 30 19:55:58 2011
####################
#Run-dependent digi job configuration file.
#SimuJobTransforms/ConfigLumi_preOptions.py

#We need to be able to adjust for different dataset sizes.
if not 'ScaleTaskLength' in dir():   ScaleTaskLength = 1
_evts = lambda x: int(ScaleTaskLength * x)

digilog = logging.getLogger('Digi_trf')
digilog.info('doing RunLumiOverride configuration from file.')
JobMaker=[
   {'run':183003, 'lb':177, 'starttstamp':1306973316, 'dt':0.000, 'evts':_evts(71), 'mu':3.750, 'force_new':False},
   {'run':183003, 'lb':178, 'starttstamp':1306973377, 'dt':0.000, 'evts':_evts(922), 'mu':4.250, 'force_new':False},
   {'run':183003, 'lb':179, 'starttstamp':1306973438, 'dt':0.000, 'evts':_evts(1314), 'mu':6.750, 'force_new':False},
   {'run':183003, 'lb':180, 'starttstamp':1306973499, 'dt':0.000, 'evts':_evts(1328), 'mu':4.750, 'force_new':False},
   {'run':183003, 'lb':181, 'starttstamp':1306973560, 'dt':0.000, 'evts':_evts(561), 'mu':7.250, 'force_new':False},
   {'run':183003, 'lb':182, 'starttstamp':1306973621, 'dt':0.000, 'evts':_evts(139), 'mu':7.750, 'force_new':False},
   {'run':183003, 'lb':183, 'starttstamp':1306973682, 'dt':0.000, 'evts':_evts(2001), 'mu':5.750, 'force_new':False},
   {'run':183003, 'lb':184, 'starttstamp':1306973743, 'dt':0.000, 'evts':_evts(1667), 'mu':6.250, 'force_new':False},
   {'run':183003, 'lb':185, 'starttstamp':1306973779, 'dt':0.000, 'evts':_evts(1997), 'mu':5.250, 'force_new':False},
#--> end hiding
]

include('RunDependentSimData/configCommon.py')

del JobMaker
