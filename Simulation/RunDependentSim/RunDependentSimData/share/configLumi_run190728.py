####################
## File configLumi.py: autogenerated configuration file from command
##/afs/cern.ch/user/a/ayana/scratch0/MC11/InstallArea/share/bin/RunDepTaskMaker.py --nMC=5010 --trigger=L1_MBTS_2 run190728.xml
## Created on Thu Oct 27 00:41:58 2011
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
   {'run':190728, 'lb':108, 'starttstamp':1318253359, 'dt':62.864, 'evts':_evts(27), 'mu':29.750, 'force_new':False},
   {'run':190728, 'lb':110, 'starttstamp':1318253481, 'dt':61.273, 'evts':_evts(27), 'mu':29.533, 'force_new':False},
   {'run':190728, 'lb':111, 'starttstamp':1318253542, 'dt':60.922, 'evts':_evts(26), 'mu':29.402, 'force_new':False},
   {'run':190728, 'lb':112, 'starttstamp':1318253603, 'dt':60.284, 'evts':_evts(26), 'mu':29.371, 'force_new':False},
   {'run':190728, 'lb':113, 'starttstamp':1318253663, 'dt':61.105, 'evts':_evts(26), 'mu':29.146, 'force_new':False},
   {'run':190728, 'lb':114, 'starttstamp':1318253724, 'dt':61.013, 'evts':_evts(26), 'mu':29.081, 'force_new':False},
   {'run':190728, 'lb':115, 'starttstamp':1318253785, 'dt':11.457, 'evts':_evts(5), 'mu':28.804, 'force_new':False},
   {'run':190728, 'lb':116, 'starttstamp':1318253797, 'dt':60.454, 'evts':_evts(26), 'mu':28.947, 'force_new':False},
   {'run':190728, 'lb':117, 'starttstamp':1318253857, 'dt':61.061, 'evts':_evts(26), 'mu':28.921, 'force_new':False},
   {'run':190728, 'lb':118, 'starttstamp':1318253918, 'dt':61.114, 'evts':_evts(26), 'mu':28.856, 'force_new':False},
   {'run':190728, 'lb':119, 'starttstamp':1318253979, 'dt':61.476, 'evts':_evts(26), 'mu':28.863, 'force_new':False},
   {'run':190728, 'lb':120, 'starttstamp':1318254041, 'dt':60.638, 'evts':_evts(26), 'mu':28.874, 'force_new':False},
   {'run':190728, 'lb':121, 'starttstamp':1318254101, 'dt':60.721, 'evts':_evts(26), 'mu':28.999, 'force_new':False},
   {'run':190728, 'lb':122, 'starttstamp':1318254162, 'dt':11.035, 'evts':_evts(5), 'mu':29.075, 'force_new':False},
   {'run':190728, 'lb':123, 'starttstamp':1318254173, 'dt':61.021, 'evts':_evts(26), 'mu':28.999, 'force_new':False},
   {'run':190728, 'lb':124, 'starttstamp':1318254234, 'dt':61.059, 'evts':_evts(26), 'mu':28.896, 'force_new':False},
   {'run':190728, 'lb':125, 'starttstamp':1318254295, 'dt':60.922, 'evts':_evts(26), 'mu':28.865, 'force_new':False},
   {'run':190728, 'lb':126, 'starttstamp':1318254356, 'dt':61.023, 'evts':_evts(26), 'mu':28.846, 'force_new':False},
   {'run':190728, 'lb':127, 'starttstamp':1318254417, 'dt':10.946, 'evts':_evts(5), 'mu':28.816, 'force_new':False},
   {'run':190728, 'lb':128, 'starttstamp':1318254428, 'dt':61.021, 'evts':_evts(26), 'mu':28.897, 'force_new':False},
   {'run':190728, 'lb':129, 'starttstamp':1318254489, 'dt':61.332, 'evts':_evts(26), 'mu':28.828, 'force_new':False},
   {'run':190728, 'lb':130, 'starttstamp':1318254550, 'dt':60.809, 'evts':_evts(26), 'mu':28.748, 'force_new':False},
   {'run':190728, 'lb':131, 'starttstamp':1318254611, 'dt':60.804, 'evts':_evts(26), 'mu':28.701, 'force_new':False},
   {'run':190728, 'lb':132, 'starttstamp':1318254672, 'dt':61.041, 'evts':_evts(26), 'mu':28.641, 'force_new':False},
   {'run':190728, 'lb':133, 'starttstamp':1318254733, 'dt':61.128, 'evts':_evts(26), 'mu':28.529, 'force_new':False},
   {'run':190728, 'lb':134, 'starttstamp':1318254794, 'dt':60.813, 'evts':_evts(25), 'mu':28.354, 'force_new':False},
   {'run':190728, 'lb':135, 'starttstamp':1318254855, 'dt':11.022, 'evts':_evts(5), 'mu':28.363, 'force_new':False},
   {'run':190728, 'lb':136, 'starttstamp':1318254866, 'dt':61.238, 'evts':_evts(25), 'mu':28.355, 'force_new':False},
   {'run':190728, 'lb':137, 'starttstamp':1318254927, 'dt':60.883, 'evts':_evts(25), 'mu':28.167, 'force_new':False},
   {'run':190728, 'lb':138, 'starttstamp':1318254988, 'dt':60.927, 'evts':_evts(25), 'mu':28.079, 'force_new':False},
   {'run':190728, 'lb':139, 'starttstamp':1318255049, 'dt':61.226, 'evts':_evts(25), 'mu':27.998, 'force_new':False},
   {'run':190728, 'lb':140, 'starttstamp':1318255110, 'dt':60.702, 'evts':_evts(25), 'mu':27.871, 'force_new':False},
   {'run':190728, 'lb':141, 'starttstamp':1318255171, 'dt':61.137, 'evts':_evts(25), 'mu':27.757, 'force_new':False},
   {'run':190728, 'lb':142, 'starttstamp':1318255232, 'dt':60.965, 'evts':_evts(25), 'mu':27.819, 'force_new':False},
   {'run':190728, 'lb':143, 'starttstamp':1318255293, 'dt':61.127, 'evts':_evts(25), 'mu':27.620, 'force_new':False},
   {'run':190728, 'lb':144, 'starttstamp':1318255354, 'dt':59.259, 'evts':_evts(24), 'mu':27.618, 'force_new':False},
   {'run':190728, 'lb':145, 'starttstamp':1318255414, 'dt':60.529, 'evts':_evts(25), 'mu':27.718, 'force_new':False},
   {'run':190728, 'lb':146, 'starttstamp':1318255474, 'dt':61.055, 'evts':_evts(25), 'mu':27.694, 'force_new':False},
   {'run':190728, 'lb':147, 'starttstamp':1318255535, 'dt':60.999, 'evts':_evts(25), 'mu':27.705, 'force_new':False},
   {'run':190728, 'lb':148, 'starttstamp':1318255596, 'dt':61.125, 'evts':_evts(25), 'mu':27.637, 'force_new':False},
   {'run':190728, 'lb':149, 'starttstamp':1318255657, 'dt':44.502, 'evts':_evts(18), 'mu':27.651, 'force_new':False},
   {'run':190728, 'lb':150, 'starttstamp':1318255702, 'dt':60.376, 'evts':_evts(24), 'mu':27.636, 'force_new':False},
   {'run':190728, 'lb':151, 'starttstamp':1318255762, 'dt':60.764, 'evts':_evts(25), 'mu':27.598, 'force_new':False},
   {'run':190728, 'lb':152, 'starttstamp':1318255823, 'dt':40.313, 'evts':_evts(16), 'mu':27.487, 'force_new':False},
   {'run':190728, 'lb':153, 'starttstamp':1318255863, 'dt':60.919, 'evts':_evts(24), 'mu':27.437, 'force_new':False},
   {'run':190728, 'lb':154, 'starttstamp':1318255924, 'dt':61.004, 'evts':_evts(24), 'mu':27.365, 'force_new':False},
   {'run':190728, 'lb':155, 'starttstamp':1318255985, 'dt':60.926, 'evts':_evts(24), 'mu':27.219, 'force_new':False},
   {'run':190728, 'lb':156, 'starttstamp':1318256046, 'dt':61.012, 'evts':_evts(24), 'mu':27.112, 'force_new':False},
   {'run':190728, 'lb':157, 'starttstamp':1318256107, 'dt':61.069, 'evts':_evts(24), 'mu':27.101, 'force_new':False},
   {'run':190728, 'lb':158, 'starttstamp':1318256168, 'dt':61.248, 'evts':_evts(24), 'mu':27.051, 'force_new':False},
   {'run':190728, 'lb':159, 'starttstamp':1318256229, 'dt':60.707, 'evts':_evts(24), 'mu':27.194, 'force_new':False},
   {'run':190728, 'lb':160, 'starttstamp':1318256290, 'dt':61.005, 'evts':_evts(24), 'mu':27.092, 'force_new':False},
   {'run':190728, 'lb':161, 'starttstamp':1318256351, 'dt':61.003, 'evts':_evts(24), 'mu':27.202, 'force_new':False},
   {'run':190728, 'lb':162, 'starttstamp':1318256412, 'dt':61.002, 'evts':_evts(24), 'mu':27.033, 'force_new':False},
   {'run':190728, 'lb':163, 'starttstamp':1318256473, 'dt':61.010, 'evts':_evts(24), 'mu':27.034, 'force_new':False},
   {'run':190728, 'lb':164, 'starttstamp':1318256534, 'dt':61.004, 'evts':_evts(24), 'mu':26.907, 'force_new':False},
   {'run':190728, 'lb':165, 'starttstamp':1318256595, 'dt':61.004, 'evts':_evts(24), 'mu':26.888, 'force_new':False},
   {'run':190728, 'lb':166, 'starttstamp':1318256656, 'dt':46.117, 'evts':_evts(18), 'mu':26.781, 'force_new':False},
   {'run':190728, 'lb':167, 'starttstamp':1318256702, 'dt':60.888, 'evts':_evts(24), 'mu':26.764, 'force_new':False},
   {'run':190728, 'lb':168, 'starttstamp':1318256763, 'dt':61.017, 'evts':_evts(24), 'mu':26.564, 'force_new':False},
   {'run':190728, 'lb':169, 'starttstamp':1318256824, 'dt':60.915, 'evts':_evts(24), 'mu':26.528, 'force_new':False},
   {'run':190728, 'lb':170, 'starttstamp':1318256885, 'dt':61.005, 'evts':_evts(24), 'mu':26.538, 'force_new':False},
   {'run':190728, 'lb':171, 'starttstamp':1318256946, 'dt':61.008, 'evts':_evts(24), 'mu':26.559, 'force_new':False},
   {'run':190728, 'lb':172, 'starttstamp':1318257007, 'dt':61.054, 'evts':_evts(24), 'mu':26.380, 'force_new':False},
   {'run':190728, 'lb':173, 'starttstamp':1318257068, 'dt':61.030, 'evts':_evts(23), 'mu':26.271, 'force_new':False},
   {'run':190728, 'lb':174, 'starttstamp':1318257129, 'dt':60.918, 'evts':_evts(23), 'mu':26.157, 'force_new':False},
   {'run':190728, 'lb':175, 'starttstamp':1318257190, 'dt':61.005, 'evts':_evts(23), 'mu':26.150, 'force_new':False},
   {'run':190728, 'lb':176, 'starttstamp':1318257251, 'dt':61.003, 'evts':_evts(23), 'mu':26.095, 'force_new':False},
   {'run':190728, 'lb':177, 'starttstamp':1318257312, 'dt':61.003, 'evts':_evts(23), 'mu':26.107, 'force_new':False},
   {'run':190728, 'lb':178, 'starttstamp':1318257373, 'dt':61.004, 'evts':_evts(23), 'mu':26.284, 'force_new':False},
   {'run':190728, 'lb':179, 'starttstamp':1318257434, 'dt':61.003, 'evts':_evts(24), 'mu':26.357, 'force_new':False},
   {'run':190728, 'lb':180, 'starttstamp':1318257495, 'dt':61.002, 'evts':_evts(24), 'mu':26.504, 'force_new':False},
   {'run':190728, 'lb':181, 'starttstamp':1318257556, 'dt':61.009, 'evts':_evts(24), 'mu':26.441, 'force_new':False},
   {'run':190728, 'lb':182, 'starttstamp':1318257617, 'dt':61.020, 'evts':_evts(24), 'mu':26.388, 'force_new':False},
   {'run':190728, 'lb':183, 'starttstamp':1318257678, 'dt':61.010, 'evts':_evts(24), 'mu':26.466, 'force_new':False},
   {'run':190728, 'lb':184, 'starttstamp':1318257739, 'dt':29.070, 'evts':_evts(11), 'mu':26.421, 'force_new':False},
   {'run':190728, 'lb':185, 'starttstamp':1318257768, 'dt':60.943, 'evts':_evts(24), 'mu':26.374, 'force_new':False},
   {'run':190728, 'lb':186, 'starttstamp':1318257829, 'dt':61.005, 'evts':_evts(24), 'mu':26.311, 'force_new':False},
   {'run':190728, 'lb':187, 'starttstamp':1318257890, 'dt':61.002, 'evts':_evts(23), 'mu':26.267, 'force_new':False},
   {'run':190728, 'lb':188, 'starttstamp':1318257951, 'dt':61.001, 'evts':_evts(23), 'mu':26.231, 'force_new':False},
   {'run':190728, 'lb':189, 'starttstamp':1318258012, 'dt':61.001, 'evts':_evts(23), 'mu':26.041, 'force_new':False},
   {'run':190728, 'lb':190, 'starttstamp':1318258073, 'dt':61.003, 'evts':_evts(23), 'mu':25.968, 'force_new':False},
   {'run':190728, 'lb':191, 'starttstamp':1318258134, 'dt':60.903, 'evts':_evts(23), 'mu':26.035, 'force_new':False},
   {'run':190728, 'lb':192, 'starttstamp':1318258195, 'dt':61.071, 'evts':_evts(23), 'mu':26.036, 'force_new':False},
   {'run':190728, 'lb':193, 'starttstamp':1318258256, 'dt':61.009, 'evts':_evts(23), 'mu':25.943, 'force_new':False},
   {'run':190728, 'lb':194, 'starttstamp':1318258317, 'dt':61.003, 'evts':_evts(23), 'mu':26.109, 'force_new':False},
   {'run':190728, 'lb':195, 'starttstamp':1318258378, 'dt':61.005, 'evts':_evts(23), 'mu':26.034, 'force_new':False},
   {'run':190728, 'lb':196, 'starttstamp':1318258439, 'dt':60.962, 'evts':_evts(23), 'mu':25.988, 'force_new':False},
   {'run':190728, 'lb':197, 'starttstamp':1318258500, 'dt':61.022, 'evts':_evts(23), 'mu':25.992, 'force_new':False},
   {'run':190728, 'lb':198, 'starttstamp':1318258561, 'dt':61.004, 'evts':_evts(23), 'mu':25.927, 'force_new':False},
   {'run':190728, 'lb':199, 'starttstamp':1318258622, 'dt':61.007, 'evts':_evts(23), 'mu':25.817, 'force_new':False},
   {'run':190728, 'lb':200, 'starttstamp':1318258683, 'dt':61.006, 'evts':_evts(23), 'mu':25.802, 'force_new':False},
   {'run':190728, 'lb':201, 'starttstamp':1318258744, 'dt':61.004, 'evts':_evts(23), 'mu':25.724, 'force_new':False},
   {'run':190728, 'lb':202, 'starttstamp':1318258805, 'dt':61.005, 'evts':_evts(23), 'mu':25.662, 'force_new':False},
   {'run':190728, 'lb':203, 'starttstamp':1318258866, 'dt':60.901, 'evts':_evts(23), 'mu':25.574, 'force_new':False},
   {'run':190728, 'lb':204, 'starttstamp':1318258927, 'dt':61.021, 'evts':_evts(23), 'mu':25.526, 'force_new':False},
   {'run':190728, 'lb':205, 'starttstamp':1318258988, 'dt':61.013, 'evts':_evts(23), 'mu':25.469, 'force_new':False},
   {'run':190728, 'lb':206, 'starttstamp':1318259049, 'dt':61.046, 'evts':_evts(23), 'mu':25.343, 'force_new':False},
   {'run':190728, 'lb':207, 'starttstamp':1318259110, 'dt':61.013, 'evts':_evts(23), 'mu':25.226, 'force_new':False},
   {'run':190728, 'lb':208, 'starttstamp':1318259171, 'dt':61.004, 'evts':_evts(23), 'mu':25.178, 'force_new':False},
   {'run':190728, 'lb':209, 'starttstamp':1318259232, 'dt':48.485, 'evts':_evts(18), 'mu':25.031, 'force_new':False},
   {'run':190728, 'lb':210, 'starttstamp':1318259281, 'dt':60.459, 'evts':_evts(22), 'mu':25.041, 'force_new':False},
   {'run':190728, 'lb':211, 'starttstamp':1318259341, 'dt':61.021, 'evts':_evts(22), 'mu':25.039, 'force_new':False},
   {'run':190728, 'lb':212, 'starttstamp':1318259402, 'dt':61.032, 'evts':_evts(22), 'mu':25.074, 'force_new':False},
   {'run':190728, 'lb':213, 'starttstamp':1318259463, 'dt':61.020, 'evts':_evts(22), 'mu':25.096, 'force_new':False},
   {'run':190728, 'lb':214, 'starttstamp':1318259524, 'dt':60.903, 'evts':_evts(22), 'mu':25.081, 'force_new':False},
   {'run':190728, 'lb':215, 'starttstamp':1318259585, 'dt':61.023, 'evts':_evts(22), 'mu':25.066, 'force_new':False},
   {'run':190728, 'lb':216, 'starttstamp':1318259646, 'dt':61.001, 'evts':_evts(22), 'mu':25.170, 'force_new':False},
   {'run':190728, 'lb':217, 'starttstamp':1318259707, 'dt':61.013, 'evts':_evts(22), 'mu':25.164, 'force_new':False},
   {'run':190728, 'lb':218, 'starttstamp':1318259768, 'dt':61.019, 'evts':_evts(23), 'mu':25.170, 'force_new':False},
   {'run':190728, 'lb':219, 'starttstamp':1318259829, 'dt':61.017, 'evts':_evts(22), 'mu':25.087, 'force_new':False},
   {'run':190728, 'lb':220, 'starttstamp':1318259890, 'dt':60.930, 'evts':_evts(22), 'mu':25.096, 'force_new':False},
   {'run':190728, 'lb':221, 'starttstamp':1318259951, 'dt':61.018, 'evts':_evts(22), 'mu':25.003, 'force_new':False},
   {'run':190728, 'lb':222, 'starttstamp':1318260012, 'dt':61.001, 'evts':_evts(22), 'mu':24.889, 'force_new':False},
   {'run':190728, 'lb':223, 'starttstamp':1318260073, 'dt':61.009, 'evts':_evts(22), 'mu':24.794, 'force_new':False},
   {'run':190728, 'lb':224, 'starttstamp':1318260134, 'dt':61.021, 'evts':_evts(22), 'mu':24.851, 'force_new':False},
   {'run':190728, 'lb':225, 'starttstamp':1318260195, 'dt':61.006, 'evts':_evts(22), 'mu':24.706, 'force_new':False},
   {'run':190728, 'lb':226, 'starttstamp':1318260256, 'dt':61.009, 'evts':_evts(22), 'mu':24.667, 'force_new':False},
   {'run':190728, 'lb':227, 'starttstamp':1318260317, 'dt':61.016, 'evts':_evts(22), 'mu':24.515, 'force_new':False},
   {'run':190728, 'lb':228, 'starttstamp':1318260378, 'dt':60.901, 'evts':_evts(22), 'mu':24.535, 'force_new':False},
   {'run':190728, 'lb':229, 'starttstamp':1318260439, 'dt':61.004, 'evts':_evts(22), 'mu':24.527, 'force_new':False},
   {'run':190728, 'lb':230, 'starttstamp':1318260500, 'dt':61.021, 'evts':_evts(22), 'mu':24.573, 'force_new':False},
   {'run':190728, 'lb':231, 'starttstamp':1318260561, 'dt':61.039, 'evts':_evts(22), 'mu':24.638, 'force_new':False},
   {'run':190728, 'lb':232, 'starttstamp':1318260622, 'dt':61.035, 'evts':_evts(22), 'mu':24.594, 'force_new':False},
   {'run':190728, 'lb':233, 'starttstamp':1318260683, 'dt':60.902, 'evts':_evts(22), 'mu':24.708, 'force_new':False},
   {'run':190728, 'lb':234, 'starttstamp':1318260744, 'dt':61.017, 'evts':_evts(22), 'mu':24.747, 'force_new':False},
   {'run':190728, 'lb':235, 'starttstamp':1318260805, 'dt':61.004, 'evts':_evts(22), 'mu':24.718, 'force_new':False},
   {'run':190728, 'lb':236, 'starttstamp':1318260866, 'dt':61.023, 'evts':_evts(22), 'mu':24.836, 'force_new':False},
   {'run':190728, 'lb':237, 'starttstamp':1318260927, 'dt':61.005, 'evts':_evts(22), 'mu':25.006, 'force_new':False},
   {'run':190728, 'lb':238, 'starttstamp':1318260988, 'dt':61.052, 'evts':_evts(22), 'mu':25.129, 'force_new':False},
   {'run':190728, 'lb':239, 'starttstamp':1318261049, 'dt':60.933, 'evts':_evts(22), 'mu':25.136, 'force_new':False},
   {'run':190728, 'lb':240, 'starttstamp':1318261110, 'dt':61.103, 'evts':_evts(23), 'mu':25.154, 'force_new':False},
   {'run':190728, 'lb':241, 'starttstamp':1318261171, 'dt':60.911, 'evts':_evts(22), 'mu':25.079, 'force_new':False},
   {'run':190728, 'lb':242, 'starttstamp':1318261232, 'dt':61.003, 'evts':_evts(22), 'mu':25.084, 'force_new':False},
   {'run':190728, 'lb':243, 'starttstamp':1318261293, 'dt':61.002, 'evts':_evts(22), 'mu':24.977, 'force_new':False},
   {'run':190728, 'lb':244, 'starttstamp':1318261354, 'dt':61.002, 'evts':_evts(22), 'mu':24.893, 'force_new':False},
   {'run':190728, 'lb':245, 'starttstamp':1318261415, 'dt':61.005, 'evts':_evts(22), 'mu':24.845, 'force_new':False},
   {'run':190728, 'lb':246, 'starttstamp':1318261476, 'dt':61.004, 'evts':_evts(22), 'mu':24.774, 'force_new':False},
   {'run':190728, 'lb':247, 'starttstamp':1318261537, 'dt':61.003, 'evts':_evts(22), 'mu':24.816, 'force_new':False},
   {'run':190728, 'lb':248, 'starttstamp':1318261598, 'dt':61.019, 'evts':_evts(22), 'mu':24.806, 'force_new':False},
   {'run':190728, 'lb':249, 'starttstamp':1318261659, 'dt':61.006, 'evts':_evts(22), 'mu':24.733, 'force_new':False},
   {'run':190728, 'lb':250, 'starttstamp':1318261720, 'dt':60.954, 'evts':_evts(22), 'mu':24.736, 'force_new':False},
   {'run':190728, 'lb':251, 'starttstamp':1318261781, 'dt':61.023, 'evts':_evts(22), 'mu':24.620, 'force_new':False},
   {'run':190728, 'lb':252, 'starttstamp':1318261842, 'dt':61.001, 'evts':_evts(22), 'mu':24.610, 'force_new':False},
   {'run':190728, 'lb':253, 'starttstamp':1318261903, 'dt':61.003, 'evts':_evts(22), 'mu':24.608, 'force_new':False},
   {'run':190728, 'lb':254, 'starttstamp':1318261964, 'dt':61.002, 'evts':_evts(22), 'mu':24.496, 'force_new':False},
   {'run':190728, 'lb':255, 'starttstamp':1318262025, 'dt':61.009, 'evts':_evts(22), 'mu':24.479, 'force_new':False},
   {'run':190728, 'lb':256, 'starttstamp':1318262086, 'dt':61.004, 'evts':_evts(22), 'mu':24.427, 'force_new':False},
   {'run':190728, 'lb':257, 'starttstamp':1318262147, 'dt':61.003, 'evts':_evts(22), 'mu':24.385, 'force_new':False},
   {'run':190728, 'lb':258, 'starttstamp':1318262208, 'dt':61.003, 'evts':_evts(22), 'mu':24.323, 'force_new':False},
   {'run':190728, 'lb':259, 'starttstamp':1318262269, 'dt':61.040, 'evts':_evts(22), 'mu':24.281, 'force_new':False},
   {'run':190728, 'lb':260, 'starttstamp':1318262330, 'dt':60.907, 'evts':_evts(22), 'mu':24.234, 'force_new':False},
   {'run':190728, 'lb':261, 'starttstamp':1318262391, 'dt':61.011, 'evts':_evts(22), 'mu':24.238, 'force_new':False},
   {'run':190728, 'lb':262, 'starttstamp':1318262452, 'dt':61.004, 'evts':_evts(22), 'mu':24.144, 'force_new':False},
   {'run':190728, 'lb':263, 'starttstamp':1318262513, 'dt':61.004, 'evts':_evts(22), 'mu':24.155, 'force_new':False},
   {'run':190728, 'lb':264, 'starttstamp':1318262574, 'dt':61.018, 'evts':_evts(22), 'mu':24.090, 'force_new':False},
   {'run':190728, 'lb':265, 'starttstamp':1318262635, 'dt':60.932, 'evts':_evts(21), 'mu':24.002, 'force_new':False},
   {'run':190728, 'lb':266, 'starttstamp':1318262696, 'dt':61.020, 'evts':_evts(21), 'mu':24.028, 'force_new':False},
   {'run':190728, 'lb':267, 'starttstamp':1318262757, 'dt':61.019, 'evts':_evts(21), 'mu':23.977, 'force_new':False},
   {'run':190728, 'lb':268, 'starttstamp':1318262818, 'dt':61.022, 'evts':_evts(21), 'mu':24.000, 'force_new':False},
   {'run':190728, 'lb':269, 'starttstamp':1318262879, 'dt':61.042, 'evts':_evts(22), 'mu':24.041, 'force_new':False},
   {'run':190728, 'lb':270, 'starttstamp':1318262940, 'dt':60.908, 'evts':_evts(22), 'mu':24.095, 'force_new':False},
   {'run':190728, 'lb':271, 'starttstamp':1318263001, 'dt':61.016, 'evts':_evts(22), 'mu':24.052, 'force_new':False},
   {'run':190728, 'lb':272, 'starttstamp':1318263062, 'dt':61.005, 'evts':_evts(22), 'mu':24.124, 'force_new':False},
   {'run':190728, 'lb':273, 'starttstamp':1318263123, 'dt':61.019, 'evts':_evts(21), 'mu':24.035, 'force_new':False},
   {'run':190728, 'lb':274, 'starttstamp':1318263184, 'dt':61.004, 'evts':_evts(21), 'mu':24.043, 'force_new':False},
   {'run':190728, 'lb':275, 'starttstamp':1318263245, 'dt':61.003, 'evts':_evts(21), 'mu':23.966, 'force_new':False},
   {'run':190728, 'lb':276, 'starttstamp':1318263306, 'dt':61.015, 'evts':_evts(21), 'mu':23.885, 'force_new':False},
   {'run':190728, 'lb':277, 'starttstamp':1318263367, 'dt':60.919, 'evts':_evts(21), 'mu':23.910, 'force_new':False},
   {'run':190728, 'lb':278, 'starttstamp':1318263428, 'dt':61.002, 'evts':_evts(21), 'mu':23.841, 'force_new':False},
   {'run':190728, 'lb':279, 'starttstamp':1318263489, 'dt':61.008, 'evts':_evts(21), 'mu':23.742, 'force_new':False},
   {'run':190728, 'lb':280, 'starttstamp':1318263550, 'dt':61.024, 'evts':_evts(21), 'mu':23.756, 'force_new':False},
   {'run':190728, 'lb':281, 'starttstamp':1318263611, 'dt':61.017, 'evts':_evts(21), 'mu':23.802, 'force_new':False},
   {'run':190728, 'lb':282, 'starttstamp':1318263672, 'dt':61.018, 'evts':_evts(21), 'mu':23.750, 'force_new':False},
   {'run':190728, 'lb':283, 'starttstamp':1318263733, 'dt':61.033, 'evts':_evts(21), 'mu':23.681, 'force_new':False},
   {'run':190728, 'lb':284, 'starttstamp':1318263794, 'dt':60.914, 'evts':_evts(21), 'mu':23.597, 'force_new':False},
   {'run':190728, 'lb':285, 'starttstamp':1318263855, 'dt':61.002, 'evts':_evts(21), 'mu':23.654, 'force_new':False},
   {'run':190728, 'lb':286, 'starttstamp':1318263916, 'dt':61.002, 'evts':_evts(21), 'mu':23.708, 'force_new':False},
   {'run':190728, 'lb':287, 'starttstamp':1318263977, 'dt':61.002, 'evts':_evts(21), 'mu':23.718, 'force_new':False},
   {'run':190728, 'lb':288, 'starttstamp':1318264038, 'dt':61.005, 'evts':_evts(21), 'mu':23.779, 'force_new':False},
   {'run':190728, 'lb':289, 'starttstamp':1318264099, 'dt':61.019, 'evts':_evts(21), 'mu':23.811, 'force_new':False},
   {'run':190728, 'lb':290, 'starttstamp':1318264160, 'dt':61.030, 'evts':_evts(21), 'mu':23.752, 'force_new':False},
   {'run':190728, 'lb':291, 'starttstamp':1318264221, 'dt':60.959, 'evts':_evts(21), 'mu':23.645, 'force_new':False},
   {'run':190728, 'lb':292, 'starttstamp':1318264282, 'dt':61.027, 'evts':_evts(21), 'mu':23.622, 'force_new':False},
   {'run':190728, 'lb':293, 'starttstamp':1318264343, 'dt':61.017, 'evts':_evts(21), 'mu':23.618, 'force_new':False},
   {'run':190728, 'lb':294, 'starttstamp':1318264404, 'dt':61.002, 'evts':_evts(21), 'mu':23.560, 'force_new':False},
   {'run':190728, 'lb':295, 'starttstamp':1318264465, 'dt':61.003, 'evts':_evts(21), 'mu':23.514, 'force_new':False},
   {'run':190728, 'lb':296, 'starttstamp':1318264526, 'dt':60.932, 'evts':_evts(21), 'mu':23.507, 'force_new':False},
   {'run':190728, 'lb':297, 'starttstamp':1318264587, 'dt':61.019, 'evts':_evts(21), 'mu':23.272, 'force_new':False},
   {'run':190728, 'lb':298, 'starttstamp':1318264648, 'dt':61.025, 'evts':_evts(21), 'mu':23.221, 'force_new':False},
   {'run':190728, 'lb':299, 'starttstamp':1318264709, 'dt':61.016, 'evts':_evts(21), 'mu':23.113, 'force_new':False},
   {'run':190728, 'lb':300, 'starttstamp':1318264770, 'dt':61.007, 'evts':_evts(21), 'mu':23.207, 'force_new':False},
   {'run':190728, 'lb':301, 'starttstamp':1318264831, 'dt':60.927, 'evts':_evts(21), 'mu':23.195, 'force_new':False},
   {'run':190728, 'lb':302, 'starttstamp':1318264892, 'dt':61.020, 'evts':_evts(21), 'mu':23.181, 'force_new':False},
   {'run':190728, 'lb':303, 'starttstamp':1318264953, 'dt':61.021, 'evts':_evts(21), 'mu':23.099, 'force_new':False},
   {'run':190728, 'lb':304, 'starttstamp':1318265014, 'dt':61.003, 'evts':_evts(21), 'mu':23.087, 'force_new':False},
   {'run':190728, 'lb':305, 'starttstamp':1318265075, 'dt':61.001, 'evts':_evts(21), 'mu':23.004, 'force_new':False},
   {'run':190728, 'lb':306, 'starttstamp':1318265136, 'dt':61.023, 'evts':_evts(20), 'mu':22.908, 'force_new':False},
   {'run':190728, 'lb':307, 'starttstamp':1318265197, 'dt':61.005, 'evts':_evts(20), 'mu':22.763, 'force_new':False},
   {'run':190728, 'lb':308, 'starttstamp':1318265258, 'dt':61.003, 'evts':_evts(20), 'mu':22.832, 'force_new':False},
   {'run':190728, 'lb':309, 'starttstamp':1318265319, 'dt':60.925, 'evts':_evts(20), 'mu':22.784, 'force_new':False},
   {'run':190728, 'lb':310, 'starttstamp':1318265380, 'dt':61.013, 'evts':_evts(20), 'mu':22.616, 'force_new':False},
   {'run':190728, 'lb':311, 'starttstamp':1318265441, 'dt':61.072, 'evts':_evts(20), 'mu':22.578, 'force_new':False},
   {'run':190728, 'lb':312, 'starttstamp':1318265502, 'dt':60.939, 'evts':_evts(20), 'mu':22.490, 'force_new':False},
   {'run':190728, 'lb':313, 'starttstamp':1318265563, 'dt':61.022, 'evts':_evts(20), 'mu':22.616, 'force_new':False},
   {'run':190728, 'lb':314, 'starttstamp':1318265624, 'dt':61.017, 'evts':_evts(20), 'mu':22.614, 'force_new':False},
   {'run':190728, 'lb':315, 'starttstamp':1318265685, 'dt':61.016, 'evts':_evts(20), 'mu':22.693, 'force_new':False},
   {'run':190728, 'lb':316, 'starttstamp':1318265746, 'dt':61.004, 'evts':_evts(20), 'mu':22.733, 'force_new':False},
   {'run':190728, 'lb':317, 'starttstamp':1318265807, 'dt':61.001, 'evts':_evts(20), 'mu':22.651, 'force_new':False},
   {'run':190728, 'lb':318, 'starttstamp':1318265868, 'dt':60.920, 'evts':_evts(20), 'mu':22.631, 'force_new':False},
   {'run':190728, 'lb':319, 'starttstamp':1318265929, 'dt':61.033, 'evts':_evts(20), 'mu':22.509, 'force_new':False},
   {'run':190728, 'lb':320, 'starttstamp':1318265990, 'dt':61.069, 'evts':_evts(20), 'mu':22.440, 'force_new':False},
   {'run':190728, 'lb':321, 'starttstamp':1318266051, 'dt':60.908, 'evts':_evts(20), 'mu':22.749, 'force_new':False},
   {'run':190728, 'lb':322, 'starttstamp':1318266112, 'dt':61.022, 'evts':_evts(22), 'mu':25.162, 'force_new':False},
   {'run':190728, 'lb':323, 'starttstamp':1318266173, 'dt':61.024, 'evts':_evts(22), 'mu':25.054, 'force_new':False},
   {'run':190728, 'lb':324, 'starttstamp':1318266234, 'dt':61.013, 'evts':_evts(22), 'mu':24.909, 'force_new':False},
   {'run':190728, 'lb':325, 'starttstamp':1318266295, 'dt':61.012, 'evts':_evts(22), 'mu':24.864, 'force_new':False},
   {'run':190728, 'lb':326, 'starttstamp':1318266356, 'dt':61.003, 'evts':_evts(22), 'mu':24.761, 'force_new':False},
   {'run':190728, 'lb':327, 'starttstamp':1318266417, 'dt':61.002, 'evts':_evts(22), 'mu':24.711, 'force_new':False},
   {'run':190728, 'lb':328, 'starttstamp':1318266478, 'dt':60.903, 'evts':_evts(22), 'mu':24.759, 'force_new':False},
   {'run':190728, 'lb':329, 'starttstamp':1318266539, 'dt':61.001, 'evts':_evts(22), 'mu':24.764, 'force_new':False},
   {'run':190728, 'lb':330, 'starttstamp':1318266600, 'dt':61.003, 'evts':_evts(22), 'mu':24.593, 'force_new':False},
   {'run':190728, 'lb':331, 'starttstamp':1318266661, 'dt':61.153, 'evts':_evts(22), 'mu':24.674, 'force_new':False},
   {'run':190728, 'lb':332, 'starttstamp':1318266722, 'dt':60.901, 'evts':_evts(22), 'mu':24.600, 'force_new':False},
   {'run':190728, 'lb':333, 'starttstamp':1318266783, 'dt':54.124, 'evts':_evts(20), 'mu':24.621, 'force_new':False},
#--> end hiding
]

include('RunDependentSimData/configCommon.py')

#cleanup python memory
if not "RunDMC_testing_configuration" in dir():
    del JobMaker
