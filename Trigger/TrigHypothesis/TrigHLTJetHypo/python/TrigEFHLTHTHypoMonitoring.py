# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from TrigMonitorBase.TrigGenericMonitoringToolConfig import defineHistogram, TrigGenericMonitoringToolConfig 
        

class TrigEFHLTHTHypoOnlineMonitoring(TrigGenericMonitoringToolConfig):
    def __init__ (self, name="TrigEFHLTHypoOnlineMonitoring"):
        """ defines histograms for online """
        super(TrigEFHLTHTHypoOnlineMonitoring, self).__init__(name)

        self.defineTarget("Online")


        self.Histograms += [ defineHistogram('HT',type='TH1F', title="EFHLT HT before cuts", xbins=200, xmin=0, xmax=1000) ]

        self.Histograms += [ defineHistogram('passedJetCuts_et',type='TH1F', title="EFHLT all jets Et after jet cuts", xbins=198, xmin=10, xmax=1000) ]
        self.Histograms += [ defineHistogram('passedJetCuts_eta',type='TH1F', title="EFHLT all jets eta after jet cuts", xbins=80, xmin=-4, xmax=4) ]
        self.Histograms += [ defineHistogram('passedJetCuts_phi',type='TH1F', title="EFHLT all jets phi after jet cuts", xbins=64, xmin=-3.2, xmax=3.2) ]

        self.Histograms += [ defineHistogram('HT_passedHT',type='TH1F', title="EFHLT HT passed HT cut", xbins=200, xmin=0, xmax=1000) ]

        self.Histograms += [ defineHistogram('nJets',type='TH1F', title="EFHLT no.of jets considered", xbins=50, xmin=0, xmax=50) ]
        self.Histograms += [ defineHistogram('nJets_passedCuts',type='TH1F', title="EFHLT no.of jets passed basic cuts", xbins=50, xmin=0, xmax=50) ]
        self.Histograms += [ defineHistogram('nJets_passedHT',type='TH1F', title="EFHLT no.of jets passed HT cuts", xbins=50, xmin=0, xmax=50) ]
        self.Histograms += [ defineHistogram('nJets_failedHT',type='TH1F', title="EFHLT no.of jets failed HT cuts", xbins=50, xmin=0, xmax=50) ]

        self.Histograms += [ defineHistogram('cutCounter',type='TH1F', title="EFHLT Cut Counter", xbins=10, xmin=0, xmax=10) ]
        

class TrigEFHLTHTHypoValidationMonitoring(TrigGenericMonitoringToolConfig):
    def __init__ (self, name="TrigEFHLTHypoValidationMonitoring"):
        """ defines histograms for validation """
        super(TrigEFHLTHTHypoValidationMonitoring, self).__init__(name)

        self.defineTarget("Validation")

        self.Histograms += [ defineHistogram('HT',type='TH1F', title="EFHLT HT before cuts", xbins=200, xmin=0, xmax=1000) ]

        self.Histograms += [ defineHistogram('passedJetCuts_et',type='TH1F', title="EFHLT all jets Et after jet cuts", xbins=198, xmin=10, xmax=1000) ]
        self.Histograms += [ defineHistogram('passedJetCuts_eta',type='TH1F', title="EFHLT all jets eta after jet cuts", xbins=80, xmin=-4, xmax=4) ]
        self.Histograms += [ defineHistogram('passedJetCuts_phi',type='TH1F', title="EFHLT all jets phi after jet cuts", xbins=64, xmin=-3.2, xmax=3.2) ]

        self.Histograms += [ defineHistogram('HT_passedHT',type='TH1F', title="EFHLT HT passed HT cut", xbins=200, xmin=0, xmax=1000) ]

        self.Histograms += [ defineHistogram('nJets',type='TH1F', title="EFHLT no.of jets considered", xbins=50, xmin=0, xmax=50) ]
        self.Histograms += [ defineHistogram('nJets_passedCuts',type='TH1F', title="EFHLT no.of jets passed basic cuts", xbins=50, xmin=0, xmax=50) ]
        self.Histograms += [ defineHistogram('nJets_passedHT',type='TH1F', title="EFHLT no.of jets passed HT cuts", xbins=50, xmin=0, xmax=50) ]
        self.Histograms += [ defineHistogram('nJets_failedHT',type='TH1F', title="EFHLT no.of jets failed HT cuts", xbins=50, xmin=0, xmax=50) ]

        self.Histograms += [ defineHistogram('cutCounter',type='TH1F', title="EFHLT Cut Counter", xbins=10, xmin=0, xmax=10) ]
        
