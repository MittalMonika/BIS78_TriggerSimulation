# dictionary defining a watch list for MR notifications
# keys   ... must be valid regular expression
# values ... must be sets containing Gitlab usernames as strings
WATCH_LIST = {}

WATCH_LIST['^CI$']       = set(['cgumpert'])
WATCH_LIST['Simulation'] = set(['ritsch','jchapman','vpascuzz'])
WATCH_LIST['Digitization'] = set(['jchapman'])
WATCH_LIST['Overlay'] = set(['jchapman','ahaas','tkharlam'])
WATCH_LIST['TrkiPatFitter'] = set(['pop'])
WATCH_LIST['MooPerformance'] = set(['pop'])
WATCH_LIST['JetCalibTools'] = set(['jbossios'])
WATCH_LIST['AFP'] = set(['ggach'])
WATCH_LIST['MuonEfficiencyCorrections'] = set(['nkoehler','jojungge'])
WATCH_LIST['MuonTPTools'] = set(['nkoehler','jojungge'])
WATCH_LIST['MuonPerformanceAlgs'] = set(['nkoehler','jojungge'])
WATCH_LIST['MuonPerformanceHistUtils'] = set(['nkoehler','jojungge'])
WATCH_LIST['^PhysicsAnalysis/D3PDTools'] = set(['krumnack'])
WATCH_LIST['QuickAna'] = set(['krumnack'])
WATCH_LIST['AsgTools'] = set(['krumnack'])
WATCH_LIST['PATInterfaces'] = set(['krumnack'])
WATCH_LIST['(PixelMonitoring)|(PixelConditionsServices)|(PixelRawData)'] = set(['kzoch','ibragimo'])
WATCH_LIST['IsolationSelection'] = set(['maklein','jojungge','jpoveda','dzhang'])
WATCH_LIST['PhysicsAnalysis/SUSYPhys'] = set(['zmarshal','szambito'])
WATCH_LIST['DerivationFrameworkDataPrep'] = set(['zmarshal','cohm'])
WATCH_LIST['DerivationFrameworkSUSY'] = set(['zmarshal','martindl','whopkins'])
