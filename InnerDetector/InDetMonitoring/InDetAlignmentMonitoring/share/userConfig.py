#
# userConfig.py
#
# in this file the user configures the input files, the directories from where the histograms are imported and the kind of output
#
doDebug = True

print " <userConfig> -- START -- "
print "           userPDF:", userPDF
print "   userOuputFolder:", userOuputFolder

outputDir = "../ZmumuTracks" # this sets the output directory where plot gifs will be saved
if (userOuputFolder):
    outputDir = userOuputFolder
    if (doDebug): print " <userConfig> output folder set to: ", outputDir

# output type
oFext="png" # output file extention without the dot
if (userPDF): oFext="pdf"

####
canvasText   = ["Zmumu Selection", "Muon Tracks", "p_{T}>15 GeV",""] #specifies additional canvas text
#canvasText   = ["Data 2016 exp", "AlignTracks", "p_{T}>5 GeV (?)",""] #specifies additional canvas text
if (len(userCanvasText)>=1):
    canvasText = SetCanvasText(userCanvasText)

####
normaliseHistos = True # set to true if you want to normalise to same area
unitArea = False # set to true if you want to draw residual histos to unit area

    
# specify the ROOT histogram files containing the monitoring plots here

SetNextInputFile("/afs/cern.ch/user/m/martis/mywork/ZmumuNtuples/TotalMonitoringDir/ZmumuMC_TotalMonitoring_S1_v0_t3_w.root", "Z#mu#mu MC p_{T}-#eta weighted", kOpenSquare, kRed+1,"SelectedMuonsRefit1_Tracks","")
#SetNextInputFile("/afs/cern.ch/user/m/martis/mywork/ZmumuNtuples/TotalMonitoringDir/Run300600_TotalMonitoring_R207_S1_v0_t1.root", "Run300600 (pre-TS1) Rel20.7", kOpenCircle, kBlack,"SelectedMuonsRefit1_Tracks","")
#SetNextInputFile("/afs/cern.ch/user/m/martis/mywork/ZmumuNtuples/TotalMonitoringDir/Run300600_TotalMonitoring_R207_S1_v3_t1.root", "Run300600 (pre-TS1) Rel21", kOpenSquare, kBlack,"SelectedMuonsRefit1_Tracks","")
#SetNextInputFile("/afs/cern.ch/user/m/martis/mywork/ZmumuNtuples/TotalMonitoringDir/Run303338_TotalMonitoring_R207_S1_v0_t2.root", "Run303338 (pre-d0fix) Rel20.7", kOpenCircle, kOrange+1,"SelectedMuonsRefit1_Tracks","")
SetNextInputFile("/afs/cern.ch/user/m/martis/mywork/ZmumuNtuples/TotalMonitoringDir/Run303338_TotalMonitoring_R207_S1_v3_t1.root", "Run303338 (pre-d0fix) Rel21", kOpenSquare, kGray+3,"SelectedMuonsRefit1_Tracks","")
#SetNextInputFile("/afs/cern.ch/user/m/martis/mywork/ZmumuNtuples/TotalMonitoringDir/Run311402_TotalMonitoring_R207_S1_v2_t1.root", "Run311402 (post-d0fix) Rel20.7", kOpenCircle, kSpring-1,"SelectedMuonsRefit1_Tracks","")
#SetNextInputFile("/afs/cern.ch/user/m/martis/mywork/ZmumuNtuples/TotalMonitoringDir/Run311402_TotalMonitoring_R207_S1_v3_t1.root", "Run311402 (post-d0fix) Rel21", kOpenSquare, kSpring-1,"SelectedMuonsRefit1_Tracks","")
SetNextInputFile("/afs/cern.ch/user/m/martis/mywork/R21_2/athena/InnerDetector/InDetMonitoring/InDetPerformanceMonitoring/share/monitoring.root", "Z#mu#mu MC Pixels Bitsche", kOpenCircle, kGreen+1,"SelectedMuonsRefit1_Tracks","")



# when arguments are passed from command line
print " <userConfig> Number of input files = ", len(userInputFiles)
if (len(userInputFiles)>0):
    # clean current list
    theInputFileList[:]= []
    theMarkerList[:] = []
    theColorList[:] = []
    theLegendList[:] = []
    thePrefix[:] = []
    
    # define parameters
    basicTrackCollection = "AlignTracks_all"
    if (len(userInputTrackCollection)>0): basicTrackCollection = userInputTrackCollection[0]

    if (len(userInputFolder)==0):
        print " ** ERROR ** no input folder given. Please provide an input folder ** "
        exit()
    else:
        print " --> userInputFolder:", userInputFolder
        basicInputFolder = userInputFolder[0]
        
    for i in range(len(userInputFiles)):
        defaultLabel = "File_"+str(i+1)
        defaultMarker = kOpenCircle
        defaultColor = kGray+3
        defaultTrackCollection = basicTrackCollection
        defaultInputFolder = basicInputFolder
        
        thisLabel = defaultLabel
        if (i+1<=len(userInputLabels)): thisLabel = userInputLabels[i]

        thisMarker = defaultMarker
        if (i+1<=len(userInputMarkers)): thisMarker = userInputMarkers[i]

        thisTrackCollection = defaultTrackCollection 
        if (i+1<=len(userInputTrackCollection)): thisTrackCollection = userInputTrackCollection[i]   

        thisInputFolder = defaultInputFolder
        if (i+1<=len(userInputFolder)): thisInputFolder = userInputFolder[i]   

        thisColor = defaultColor
        if (i+1<=len(userColors)): thisColor = userColors[i]   

        if (True):    
            print " file ", i, " --> ", userInputFiles[i]
            print "         label  --> ", thisLabel
            print "         marker --> ", thisMarker
            print "         color  --> ", defaultColor
            print "         trks   --> ", thisTrackCollection
            print "         folder --> ", thisInputFolder
        
        SetNextInputFile(userInputFiles[i], thisLabel, thisMarker, thisColor, thisTrackCollection, thisInputFolder)


#following array determines ROOT file folders. 
# Silicon (IBL, PIX & SCT residuals)
residualsDir = [
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/"
    ] 

# TRT residuals
trtResidualsDir = [
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/",
    "/IDAlignMon/AlignTracks_all/Residuals/"
    ] 

# Track parameters, hits, etc
genericTracksDir = [
    "/IDAlignMon/AlignTracks_all/GenericTracks/",
    "/IDAlignMon/AlignTracks_all/GenericTracks/",
    "/IDAlignMon/AlignTracks_all/GenericTracks/",
    "/IDAlignMon/AlignTracks_all/GenericTracks/",
    "/IDAlignMon/AlignTracks_all/GenericTracks/",
    "/IDAlignMon/AlignTracks_all/GenericTracks/",
    "/IDAlignMon/AlignTracks_all/GenericTracks/",
    "/IDAlignMon/AlignTracks_all/GenericTracks/",
    "/IDAlignMon/AlignTracks_all/GenericTracks/",
    ] 

# Hit efficiency 
hitEffDir = [
    "/IDAlignMon/AlignTracks_all/HitEfficiencies/",
    "/IDAlignMon/AlignTracks_all/HitEfficiencies/",
    "/IDAlignMon/AlignTracks_all/HitEfficiencies/",
    "/IDAlignMon/AlignTracks_all/HitEfficiencies/",
    "/IDAlignMon/AlignTracks_all/HitEfficiencies/",
    "/IDAlignMon/AlignTracks_all/HitEfficiencies/",
    "/IDAlignMon/AlignTracks_all/HitEfficiencies/",
    "/IDAlignMon/AlignTracks_all/HitEfficiencies/",
    "/IDAlignMon/AlignTracks_all/HitEfficiencies/",
    ] 
#following array determines ROOT file directory from which SivsTRT plots are taken
#for each of the 3 files 
siTRTDir = [
    "",
    "",
    ""
    ] 

#following array determines ROOT file directory from which cosmic SplitTracks plots are taken
splitTracksDir = [
    "/IDAlignMon/AlignTracks_NoTriggerSelection/TrackSegments/",
    "/IDAlignMon/AlignTracks_NoTriggerSelection/TrackSegments/",
    "/IDAlignMon/AlignTracks_NoTriggerSelection/TrackSegments/",
    "/IDAlignMon/AlignTracks_NoTriggerSelection/TrackSegments/",
    "/IDAlignMon/AlignTracks_NoTriggerSelection/TrackSegments/",
    "/IDAlignMon/AlignTracks_NoTriggerSelection/TrackSegments/",
    "/IDAlignMon/AlignTracks_NoTriggerSelection/TrackSegments/",
    "/IDAlignMon/AlignTracks_NoTriggerSelection/TrackSegments/",
    "/IDAlignMon/AlignTracks_NoTriggerSelection/TrackSegments/"
    ] 

#
# in case user give files in the new format (March-April 2015)
#

if len(theInputFileList)>0:
    rootFileNames = []
    markerStyles = []
    markerColors = []
    legendTitles = []
    for i in range(len(theInputFileList)):
        rootFileNames.append(theInputFileList[i])
        markerStyles.append(theMarkerList[i])
        markerColors.append(theColorList[i])
        legendTitles.append(theLegendList[i])
        if (len(theTrackCollectionList[i])>0):
            residualsDir[i] = "/"+thePrefix[i]+"/IDAlignMon/"+ theTrackCollectionList[i] + "/Residuals/"
            trtResidualsDir[i] = "/"+thePrefix[i]+"/IDAlignMon/"+ theTrackCollectionList[i] + "/Residuals/"
            genericTracksDir[i] ="/"+thePrefix[i]+"/IDAlignMon/"+ theTrackCollectionList[i] + "/GenericTracks/"
            hitEffDir[i] = "/"+thePrefix[i]+"/IDAlignMon/"+ theTrackCollectionList[i] + "/HitEfficiencies/"
            splitTracksDir[i] = "/"+thePrefix[i]+splitTracksDir[i]
            
            
##----------------------------
