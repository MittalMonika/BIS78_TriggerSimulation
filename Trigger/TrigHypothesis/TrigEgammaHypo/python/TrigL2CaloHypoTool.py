# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
import re
__pattern = "e(?P<threshold>\d+)"
__cpattern = re.compile( __pattern )

from AthenaCommon.SystemOfUnits import GeV

from TrigEgammaHypo.TrigEgammaHypoConf import TrigL2CaloHypoTool
def TrigL2CaloHypoToolFromName( name ):
    """ decode the name ( chain ) and figure out the threshold and selection from it """

    assert name.startswith( "HLT_e" ), "Not a single electron chain"+name+" can't handle it yet"
    bname = name.split('_')
    m = __cpattern.match( bname[1] )
    assert m, "The chain name part 1 "+name+" does not match pattern "+__pattern

    # print m.groups()
    threshold = int( m.group( "threshold" ) )
    sel = bname[2]
    
 #   print "configuring threshold " , threshold , " slection " , sel

    from TrigL2CaloHypoCutDefs import L2CaloCutMaps    
    possibleSel = L2CaloCutMaps( threshold ).MapsHADETthr.keys()
    

    tool = TrigL2CaloHypoTool( name ) 
    tool.AcceptAll = False
    from TriggerJobOpts.TriggerFlags import TriggerFlags
#    print "monitoring", TriggerFlags.enableMonitoring()


    if 'Validation' in TriggerFlags.enableMonitoring() or 'Online' in  TriggerFlags.enableMonitoring():
        from AthenaMonitoring.AthenaMonitoringConf import GenericMonitoringTool
        from AthenaMonitoring.DefineHistogram import defineHistogram
        monTool = GenericMonitoringTool("MonTool"+name)
        monTool.Histograms = [ defineHistogram('dEta', type='TH1F', title="L2Calo Hypo #Delta#eta_{L2 L1}; #Delta#eta_{L2 L1}", xbins=80, xmin=-0.01, xmax=0.01),
                               defineHistogram('dPhi', type='TH1F', title="L2Calo Hypo #Delta#phi_{L2 L1}; #Delta#phi_{L2 L1}", xbins=80, xmin=-0.01, xmax=0.01),
                               defineHistogram('Et_em', type='TH1F', title="L2Calo Hypo cluster E_{T}^{EM};E_{T}^{EM} [MeV]", xbins=50, xmin=-2000, xmax=100000),
                               defineHistogram('Eta', type='TH1F', title="L2Calo Hypo entries per Eta;Eta", xbins=100, xmin=-2.5, xmax=2.5),
                               defineHistogram('Phi', type='TH1F', title="L2Calo Hypo entries per Phi;Phi", xbins=128, xmin=-3.2, xmax=3.2) ]

        cuts=['Input','has one TrigEMCluster', '#Delta #eta L2-L1', '#Delta #phi L2-L1','eta','rCore',
              'eRatio','E_{T}^{EM}', 'E_{T}^{Had}','f_{1}','Weta2','Wstot','F3']

        labelsDescription = ''
        for c in cuts:
            labelsDescription +=  c+':'
            
        monTool.Histograms += [ defineHistogram('CutCounter', type='TH1I', title="L2Calo Hypo Passed Cuts;Cut",
                                             xbins=13, xmin=-1.5, xmax=12.5,  opt="kCumulative", labels=labelsDescription) ]

        if 'Validation' in TriggerFlags.enableMonitoring():
            monTool.Histograms += [ defineHistogram('Et_had', type='TH1F', title="L2Calo Hypo E_{T}^{had} in first layer;E_{T}^{had} [MeV]", xbins=50, xmin=-2000, xmax=100000),
                                    defineHistogram('Rcore', type='TH1F', title="L2Calo Hypo R_{core};E^{3x3}/E^{3x7} in sampling 2", xbins=48, xmin=-0.1, xmax=1.1),
                                    defineHistogram('Eratio', type='TH1F', title="L2Calo Hypo E_{ratio};E^{max1}-E^{max2}/E^{max1}+E^{max2} in sampling 1 (excl.crack)", xbins=64, xmin=-0.1, xmax=1.5),
                                    defineHistogram('EtaBin', type='TH1I', title="L2Calo Hypo entries per Eta bin;Eta bin no.", xbins=11, xmin=-0.5, xmax=10.5),
                                    defineHistogram('F1', type='TH1F', title="L2Calo Hypo f_{1};f_{1}", xbins=34, xmin=-0.5, xmax=1.2),                                    
                                    defineHistogram('Weta2', type='TH1F', title="L2Calo Hypo Weta2; E Width in sampling 2", xbins=96, xmin=-0.1, xmax=0.61),     
                                    defineHistogram('Wstot', type='TH1F', title="L2Calo Hypo Wstot; E Width in sampling 1", xbins=48, xmin=-0.1, xmax=11.),
                                    defineHistogram('F3', type='TH1F', title="L2Calo Hypo F3; E3/(E0+E1+E2+E3)", xbins=96, xmin=-0.1, xmax=1.1) ]        
            

        tool.MonTool = monTool
        tool.MonTool.HistogramsGroupName = 'L2CaloHypo/'+tool.name()


        

    tool.EtaBins        = [0.0, 0.6, 0.8, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47]
    def same( val ):
        return [val]*( len( tool.EtaBins ) - 1 )

    tool.ETthr          = same( float(threshold) )
    tool.dETACLUSTERthr = 0.1
    tool.dPHICLUSTERthr = 0.1
    tool.F1thr          = same( 0.005 )
    tool.ET2thr         = same( 90.0*GeV )
    tool.HADET2thr      = same( 999.0 )
    tool.HADETthr       = same( 0.058 ) 
    tool.WETA2thr       = same( 99999. ) 
    tool.WSTOTthr       = same( 99999. )
    tool.F3thr          = same( 99999. )
    tool.CARCOREthr     = same( -9999. ) 
    tool.CAERATIOthr    = same( -9999. )

    if sel == 'nocut':
        tool.AcceptAll = True
        tool.ETthr          = same( float( threshold )*GeV ) 
        tool.dETACLUSTERthr = 9999.
        tool.dPHICLUSTERthr = 9999.
        tool.F1thr          = same( 0.0 )
        tool.HADETthr       = same( 9999. )
        tool.CARCOREthr     = same( -9999. ) 
        tool.CAERATIOthr    = same( -9999. )

    elif sel == "etcut":
        tool.ETthr          = same( ( float( threshold ) -  3 )*GeV ) 
        # No other cuts applied
        tool.dETACLUSTERthr = 9999.
        tool.dPHICLUSTERthr = 9999.
        tool.F1thr          = same( 0.0 )
        tool.HADETthr       = same( 9999. )
        tool.CARCOREthr     = same( -9999. )
        tool.CAERATIOthr    = same( -9999. )

    elif sel in possibleSel: # real selection
        tool.ETthr       = same( ( float( threshold ) - 3 )*GeV )
        tool.HADETthr    = L2CaloCutMaps( threshold ).MapsHADETthr[sel]
        tool.CARCOREthr  = L2CaloCutMaps( threshold ).MapsCARCOREthr[sel]
        tool.CAERATIOthr = L2CaloCutMaps( threshold ).MapsCAERATIOthr[sel]

    
    etaBinsLen = len( tool.EtaBins ) - 1
    for prop in "ETthr HADETthr CARCOREthr CARCOREthr F1thr F3thr WSTOTthr WETA2thr HADETthr HADETthr ET2thr".split():
        propLen = len( getattr( tool, prop ) ) 
        assert propLen == etaBinsLen , "In " + name + " " + prop + " has length " + str( propLen ) + " which is different from EtaBins which has length " + str( etaBinsLen )
        

            
            #    assert  __l( EtaBins, tool.ETthr, tool.HADETthr, tool.CARCOREthr, tool.CARCOREthr ) , "All list properties should have equal length ( as EtaBins )"
    #print tool
    return tool

if __name__ == "__main__":    
    from TriggerJobOpts.TriggerFlags import TriggerFlags
    TriggerFlags.enableMonitoring=['Validation']
    t = TrigL2CaloHypoToolFromName( "HLT_e10_nocut" )
    assert t, "cant configure NoCut"    
    print t
    t = TrigL2CaloHypoToolFromName( "HLT_e10_etcut" )
    assert t, "cant configure EtCut"
    print t
    t  = TrigL2CaloHypoToolFromName( "HLT_e10_tight" )
    assert t, "cant configure rel selection - tight"
    print t    

    t  = TrigL2CaloHypoToolFromName( "HLT_e0_perf" )
    assert t, "cant configure rel selection - perf"
    print t    

    print ( "\n\nALL OK\n\n" )
