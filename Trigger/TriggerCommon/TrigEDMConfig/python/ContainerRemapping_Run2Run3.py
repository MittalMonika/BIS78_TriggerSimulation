#==============================================================
# ContainerRemapping_Run2Run3.py
# A jobOptions fragment for aliasing Run 3 HLT container names
# to the Run 2 predecessors, e.g. for validation
#

# Mapping of Run 3 name to Run 2 name
# Should suffice to just do the interface container
HLT_Name_Changes = {
    "xAOD::JetContainer":
        [
        # Small-R
        ('HLT_AntiKt4EMTopoJets_subjesIS',                   'HLT_xAOD__JetContainer_a4tcemsubjesISFS'),
        ('HLT_AntiKt4EMTopoJets_subjes',                     'HLT_xAOD__JetContainer_a4tcemsubjesFS'),
        ('HLT_AntiKt4EMTopoJets_nojcalib',                   'HLT_xAOD__JetContainer_a4tcemnojcalibISFS'),
        # Large-R reclustered
        ('HLT_AntiKt10JetRCJets_subjesIS',                   'HLT_xAOD__JetContainer_a10r_tcemsubjesISFS'),
        # Large-R ungroomed
        ('HLT_AntiKt10LCTopoJets_subjes',                    'HLT_xAOD__JetContainer_a10tclcwsubjesFS'),
        ('HLT_AntiKt10LCTopoJets_nojcalib',                  'HLT_xAOD__JetContainer_a10tclcwnojcalibFS'),
        # Large-R groomed
        ('HLT_AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets_jes', 'HLT_xAOD__JetContainer_a10ttclcwjesFS'),
        ],
    
    'xAOD::CaloClusterContainer':
        [('HLT_TopoCaloClustersFS', 'HLT_TrigCaloClusterMaker_topo')],

}

from SGComps import AddressRemappingSvc

def remapHLTContainerNames():
    for containertype, renamelist in HLT_Name_Changes.iteritems():
        for run3name, run2name in renamelist:
            print "Remapping {} object SG Keys: {} --> {}".format(containertype,run3name,run2name)

            auxcontainertype = containertype.replace("Container","AuxContainer")
            if containertype=="xAOD::CaloClusterContainer":
                auxcontainertype = "xAOD::CaloClusterTrigAuxContainer"

            #AddressRemappingSvc.addInputRename(containertype,run3name,run2name)
            ars = AddressRemappingSvc.getAddressRemappingSvc()
            ars.TypeKeyOverwriteMaps += [ '%s#%s->%s#%s' % (containertype, run3name,
                                                            containertype, run2name) ]

            #AddressRemappingSvc.addInputRename(auxcontainertype,run3name+"Aux.",run2name+"Aux.")
            ars.TypeKeyOverwriteMaps += [ '%s#%s->%s#%s' % (auxcontainertype, run3name+"Aux.",
                                                            auxcontainertype, run2name+"Aux.") ]
            
