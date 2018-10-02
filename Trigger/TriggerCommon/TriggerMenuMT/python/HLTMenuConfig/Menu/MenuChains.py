## chain name, HypoTool confname (for combined chains there is more than one)

ChainDictionary ={  'HLT_e8'     : ['HLT_e8'],
                    'HLT_e20'    : ['HLT_e20'],
                    'HLT_mu6'    : ['HLT_mu6'],
                    'HLT_mu6Comb' : ['HLT_mu6'],
                    'HLT_mu6fast' : ['HLT_mu6'],
                    'HLT_mu8'    : ['HLT_mu8'],
                    'HLT_mu81step': ['HLT_mu8'],
                    'HLT_mu20'   : ['HLT_mu20'],
                    'HLT_2mu6'   : ['HLT_2mu6'], # review double object
                    'HLT_2mu6Comb'   : ['HLT_2mu6'], # review double object
                    'HLT_mu8_e8' : ['HLT_mu8','HLT_e8'],                    
                    'HLT_e3_etcut': ['HLT_e3_etcut'],
                    'HLT_e3_etcut1step': ['HLT_e3_etcut'],
                    'HLT_e5_etcut': ['HLT_e5_etcut'],
                    'HLT_e7_etcut': ['HLT_e7_etcut'],
                    'HLT_mu6_e3_etcut': ['HLT_mu6', 'HLT_e3_etcut'],
                    'HLT_e3_etcut_mu6': ['HLT_e3_etcut', 'HLT_mu6'],
                    'HLT_g5_etcut' : ['HLT_g5_etcut'],
                    'HLT_j85'      : ['HLT_j85']
                 }



def getConfFromChainName(name):  
    if name in ChainDictionary:
        print "MenuChains.getConfFromChainName: Called chain "+name+" and hypoTool conf "+ str(ChainDictionary[name])
        return ChainDictionary[name]
    log.error("MenuChains.getConfFromChainName: Wrong chain name given: found %s",name)
    sys.exit("ERROR, in chain configuration") 


    
