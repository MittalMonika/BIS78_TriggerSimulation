# dictionary defining a watch list for MR notifications
# keys   ... must be valid regular expression
# values ... must be sets containing Gitlab usernames as strings
WATCH_LIST = {}

WATCH_LIST['^CI$']       = set(['cgumpert'])
WATCH_LIST['Simulation'] = set(['ritsch'])
WATCH_LIST['TrkiPatFitter'] = set(['pop'])
WATCH_LIST['MooPerformance'] = set(['pop'])
WATCH_LIST['JetCalibTools'] = set(['jbossios'])
WATCH_LIST['AFP'] = set(['ggach'])

