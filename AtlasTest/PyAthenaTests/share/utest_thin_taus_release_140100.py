############################
if not 'INPUT' in dir():
    INPUT=[
        'root://eosatlas//eos/atlas/user/b/binet/reffiles/14.1.0.x'\
        '/AllBasicSamples.AOD.pool.root'
        ]
    pass
if not 'OUTPUT' in dir():
    OUTPUT = 'thinned-taus.pool'
    pass
############################

PoolAODInput = INPUT
PoolAODOutput= 'dummy.aod.pool'

# AOD reprocessing
include ('RecExCommon/aodtoaod.py')
svcMgr.GeoModelSvc.AtlasVersion = 'ATLAS-CSC-05-01-00'

include('PyAthenaTests/pythin_jobOptions.py')
del job.StreamAOD
del job.StreamAOD_FH
del job.Thinner

job.insert(-1, Thinner('thin_taus',
                       coll_type='Analysis::TauJetContainer',
                       coll_name='TauRecContainer'))
job.OutStream.TakeItemsFromInput = False
job.OutStream.ForceRead = False
job.OutStream.ItemList = [ 'EventInfo#*',
                           'Analysis::TauJetContainer#*',
                           "TrackCollection#*",]
