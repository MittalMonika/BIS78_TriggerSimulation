# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from TrigMonitorBase.TrigGenericMonitoringToolConfig import defineHistogram, TrigGenericMonitoringToolConfig 

ERROR_labels = ' No_EventInfo : No_RoI : No_TrackColl : No_muon : Empty_TrackColl : More_TrackColl : AddTrack_Fails : Unique_AddTrack_Fails : CalcInvMass_Fails : CalcMother_Fails : CalcMassPull_Fails : BphysColl_Fails '
ERROR_number = 11

ACCEPTANCE_labels = ' Input : AcceptAll : Got_RoI : Got_TrackColl : Full_TrackColl : NotUsed1 : Full_IDTracks : Mu1_Chi2_Cut : Mu1_pT_Cut : Mu1_Chi2Pt_Cut : Mu1_dEta_Cut : Mu1_dPhi_Cut : Mu1_dR_Cut : Mu2_Chi2_Cut : Mu2_pT_Cut : Mu2_Chi2Pt_Cut : Opp_Charge : InvMass_Cut : NotUsed2 : Vertexing : CalcInvMass : NotUsed3 : NotUsed4 : NotUsed5 : BphysColl_not_Empty : NotUsed6 : Each_Mu1_Chi2_Cut : Each_Mu1_pT_Cut : Each_Mu1_Chi2Pt_Cuts : Each_Mu1_dEta_Cut : Each_Mu1_dPhi_Cut : Each_Mu1_dR_Cut : NotUsed7 : Each_Mu2_Chi2_Cut : Each_Mu2_pT_Cut : Each_Mu2_Chi2Pt_Cuts : Each_Opp_Charge : Each_InvMass_Cut : NotUsed8 : NotUsed9 : NotUsed10 : NotUsed11 : NotUsed12  '
ACCEPTANCE_labels = ACCEPTANCE_labels.replace(" ","") # remove spaces which confuse histogram labelling
ACCEPTANCE_number = 43
 
class TrigEFTrkMassFexValidationMonitoring_RoI(TrigGenericMonitoringToolConfig):
    def __init__ (self, name="EFTrkMassFexValidation_RoI"):
        super(TrigEFTrkMassFexValidationMonitoring_RoI, self).__init__(name)
        self.defineTarget("Validation")
        self.Histograms  = [ defineHistogram ( 'Errors', type = 'TH1F',
                                               title = 'EFTrkMassFex - Algorithm errors ; ; # Events / Candidates',
                                               xbins = ERROR_number , xmin = -0.5, xmax = ERROR_number - 0.5,
                                               labels = ERROR_labels ) ]
        self.Histograms += [ defineHistogram ( 'Acceptance', type = 'TH1F',
                                               title = 'EFTrkMassFex - Reached steps - acceptance ; ; # Events / Candidates',
                                               xbins = ACCEPTANCE_number , xmin = -0.5, xmax = ACCEPTANCE_number - 0.5,
                                               labels = ACCEPTANCE_labels ) ]
        self.Histograms += [ defineHistogram ( 'ROIEta, ROIPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - ROI direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'nTracks', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of ID tracks ; # Tracks ; # Events',
                                               xbins = 101, xmin = -1.5, xmax = 99.5 ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} - low range ; p_{T} [GeV] ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} ; p_{T} [GeV] ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 200. ) ]
        self.Histograms += [ defineHistogram ( 'TrkEta, TrkPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta, TrkROIdPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 60, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 64, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Pt, Trk2Pt', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks p_{T} correlation ; p_{T#mu1} [GeV] ; p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 50, xmin = 0., xmax = 50.,
                                               ybins = 50, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Eta, Trk2Eta', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #eta correlation ; #eta_{#mu1} ; #eta_{#mu2} ; # Candidates',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 30, ymin = -3.0, ymax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Phi, Trk2Phi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #phi correlation ; #phi_{#mu1} ; #phi_{#mu2} ; # Candidates',
                                               xbins = 32, xmin = -3.2, xmax = 3.2,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta, Trk1Trk2dPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Candidates',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 32, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'SumPtTrk12', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1+Track2 p_{T} ; p_{T#mu1}+p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass, InvMass_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Before vs. after vertex fit invariant mass ; Fitted mass [GeV] ; Invariant mass [GeV] ; # Candidates',
                                               xbins = 25, xmin = 1., xmax = 6.,
                                               ybins = 25, ymin = 1., ymax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoF', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF} ; #chi^{2}/_{NDoF} ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoFProb', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF}-probability ; prob(#chi^{2}/_{NDoF}) ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 1. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} ; p_{T} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt, SumPtTrk12_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} vs. track1+track2 p_{T} ; p_{T} [GeV] ; (p_{T#mu1}+p_{T#mu2}) [GeV] ; # Candidates',
                                               xbins = 25, xmin = 0., xmax = 50.,
                                               ybins = 25, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex R-position ; R [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 10. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxZ', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex Z-position ; Z [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 500. ) ]
        self.Histograms += [ defineHistogram ( 'nBphys', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of di-muon candidates ; # di-muons ; # Events',
                                               xbins = 50, xmin = -0.5, xmax = 49.5 ) ]
        self.Histograms += [ defineHistogram ( 'TotalTime', type = 'TH1F',
                                               title = 'EFTrkMassFex - Total processing time ; t_{tot} [ms] ; # Events',
                                               xbins = 100, xmin = 0., xmax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'VertexingTime', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit processing time ; t_{vtx} [ms] ; # Events',
                                               xbins = 100, xmin = 0., xmax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'TotalTime, VertexingTime', type = 'TH2F',
                                               title = 'EFTrkMassFex - Total vs. vertex fit processing time ; t_{tot} [ms] ; t_{vtx} [ms] ; # Events',
                                               xbins = 100, xmin = 0., xmax = 50.,
                                               ybins = 100, ymin = 0., ymax = 50. ) ]

class TrigEFTrkMassFexOnlineMonitoring_RoI(TrigGenericMonitoringToolConfig):
    def __init__ (self, name="EFTrkMassFexOnline_RoI"):
        super(TrigEFTrkMassFexOnlineMonitoring_RoI, self).__init__(name)
        self.defineTarget("Online")
        self.Histograms  = [ defineHistogram ( 'Errors', type = 'TH1F',
                                               title = 'EFTrkMassFex - Algorithm errors ; ; # Events / Candidates',
                                               xbins = ERROR_number , xmin = -0.5, xmax = ERROR_number - 0.5,
                                               labels = ERROR_labels ) ]
        self.Histograms += [ defineHistogram ( 'Acceptance', type = 'TH1F',
                                               title = 'EFTrkMassFex - Reached steps - acceptance ; ; # Events / Candidates',
                                               xbins = ACCEPTANCE_number , xmin = -0.5, xmax = ACCEPTANCE_number - 0.5,
                                               labels = ACCEPTANCE_labels ) ]
        self.Histograms += [ defineHistogram ( 'ROIEta, ROIPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - ROI direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'nTracks', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of ID tracks ; # Tracks ; # Events',
                                               xbins = 101, xmin = -1.5, xmax = 99.5 ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} - low range ; p_{T} [GeV] ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} ; p_{T} [GeV] ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 200. ) ]
        self.Histograms += [ defineHistogram ( 'TrkEta, TrkPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta, TrkROIdPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 60, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 64, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Pt, Trk2Pt', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks p_{T} correlation ; p_{T#mu1} [GeV] ; p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 50, xmin = 0., xmax = 50.,
                                               ybins = 50, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Eta, Trk2Eta', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #eta correlation ; #eta_{#mu1} ; #eta_{#mu2} ; # Candidates',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 30, ymin = -3.0, ymax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Phi, Trk2Phi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #phi correlation ; #phi_{#mu1} ; #phi_{#mu2} ; # Candidates',
                                               xbins = 32, xmin = -3.2, xmax = 3.2,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta, Trk1Trk2dPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Candidates',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 32, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'SumPtTrk12', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1+Track2 p_{T} ; p_{T#mu1}+p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass, InvMass_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Before vs. after vertex fit invariant mass ; Fitted mass [GeV] ; Invariant mass [GeV] ; # Candidates',
                                               xbins = 25, xmin = 1., xmax = 6.,
                                               ybins = 25, ymin = 1., ymax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoF', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF} ; #chi^{2}/_{NDoF} ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoFProb', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF}-probability ; prob(#chi^{2}/_{NDoF}) ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 1. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} ; p_{T} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt, SumPtTrk12_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} vs. track1+track2 p_{T} ; p_{T} [GeV] ; (p_{T#mu1}+p_{T#mu2}) [GeV] ; # Candidates',
                                               xbins = 25, xmin = 0., xmax = 50.,
                                               ybins = 25, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex R-position ; R [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 10. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxZ', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex Z-position ; Z [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 500. ) ]
        self.Histograms += [ defineHistogram ( 'nBphys', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of di-muon candidates ; # di-muons ; # Events',
                                               xbins = 50, xmin = -0.5, xmax = 49.5 ) ]
#        self.Histograms += [ defineHistogram ( 'TotalTime', type = 'TH1F',
#                                               title = 'EFTrkMassFex - Total processing time ; t_{tot} [ms] ; # Events',
#                                               xbins = 100, xmin = 0., xmax = 50. ) ]
#        self.Histograms += [ defineHistogram ( 'VertexingTime', type = 'TH1F',
#                                               title = 'EFTrkMassFex - Vertex fit processing time ; t_{vtx} [ms] ; # Events',
#                                               xbins = 100, xmin = 0., xmax = 50. ) ]
#        self.Histograms += [ defineHistogram ( 'TotalTime, VertexingTime', type = 'TH2F',
#                                               title = 'EFTrkMassFex - Total vs. vertex fit processing time ; t_{tot} [ms] ; t_{vtx} [ms] ; # Events',
#                                               xbins = 100, xmin = 0., xmax = 50.,
#                                               ybins = 100, ymin = 0., ymax = 50. ) ]

class TrigEFTrkMassFexValidationMonitoring_FS(TrigGenericMonitoringToolConfig):
    def __init__ (self, name="EFTrkMassFexValidation_FS"):
        super(TrigEFTrkMassFexValidationMonitoring_FS, self).__init__(name)
        self.defineTarget("Validation")
        self.Histograms  = [ defineHistogram ( 'Errors', type = 'TH1F',
                                               title = 'EFTrkMassFex - Algorithm errors ; ; # Events / Candidates',
                                               xbins = ERROR_number , xmin = -0.5, xmax = ERROR_number - 0.5,
                                               labels = ERROR_labels ) ]
        self.Histograms += [ defineHistogram ( 'Acceptance', type = 'TH1F',
                                               title = 'EFTrkMassFex - Reached steps - acceptance ; ; # Events / Candidates',
                                               xbins = ACCEPTANCE_number , xmin = -0.5, xmax = ACCEPTANCE_number - 0.5,
                                               labels = ACCEPTANCE_labels ) ]
        self.Histograms += [ defineHistogram ( 'ROIEta, ROIPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - ROI direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'nTracks', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of ID tracks ; # Tracks ; # Events',
                                               xbins = 101, xmin = -1.5, xmax = 99.5 ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} - low range ; p_{T} [GeV] ; # Tracks',
                                               xbins = 100, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} ; p_{T} [GeV] ; # Tracks',
                                               xbins = 100, xmin = 0., xmax = 200. ) ]
        self.Histograms += [ defineHistogram ( 'TrkEta, TrkPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta, TrkROIdPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 32, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Pt, Trk2Pt', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks p_{T} correlation ; p_{T#mu1} [GeV] ; p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 25, xmin = 0., xmax = 50.,
                                               ybins = 25, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Eta, Trk2Eta', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #eta correlation ; #eta_{#mu1} ; #eta_{#mu2} ; # Candidates',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 30, ymin = -3.0, ymax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Phi, Trk2Phi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #phi correlation ; #phi_{#mu1} ; #phi_{#mu2} ; # Candidates',
                                               xbins = 32, xmin = -3.2, xmax = 3.2,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta, Trk1Trk2dPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Candidates',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 32, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'SumPtTrk12', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1+Track2 p_{T} ; p_{T#mu1}+p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass, InvMass_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Before vs. after vertex fit invariant mass ; Fitted mass [GeV] ; Invariant mass [GeV] ; # Candidates',
                                               xbins = 25, xmin = 1., xmax = 6.,
                                               ybins = 25, ymin = 1., ymax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoF', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF} ; #chi^{2}/_{NDoF} ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoFProb', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF}-probability ; prob(#chi^{2}/_{NDoF}) ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 1. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} ; p_{T} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt, SumPtTrk12_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} vs. track1+track2 p_{T} ; p_{T} [GeV] ; (p_{T#mu1}+p_{T#mu2}) [GeV] ; # Candidates',
                                               xbins = 25, xmin = 0., xmax = 50.,
                                               ybins = 25, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex R-position ; R [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 10. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxZ', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex Z-position ; Z [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 500. ) ]
        self.Histograms += [ defineHistogram ( 'nBphys', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of di-muon candidates ; # di-muons ; # Events',
                                               xbins = 50, xmin = -0.5, xmax = 49.5 ) ]
        self.Histograms += [ defineHistogram ( 'TotalTime', type = 'TH1F',
                                               title = 'EFTrkMassFex - Total processing time ; t_{tot} [ms] ; # Events',
                                               xbins = 100, xmin = 0., xmax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'VertexingTime', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit processing time ; t_{vtx} [ms] ; # Events',
                                               xbins = 100, xmin = 0., xmax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'TotalTime, VertexingTime', type = 'TH2F',
                                               title = 'EFTrkMassFex - Total vs. vertex fit processing time ; t_{tot} [ms] ; t_{vtx} [ms] ; # Events',
                                               xbins = 100, xmin = 0., xmax = 50.,
                                               ybins = 100, ymin = 0., ymax = 50. ) ]

class TrigEFTrkMassFexOnlineMonitoring_FS(TrigGenericMonitoringToolConfig):
    def __init__ (self, name="EFTrkMassFexOnline_FS"):
        super(TrigEFTrkMassFexOnlineMonitoring_FS, self).__init__(name)
        self.defineTarget("Online")
        self.Histograms  = [ defineHistogram ( 'Errors', type = 'TH1F',
                                               title = 'EFTrkMassFex - Algorithm errors ; ; # Events / Candidates',
                                               xbins = ERROR_number , xmin = -0.5, xmax = ERROR_number - 0.5,
                                               labels = ERROR_labels ) ]
        self.Histograms += [ defineHistogram ( 'Acceptance', type = 'TH1F',
                                               title = 'EFTrkMassFex - Reached steps - acceptance ; ; # Events / Candidates',
                                               xbins = ACCEPTANCE_number , xmin = -0.5, xmax = ACCEPTANCE_number - 0.5,
                                               labels = ACCEPTANCE_labels ) ]
        self.Histograms += [ defineHistogram ( 'ROIEta, ROIPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - ROI direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'nTracks', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of ID tracks ; # Tracks ; # Events',
                                               xbins = 101, xmin = -1.5, xmax = 99.5 ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} - low range ; p_{T} [GeV] ; # Tracks',
                                               xbins = 100, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} ; p_{T} [GeV] ; # Tracks',
                                               xbins = 100, xmin = 0., xmax = 200. ) ]
        self.Histograms += [ defineHistogram ( 'TrkEta, TrkPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta, TrkROIdPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 32, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Pt, Trk2Pt', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks p_{T} correlation ; p_{T#mu1} [GeV] ; p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 25, xmin = 0., xmax = 50.,
                                               ybins = 25, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Eta, Trk2Eta', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #eta correlation ; #eta_{#mu1} ; #eta_{#mu2} ; # Candidates',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 30, ymin = -3.0, ymax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Phi, Trk2Phi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #phi correlation ; #phi_{#mu1} ; #phi_{#mu2} ; # Candidates',
                                               xbins = 32, xmin = -3.2, xmax = 3.2,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta, Trk1Trk2dPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Candidates',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 32, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'SumPtTrk12', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1+Track2 p_{T} ; p_{T#mu1}+p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 1., xmax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass, InvMass_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Before vs. after vertex fit invariant mass ; Fitted mass [GeV] ; Invariant mass [GeV] ; # Candidates',
                                               xbins = 25, xmin = 1., xmax = 6.,
                                               ybins = 25, ymin = 1., ymax = 6. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoF', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF} ; #chi^{2}/_{NDoF} ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoFProb', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF}-probability ; prob(#chi^{2}/_{NDoF}) ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 1. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} ; p_{T} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt, SumPtTrk12_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} vs. track1+track2 p_{T} ; p_{T} [GeV] ; (p_{T#mu1}+p_{T#mu2}) [GeV] ; # Candidates',
                                               xbins = 25, xmin = 0., xmax = 50.,
                                               ybins = 25, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex R-position ; R [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 10. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxZ', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex Z-position ; Z [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 500. ) ]
        self.Histograms += [ defineHistogram ( 'nBphys', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of di-muon candidates ; # di-muons ; # Events',
                                               xbins = 50, xmin = -0.5, xmax = 49.5 ) ]
#        self.Histograms += [ defineHistogram ( 'TotalTime', type = 'TH1F',
#                                               title = 'EFTrkMassFex - Total processing time ; t_{tot} [ms] ; # Events',
#                                               xbins = 100, xmin = 0., xmax = 50. ) ]
#        self.Histograms += [ defineHistogram ( 'VertexingTime', type = 'TH1F',
#                                               title = 'EFTrkMassFex - Vertex fit processing time ; t_{vtx} [ms] ; # Events',
#                                               xbins = 100, xmin = 0., xmax = 50. ) ]
#        self.Histograms += [ defineHistogram ( 'TotalTime, VertexingTime', type = 'TH2F',
#                                               title = 'EFTrkMassFex - Total vs. vertex fit processing time ; t_{tot} [ms] ; t_{vtx} [ms] ; # Events',
#                                               xbins = 100, xmin = 0., xmax = 50.,
#                                               ybins = 100, ymin = 0., ymax = 50. ) ]

class TrigEFTrkMassFexValidationMonitoring_Upsi_FS(TrigGenericMonitoringToolConfig):
    def __init__ (self, name="EFTrkMassFexValidation_Upsi_FS"):
        super(TrigEFTrkMassFexValidationMonitoring_Upsi_FS, self).__init__(name)
        self.defineTarget("Validation")
        self.Histograms  = [ defineHistogram ( 'Errors', type = 'TH1F',
                                               title = 'EFTrkMassFex - Algorithm errors ; ; # Events / Candidates',
                                               xbins = ERROR_number , xmin = -0.5, xmax = ERROR_number - 0.5,
                                               labels = ERROR_labels ) ]
        self.Histograms += [ defineHistogram ( 'Acceptance', type = 'TH1F',
                                               title = 'EFTrkMassFex - Reached steps - acceptance ; ; # Events / Candidates',
                                               xbins = ACCEPTANCE_number , xmin = -0.5, xmax = ACCEPTANCE_number - 0.5,
                                               labels = ACCEPTANCE_labels ) ]
        self.Histograms += [ defineHistogram ( 'ROIEta, ROIPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - ROI direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'nTracks', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of ID tracks ; # Tracks ; # Events',
                                               xbins = 101, xmin = -1.5, xmax = 99.5 ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} - low range ; p_{T} [GeV] ; # Tracks',
                                               xbins = 100, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} ; p_{T} [GeV] ; # Tracks',
                                               xbins = 100, xmin = 0., xmax = 200. ) ]
        self.Histograms += [ defineHistogram ( 'TrkEta, TrkPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta, TrkROIdPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 32, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 140, xmin = 1., xmax = 15. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 140, xmin = 1., xmax = 15. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Pt, Trk2Pt', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks p_{T} correlation ; p_{T#mu1} [GeV] ; p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 25, xmin = 0., xmax = 50.,
                                               ybins = 25, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Eta, Trk2Eta', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #eta correlation ; #eta_{#mu1} ; #eta_{#mu2} ; # Candidates',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 30, ymin = -3.0, ymax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Phi, Trk2Phi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #phi correlation ; #phi_{#mu1} ; #phi_{#mu2} ; # Candidates',
                                               xbins = 32, xmin = -3.2, xmax = 3.2,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta, Trk1Trk2dPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Candidates',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 60, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 64, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'SumPtTrk12', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1+Track2 p_{T} ; p_{T#mu1}+p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 140, xmin = 1., xmax = 15. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass, InvMass_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Before vs. after vertex fit invariant mass ; Fitted mass [GeV] ; Invariant mass [GeV] ; # Candidates',
                                               xbins = 28, xmin = 1., xmax = 15.,
                                               ybins = 28, ymin = 1., ymax = 15. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoF', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF} ; #chi^{2}/_{NDoF} ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoFProb', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF}-probability ; prob(#chi^{2}/_{NDoF}) ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 1. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} ; p_{T} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt, SumPtTrk12_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} vs. track1+track2 p_{T} ; p_{T} [GeV] ; (p_{T#mu1}+p_{T#mu2}) [GeV] ; # Candidates',
                                               xbins = 25, xmin = 0., xmax = 50.,
                                               ybins = 25, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex R-position ; R [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 10. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxZ', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex Z-position ; Z [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 500. ) ]
        self.Histograms += [ defineHistogram ( 'nBphys', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of di-muon candidates ; # di-muons ; # Events',
                                               xbins = 50, xmin = -0.5, xmax = 49.5 ) ]
        self.Histograms += [ defineHistogram ( 'TotalTime', type = 'TH1F',
                                               title = 'EFTrkMassFex - Total processing time ; t_{tot} [ms] ; # Events',
                                               xbins = 100, xmin = 0., xmax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'VertexingTime', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit processing time ; t_{vtx} [ms] ; # Events',
                                               xbins = 100, xmin = 0., xmax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'TotalTime, VertexingTime', type = 'TH2F',
                                               title = 'EFTrkMassFex - Total vs. vertex fit processing time ; t_{tot} [ms] ; t_{vtx} [ms] ; # Events',
                                               xbins = 100, xmin = 0., xmax = 50.,
                                               ybins = 100, ymin = 0., ymax = 50. ) ]

class TrigEFTrkMassFexOnlineMonitoring_Upsi_FS(TrigGenericMonitoringToolConfig):
    def __init__ (self, name="EFTrkMassFexOnline_Upsi_FS"):
        super(TrigEFTrkMassFexOnlineMonitoring_Upsi_FS, self).__init__(name)
        self.defineTarget("Online")
        self.Histograms  = [ defineHistogram ( 'Errors', type = 'TH1F',
                                               title = 'EFTrkMassFex - Algorithm errors ; ; # Events / Candidates',
                                               xbins = ERROR_number , xmin = -0.5, xmax = ERROR_number - 0.5,
                                               labels = ERROR_labels ) ]
        self.Histograms += [ defineHistogram ( 'Acceptance', type = 'TH1F',
                                               title = 'EFTrkMassFex - Reached steps - acceptance ; ; # Events / Candidates',
                                               xbins = ACCEPTANCE_number , xmin = -0.5, xmax = ACCEPTANCE_number - 0.5,
                                               labels = ACCEPTANCE_labels ) ]
        self.Histograms += [ defineHistogram ( 'ROIEta, ROIPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - ROI direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'nTracks', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of ID tracks ; # Tracks ; # Events',
                                               xbins = 101, xmin = -1.5, xmax = 99.5 ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} - low range ; p_{T} [GeV] ; # Tracks',
                                               xbins = 100, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'TrkPt_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - ID tracks p_{T} ; p_{T} [GeV] ; # Tracks',
                                               xbins = 100, xmin = 0., xmax = 200. ) ]
        self.Histograms += [ defineHistogram ( 'TrkEta, TrkPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track direction ; #eta ; #phi ; # Events',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta, TrkROIdPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 30, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 32, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'TrkROIdR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track-ROI #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 140, xmin = 1., xmax = 15. ) ]
        self.Histograms += [ defineHistogram ( 'InvMassNoTrkPtCut_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - p_{T#mu2} > 2 GeV ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 140, xmin = 1., xmax = 15. ) ]
        self.Histograms += [ defineHistogram ( 'InvMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Invariant mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Pt, Trk2Pt', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks p_{T} correlation ; p_{T#mu1} [GeV] ; p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 25, xmin = 0., xmax = 50.,
                                               ybins = 25, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Eta, Trk2Eta', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #eta correlation ; #eta_{#mu1} ; #eta_{#mu2} ; # Candidates',
                                               xbins = 30, xmin = -3.0, xmax = 3.0,
                                               ybins = 30, ymin = -3.0, ymax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Phi, Trk2Phi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Tracks #phi correlation ; #phi_{#mu1} ; #phi_{#mu2} ; # Candidates',
                                               xbins = 32, xmin = -3.2, xmax = 3.2,
                                               ybins = 32, ymin = -3.2, ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta, Trk1Trk2dPhi', type = 'TH2F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta#Delta#phi difference ; #Delta#eta ; #Delta#phi ; # Candidates',
                                               xbins = 30, xmin = 0., xmax = 3.0,
                                               ybins = 32, ymin = 0., ymax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dEta', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#eta difference ; #Delta#eta ; # Tracks',
                                               xbins = 60, xmin = 0., xmax = 3.0 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dPhi', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #Delta#phi difference ; #Delta#phi ; # Tracks',
                                               xbins = 64, xmin = 0., xmax = 3.2 ) ]
        self.Histograms += [ defineHistogram ( 'Trk1Trk2dR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1-Track2 #DeltaR difference ; #DeltaR ; # Tracks',
                                               xbins = 50, xmin = 0., xmax = 5. ) ]
        self.Histograms += [ defineHistogram ( 'SumPtTrk12', type = 'TH1F',
                                               title = 'EFTrkMassFex - Track1+Track2 p_{T} ; p_{T#mu1}+p_{T#mu2} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass - low range ; Mass [GeV] ; # Candidates',
                                               xbins = 140, xmin = 1., xmax = 15. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass_wideRange', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted mass ; Mass [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitMass, InvMass_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Before vs. after vertex fit invariant mass ; Fitted mass [GeV] ; Invariant mass [GeV] ; # Candidates',
                                               xbins = 28, xmin = 1., xmax = 15.,
                                               ybins = 28, ymin = 1., ymax = 15. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoF', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF} ; #chi^{2}/_{NDoF} ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 25. ) ]
        self.Histograms += [ defineHistogram ( 'Chi2toNDoFProb', type = 'TH1F',
                                               title = 'EFTrkMassFex - Vertex fit #chi^{2}/_{NDoF}-probability ; prob(#chi^{2}/_{NDoF}) ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 1. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} ; p_{T} [GeV] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 100. ) ]
        self.Histograms += [ defineHistogram ( 'FitTotalPt, SumPtTrk12_okFit', type = 'TH2F',
                                               title = 'EFTrkMassFex - Fitted di-muon p_{T} vs. track1+track2 p_{T} ; p_{T} [GeV] ; (p_{T#mu1}+p_{T#mu2}) [GeV] ; # Candidates',
                                               xbins = 25, xmin = 0., xmax = 50.,
                                               ybins = 25, ymin = 0., ymax = 50. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxR', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex R-position ; R [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 10. ) ]
        self.Histograms += [ defineHistogram ( 'FitVtxZ', type = 'TH1F',
                                               title = 'EFTrkMassFex - Fitted vertex Z-position ; Z [mm] ; # Candidates',
                                               xbins = 100, xmin = 0., xmax = 500. ) ]
        self.Histograms += [ defineHistogram ( 'nBphys', type = 'TH1F',
                                               title = 'EFTrkMassFex - Number of di-muon candidates ; # di-muons ; # Events',
                                               xbins = 50, xmin = -0.5, xmax = 49.5 ) ]
#        self.Histograms += [ defineHistogram ( 'TotalTime', type = 'TH1F',
#                                               title = 'EFTrkMassFex - Total processing time ; t_{tot} [ms] ; # Events',
#                                               xbins = 100, xmin = 0., xmax = 50. ) ]
#        self.Histograms += [ defineHistogram ( 'VertexingTime', type = 'TH1F',
#                                               title = 'EFTrkMassFex - Vertex fit processing time ; t_{vtx} [ms] ; # Events',
#                                               xbins = 100, xmin = 0., xmax = 50. ) ]
#        self.Histograms += [ defineHistogram ( 'TotalTime, VertexingTime', type = 'TH2F',
#                                               title = 'EFTrkMassFex - Total vs. vertex fit processing time ; t_{tot} [ms] ; t_{vtx} [ms] ; # Events',
#                                               xbins = 100, xmin = 0., xmax = 50.,
#                                               ybins = 100, ymin = 0., ymax = 50. ) ]

