# dictionary for mapping package names to software domains
# keys   ... must be strings for the name of the software domain
# values ... must be sets containing valid regular expression strings
DOMAIN_MAP = {}

DOMAIN_MAP['Analysis']         = set(['^PhysicsAnalysis/'])
DOMAIN_MAP['Build']            = set(['^Build$','^Projects/'])
DOMAIN_MAP['Calorimeter']      = set(['^Calorimeter/'])
DOMAIN_MAP['CI']               = set(['^CI'])
DOMAIN_MAP['Core']             = set(['^Control/'])
DOMAIN_MAP['Database']         = set(['^Database/'])
DOMAIN_MAP['DQ']               = set(['^DataQuality/','^LumiBlock/'])
DOMAIN_MAP['EDM']              = set(['^Event/'])
DOMAIN_MAP['Egamma']           = set(['^egamma'])
DOMAIN_MAP['EventDisplay']     = set(['^graphics/'])
DOMAIN_MAP['Externals']        = set(['^External/'])
DOMAIN_MAP['ForwardDetectors'] = set(['^ForwardDetectors/'])
DOMAIN_MAP['Generators']       = set(['^Generators/'])
DOMAIN_MAP['Geometry']         = set(['^AtlasGeometryCommon/','^DetectorDescription/'])
DOMAIN_MAP['InnerDetector']    = set(['^InnerDetector/'])
DOMAIN_MAP['JetEtmiss']        = set(['Jet','(?<!spectro)(?<!geo)MET','MissingEt','EventShape'])
DOMAIN_MAP['LAr']              = set(['^LArCalorimeter/'])
DOMAIN_MAP['Magnets']          = set(['^MagneticField/'])
DOMAIN_MAP['MuonSpectrometer'] = set(['^MuonSpectrometer/'])
DOMAIN_MAP['Other']            = set(['^Commission/','^LCG_Interfaces/','^PackDist/','^ReleaseTests/'])
DOMAIN_MAP['Reconstruction']   = set(['^Reconstruction/'])
DOMAIN_MAP['Simulation']       = set(['^Simulation/'])
DOMAIN_MAP['Tau']              = set(['Tau'])
DOMAIN_MAP['Test']             = set(['^AtlasTest/','^TestPolicy/'])
DOMAIN_MAP['TestBeam']         = set(['^TestBeam/'])
DOMAIN_MAP['Tile']             = set(['^TileCalorimeter/'])
DOMAIN_MAP['Tools']            = set(['^Tools/'])
DOMAIN_MAP['Tracking']         = set(['^Tracking/'])
DOMAIN_MAP['Trigger']          = set(['^Trigger/','^HLT/','Trig','^DetectorDescription/.?Reg','^DetectorDescription/RoiDescriptor','RegionSelector'])

