/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Name    : NnClusterizationFactory.cxx
/// Package : SiClusterizationTool 
/// Author  : Giacinto Piacquadio (PH-ADE-ID)
/// Created : January 2011
///
/// DESCRIPTION: Load neural networks used for clustering 
///              and deal with:
///              1) estimate of number of particles
///              2) estimate of position of cluster / subclusters
///              3) estimate of error in all cases 
///                 (full PDF or just RMS)
///////////////////////////////////////////////////////////////////////////////////////////////////////



#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include <TMath.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
#include "TrkNeuralNetworkUtils/NeuralNetworkToHistoTool.h"
#include "TrkNeuralNetworkUtils/TTrainedNetwork.h"
#include "SiClusterizationTool/NnClusterizationFactory.h"
#include "SiClusterizationTool/NnNormalization.h"

//for position estimate and clustering
#include "InDetIdentifier/PixelID.h"
#include "InDetPrepRawData/PixelCluster.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/PixelModuleDesign.h"
#include "InDetReadoutGeometry/SiLocalPosition.h"

#include "TrkEventPrimitives/ParamDefs.h"

#include "PixelConditionsServices/IPixelCalibSvc.h"

// utilities
#include "PathResolver/PathResolver.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"

//get std::isnan()
#include <cmath>



namespace InDet {

  NnClusterizationFactory::NnClusterizationFactory(const std::string& name,
                                                   const std::string& n, const IInterface* p):
          AthAlgTool(name, n,p),
          m_loadTTrainedNetworks(true),
          m_configTTrainedNetwork(""),
          m_NetworkEstimateNumberParticles(0),
          m_layerInfoHistogram("LayersInfo"),
          m_layerPrefix("Layer"), 
          m_weightIndicator("_weights"),
          m_thresholdIndicator("_thresholds"),
          m_networkToHistoTool("Trk::NeuralNetworkToHistoTool/NeuralNetworkToHistoTool"),
          m_calibSvc("PixelCalibSvc", name),
          m_useToT(true),
          m_doRunI(false),
          m_correctLorShiftBarrelWithTracks(0.)
  {
    // histogram loading, TTrainedNetwork format
    declareProperty("TTrainedNetwork_configFile",   m_configTTrainedNetwork);
    declareProperty("LayerInfoHistogram",           m_layerInfoHistogram);
    declareProperty("LayerPrefix",                  m_layerPrefix);
    declareProperty("LayerWeightIndicator",         m_weightIndicator);
    declareProperty("LayerThresholdIndicator",      m_thresholdIndicator);
    declareProperty("NetworkToHistoTool",m_networkToHistoTool);    
    // further properties
    declareProperty("PixelCalibSvc",m_calibSvc);
    declareProperty("LoadTTrainedNetworks",m_loadTTrainedNetworks);
    declareProperty("useToT",m_useToT);
    declareProperty("doRunI",m_doRunI);
    declareProperty("correctLorShiftBarrelWithTracks",m_correctLorShiftBarrelWithTracks);
        
    declareInterface<NnClusterizationFactory>(this);
  } 
  
/////////////////////////////////////////////////////////////////////////////////////
/// Destructor - check up memory allocation
/// delete any memory allocation on the heap
  
  NnClusterizationFactory::~NnClusterizationFactory() {
    
    if (m_NetworkEstimateNumberParticles)
    {
      delete m_NetworkEstimateNumberParticles;
      m_NetworkEstimateNumberParticles=0;
    }
    for (unsigned int i=0;i<m_NetworkEstimateImpactPoints.size();i++)
    {
      delete m_NetworkEstimateImpactPoints[i];
      m_NetworkEstimateImpactPoints[i]=0;
    }
    for (unsigned int i=0;i<m_NetworkEstimateImpactPointErrorsX.size();i++)
    {
      delete m_NetworkEstimateImpactPointErrorsX[i];
      m_NetworkEstimateImpactPointErrorsX[i]=0;
    }
    for (unsigned int i=0;i<m_NetworkEstimateImpactPointErrorsY.size();i++)
    {
      delete m_NetworkEstimateImpactPointErrorsY[i];
      m_NetworkEstimateImpactPointErrorsY[i]=0;
    }

  }
  
  StatusCode NnClusterizationFactory::initialize() {
        
    // Read config from the CalibArea on cvmfs.
    m_configTTrainedNetwork = PathResolverFindCalibFile(m_configTTrainedNetwork);
    ATH_MSG_INFO("NN file resolved to: " << m_configTTrainedNetwork);

    // We will use this to read the TTrainedNetwork-based inputs    
    if (m_networkToHistoTool.retrieve().isFailure())
    {
      ATH_MSG_ERROR("Could not load: " << m_networkToHistoTool);
      return StatusCode::FAILURE;
    }

    // Set them up:
    if (! nnSetup()) return StatusCode::FAILURE;

    // We will use this to retrieve pixel conditions.    
    if (m_calibSvc.retrieve().isFailure()){
        ATH_MSG_ERROR("Could not retrieve " << m_calibSvc);
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }
  
  
std::vector<double> NnClusterizationFactory::assembleInput(NNinput& input,
                                                             int sizeX,
                                                             int sizeY)
{

    std::vector<double> inputData;
    for (int u=0;u<sizeX;u++)
    {
      for (int s=0;s<sizeY;s++)
      {
        if (m_useToT)
        {
          inputData.push_back(input.matrixOfToT[u][s]);
        }
        else
        {
          inputData.push_back(input.matrixOfToT[u][s]);
        }
      }
    }
    for (int s=0;s<sizeY;s++)
    {
      inputData.push_back(input.vectorOfPitchesY[s]);
    }
    inputData.push_back(input.ClusterPixLayer);
    inputData.push_back(input.ClusterPixBarrelEC);

    inputData.push_back(input.phi);
    inputData.push_back(input.theta);
    
    return inputData;

  }
  
  std::vector<double> NnClusterizationFactory::estimateNumberOfParticles(const InDet::PixelCluster& pCluster,
                                                                         const Trk::Surface& pixelSurface,
                                                                         const Trk::TrackParameters& trackParsAtSurface,
                                                                         int sizeX,
                                                                         int sizeY)
  {
    
    Amg::Vector3D dummyBS(0,0,0);

    double tanl=0;

    NNinput* input=createInput(pCluster,
                               dummyBS,
                               tanl,
                               sizeX,
                               sizeY);
    
    if (input==0)
    {
      return std::vector<double>();
    }
    
    addTrackInfoToInput(input,pixelSurface,trackParsAtSurface,tanl);
    
    std::vector<double> inputData=assembleInput(*input,sizeX,sizeY);
    


    std::vector<double> resultNN;

    resultNN = m_NetworkEstimateNumberParticles->calculateNormalized(inputData);

    ATH_MSG_VERBOSE(" Prob of n. particles (1): " << resultNN[0] << " (2): " << resultNN[1] << " (3): " << resultNN[2]);

    delete input;
    input=0;

    return resultNN;
  }
   
  std::vector<Amg::Vector2D> NnClusterizationFactory::estimatePositions(const InDet::PixelCluster& pCluster,
                                                                             const Trk::Surface& pixelSurface,
                                                                             const Trk::TrackParameters& trackParsAtSurface,
                                                                             std::vector<Amg::MatrixX> & errors,
                                                                             int numberSubClusters,
                                                                             int sizeX,
                                                                             int sizeY)
  {
    

    Amg::Vector3D dummyBS(0,0,0);

    double tanl=0;
    
    NNinput* input=createInput(pCluster,
                               dummyBS,
                               tanl,
                               sizeX,
                               sizeY);
    
    if (input==0)
    {
       return std::vector<Amg::Vector2D>();
    }
    
    addTrackInfoToInput(input,pixelSurface,trackParsAtSurface,tanl);

    std::vector<double> inputData=assembleInput(*input,sizeX,sizeY);

    // We only go up to 3: if this looks like more just call it that
    if (numberSubClusters > 3) numberSubClusters = 3;

    std::vector<double> positions = m_NetworkEstimateImpactPoints[numberSubClusters-1]->calculateNormalized(inputData);
    std::vector<Amg::Vector2D> myPositions=getPositionsFromOutput(positions,*input,pCluster,sizeX,sizeY);

    for (int cluster=0; cluster < numberSubClusters; cluster++) {
      ATH_MSG_VERBOSE(" RAW vs FINAL estimated positions (" << cluster+1 << ")");
      ATH_MSG_VERBOSE("\tx: " << back_posX(positions[cluster*2],false) << " y: " << back_posY(positions[cluster*2+1]));
      ATH_MSG_VERBOSE("\tx: " << myPositions[cluster][Trk::locX] << " y: " << myPositions[cluster][Trk::locY]);
    }

    std::vector<double> inputDataNew=inputData;
    for (auto pos : positions) inputDataNew.push_back(pos);

    std::vector<double> errorsX;
    std::vector<double> errorsY;

    errorsX=m_NetworkEstimateImpactPointErrorsX[numberSubClusters-1]->calculateNormalized(inputDataNew);
    errorsY=m_NetworkEstimateImpactPointErrorsY[numberSubClusters-1]->calculateNormalized(inputDataNew);    

    std::vector<Amg::MatrixX> errorMatrices;
    getErrorMatrixFromOutput(errorsX,errorsY,errorMatrices,numberSubClusters);


    for (unsigned int i=0;i<myPositions.size();i++)
    {
      errors.push_back(errorMatrices[i]);
    }
  
    delete input;
    input=0;
    return myPositions;
  }
  

    
  void NnClusterizationFactory::getErrorMatrixFromOutput(std::vector<double>& outputX,
                                                         std::vector<double>& outputY,
                                                         std::vector<Amg::MatrixX>& errorMatrix,
                                                         int nParticles)
  {
    
    int sizeOutputX=outputX.size()/nParticles;
    int sizeOutputY=outputY.size()/nParticles;
    
    double minimumX=-errorHalfIntervalX(nParticles);
    double maximumX=errorHalfIntervalX(nParticles);

    double minimumY=-errorHalfIntervalY(nParticles);
    double maximumY=errorHalfIntervalY(nParticles);

    
    //X=0...sizeOutput-1
    //Y=minimum+(maximum-minimum)/sizeOutput*(X+1./2.)
    
    for (int i=0;i<nParticles;i++)
    {
      double sumValuesX=0;
      for (int u=0;u<sizeOutputX;u++)
      {
        sumValuesX+=outputX[i*sizeOutputX+u];
      }
      double sumValuesY=0;
      for (int u=0;u<sizeOutputY;u++)
      {
        sumValuesY+=outputY[i*sizeOutputY+u];
      }


      ATH_MSG_VERBOSE(" minimumX: " << minimumX << " maximumX: " << maximumX << " sizeOutputX " << sizeOutputX);
      ATH_MSG_VERBOSE(" minimumY: " << minimumY << " maximumY: " << maximumY << " sizeOutputY " << sizeOutputY);
      

      double RMSx=0;
      for (int u=0;u<sizeOutputX;u++)
      {
        //GP This is wrong...
        //RMSx+=outputX[i*sizeOutputX+u]/sumValuesX*std::pow(minimumX+(maximumX-minimumX)/(double)sizeOutputX*(u+1./2.),2);
        RMSx+=outputX[i*sizeOutputX+u]/sumValuesX*std::pow(minimumX+(maximumX-minimumX)/(double)(sizeOutputX-2)*(u-1./2.),2);
      }
      RMSx=sqrt(RMSx);//computed error!

      ATH_MSG_VERBOSE(" first Iter RMSx: " << RMSx);
      
      double intervalErrorX=3*RMSx;

      //now recompute between -3*RMSx and +3*RMSx
      int minBinX=(int)(1+(-intervalErrorX-minimumX)/(maximumX-minimumX)*(double)(sizeOutputX-2));
      int maxBinX=(int)(1+(intervalErrorX-minimumX)/(maximumX-minimumX)*(double)(sizeOutputX-2));

      if (maxBinX>sizeOutputX-1) maxBinX=sizeOutputX-1;
      if (minBinX<0) minBinX=0;

      ATH_MSG_VERBOSE(" minBinX: " << minBinX << " maxBinX: " << maxBinX );

      RMSx=0;
      for (int u=minBinX;u<maxBinX+1;u++)
      {
        RMSx+=outputX[i*sizeOutputX+u]/sumValuesX*std::pow(minimumX+(maximumX-minimumX)/(double)(sizeOutputX-2)*(u-1./2.),2);
      }
      RMSx=sqrt(RMSx);//computed error!
      
      double RMSy=0;
      for (int u=0;u<sizeOutputY;u++)
      {
        RMSy+=outputY[i*sizeOutputY+u]/sumValuesY*std::pow(minimumY+(maximumY-minimumY)/(double)(sizeOutputY-2)*(u-1./2.),2);
      }
      RMSy=sqrt(RMSy);//computed error!

      ATH_MSG_VERBOSE("first Iter RMSy: " << RMSy );

      double intervalErrorY=3*RMSy;

      //now recompute between -3*RMSy and +3*RMSy
      int minBinY=(int)(1+(-intervalErrorY-minimumY)/(maximumY-minimumY)*(double)(sizeOutputY-2));
      int maxBinY=(int)(1+(intervalErrorY-minimumY)/(maximumY-minimumY)*(double)(sizeOutputY-2));
      
      if (maxBinY>sizeOutputY-1) maxBinY=sizeOutputY-1;
      if (minBinY<0) minBinY=0;

      ATH_MSG_VERBOSE("minBinY: " << minBinY << " maxBinY: " << maxBinY );

      RMSy=0;
      for (int u=minBinY;u<maxBinY+1;u++)
      {
        RMSy+=outputY[i*sizeOutputY+u]/sumValuesY*std::pow(minimumY+(maximumY-minimumY)/(double)(sizeOutputY-2)*(u-1./2.),2);
      }
      RMSy=sqrt(RMSy);//computed error!
     
 
      ATH_MSG_VERBOSE("Computed error, sigma(X) " << RMSx << " sigma(Y) " << RMSy );

      Amg::MatrixX erm(2,2);
      erm.setZero();
      erm(0,0)=RMSx*RMSx;
      erm(1,1)=RMSy*RMSy;
      errorMatrix.push_back(erm);
       
    }//end nParticles

    return;
  
  }//getErrorMatrixFromOutput
  
    
  std::vector<Amg::Vector2D> NnClusterizationFactory::getPositionsFromOutput(std::vector<double> & output,
                                                                                  NNinput & input,
                                                                                  const InDet::PixelCluster& pCluster,
                                                                                  int /* sizeX */,
                                                                                  int /* sizeY */)
  {
    std::vector<Amg::Vector2D> invalidResult;
    ATH_MSG_VERBOSE(" Translating output back into a position " );
    const InDetDD::SiDetectorElement* element=pCluster.detectorElement();//DEFINE
    const InDetDD::PixelModuleDesign* design
        (dynamic_cast<const InDetDD::PixelModuleDesign*>(&element->design()));
		if (not design){
			ATH_MSG_ERROR("Dynamic cast failed at line "<<__LINE__<<" of NnClusterizationFactory.cxx.");
			return invalidResult;
		}
    int numParticles=output.size()/2;


    int columnWeightedPosition=input.columnWeightedPosition;
    int rowWeightedPosition=input.rowWeightedPosition;
    
    ATH_MSG_VERBOSE(" REF POS columnWeightedPos: " << columnWeightedPosition << " rowWeightedPos: " << rowWeightedPosition );

    std::vector<Amg::Vector2D> positions;
    for (int u=0;u<numParticles;u++)
    {
      double posXid;
      double posYid;

	    posXid=output[2*u]+rowWeightedPosition;
	    posYid=output[2*u+1]+columnWeightedPosition;
      
      ATH_MSG_VERBOSE(" N. particle: " << u << " idx posX " << posXid << " posY " << posYid );

      int posXid_int=(int)(posXid+0.5);
      int posYid_int=(int)(posYid+0.5);
      
      ATH_MSG_VERBOSE(" N. particle: " << u << " TO INTEGER idx posX " << posXid_int << " posY " << posYid_int );
      
      InDetDD::SiLocalPosition siLocalPositionDiscrete(design->positionFromColumnRow(posYid_int,posXid_int)); 
      InDetDD::SiCellId cellIdOfPositionDiscrete=design->cellIdOfPosition(siLocalPositionDiscrete);
      if (!cellIdOfPositionDiscrete.isValid())
      {
        ATH_MSG_WARNING(" Cell is outside validity region with index Y: " << posYid_int << " and index X: " << posXid_int << ". Not foreseen... " );
      }
      
      InDetDD::SiDiodesParameters diodeParameters = design->parameters(cellIdOfPositionDiscrete);
      double pitchY = diodeParameters.width().xEta();
      double pitchX = diodeParameters.width().xPhi();
      ATH_MSG_VERBOSE(" Translated weighted position : " << siLocalPositionDiscrete.xPhi() 
                                                 << " Translated weighted position : " << siLocalPositionDiscrete.xEta() );

      //FOR TEST
      InDetDD::SiLocalPosition siLocalPositionDiscreteOneRowMoreOneColumnMore(design->positionFromColumnRow(posYid_int+1,posXid_int+1)); 
      ATH_MSG_VERBOSE(" Translated weighted position +1col +1row phi: " << siLocalPositionDiscreteOneRowMoreOneColumnMore.xPhi() 
                                                 << " Translated weighted position +1col +1row eta: " << siLocalPositionDiscreteOneRowMoreOneColumnMore.xEta() );


      ATH_MSG_VERBOSE("PitchY: " << pitchY << " pitchX " << pitchX );

      InDetDD::SiLocalPosition siLocalPositionAdd(pitchY*(posYid-(double)posYid_int),
                                                  pitchX*(posXid-(double)posXid_int));

      

      // Is this shift due to an actual detector alignment issue present in Run 1?
      // Or is it due to some peculiarity in the NN training at that time?
      double lorentzShift=element->getLorentzCorrection();
      if (input.ClusterPixBarrelEC == 0)
      {
          lorentzShift+=m_correctLorShiftBarrelWithTracks;
      }

      InDetDD::SiLocalPosition siLocalPosition(siLocalPositionDiscrete.xEta()+pitchY*(posYid-(double)posYid_int),
                                               siLocalPositionDiscrete.xPhi()+pitchX*(posXid-(double)posXid_int)+lorentzShift);


      ATH_MSG_VERBOSE(" Translated final position phi: " << siLocalPosition.xPhi() << " eta: " <<  siLocalPosition.xEta() );

      if (siLocalPositionDiscrete.xPhi() > design->width()/2.)
      {
        siLocalPosition=InDetDD::SiLocalPosition(siLocalPositionDiscrete.xEta()+pitchY*(posYid-(double)posYid_int),
                                                 design->width()/2.-1e-6);
        ATH_MSG_WARNING(" Corrected out of boundary cluster from  x(phi): " << siLocalPositionDiscrete.xPhi()+pitchX*(posXid-(double)posXid_int) 
                        << " to: " << design->width()/2.-1e-6);
                                                 
      }
      else if (siLocalPositionDiscrete.xPhi() < -design->width()/2.)
      {
        siLocalPosition=InDetDD::SiLocalPosition(siLocalPositionDiscrete.xEta()+pitchY*(posYid-(double)posYid_int),
                                                 -design->width()/2.+1e-6);
        ATH_MSG_WARNING(" Corrected out of boundary cluster from  x(phi): " << siLocalPositionDiscrete.xPhi()+pitchX*(posXid-(double)posXid_int) 
                        << " to: " << -design->width()/2.+1e-6);
      }
      

      positions.push_back(Amg::Vector2D(siLocalPosition));
    }//iterate over all particles
    return positions;
  }
  
    
  void NnClusterizationFactory::addTrackInfoToInput(NNinput* input,
                                                    const Trk::Surface& pixelSurface,
                                                    const Trk::TrackParameters& trackParsAtSurface,
                                                    const double tanl)//  const Trk::Surface& pixelSurface = pcot->associatedSurface();
  {
    
    Amg::Vector3D particleDir = trackParsAtSurface.momentum().unit();
    Amg::Vector3D localIntersection = pixelSurface.transform().inverse().linear() * particleDir;
    localIntersection *= 0.250/cos(localIntersection.theta());
    float trackDeltaX = (float)localIntersection.x();
    float trackDeltaY = (float)localIntersection.y();
    input->theta=TMath::ATan2(trackDeltaY,0.250);
    input->phi=TMath::ATan2(trackDeltaX,0.250);

    ATH_MSG_VERBOSE("Angle phi bef Lorentz corr: " << input->phi );

    input->phi=atan(tan(input->phi)-tanl);
    
    ATH_MSG_VERBOSE(" From track: angle phi: " << input->phi << " theta: " << input->theta );
  }
  
  
  NNinput* NnClusterizationFactory::createInput(const InDet::PixelCluster& pCluster,
                                                Amg::Vector3D & beamSpotPosition,
                                                double & tanl,
                                                int sizeX,
                                                int sizeY)
{

  ATH_MSG_VERBOSE(" Starting creating input from cluster " );

  const InDetDD::SiDetectorElement* element=pCluster.detectorElement();
  if (element==0) {
    ATH_MSG_ERROR("Could not get detector element");
    return 0;
  }
  const AtlasDetectorID* aid = element->getIdHelper();
  if (aid==0)
  {
    ATH_MSG_ERROR("Could not get ATLASDetectorID");
    return 0;
  }
  const PixelID* pixelIDp=dynamic_cast<const PixelID*>(aid);
  if (!pixelIDp){
    ATH_MSG_ERROR("Could not get PixelID pointer");
    return nullptr;
  } 
  const PixelID& pixelID = *pixelIDp;
  
  NNinput* input=new NNinput();

  //Identifier elementID = element->identify();
  const InDetDD::PixelModuleDesign* design
      (dynamic_cast<const InDetDD::PixelModuleDesign*>(&element->design()));
  if (not design){
    ATH_MSG_ERROR("Dynamic cast failed at line "<<__LINE__<<" of NnClusterizationFactory.cxx.");
    delete input;input=nullptr;
    return nullptr;
  }

//  const InDet::PixelCluster* pCluster  = pcot->prepRawData();
  const std::vector<Identifier>& rdos  = pCluster.rdoList();  

  ATH_MSG_VERBOSE(" Number of RDOs: " << rdos.size() );

  const std::vector<float>& chList     = pCluster.chargeList();
  const std::vector<int>&  totList     = pCluster.totList();
  std::vector<float> chListRecreated; 

  ATH_MSG_VERBOSE(" Number of charges: " << chList.size() );

  std::vector<int>::const_iterator tot = totList.begin();    

  std::vector<Identifier>::const_iterator rdosBegin = rdos.begin();
  std::vector<Identifier>::const_iterator rdosEnd = rdos.end();
 
  std::vector<int>  totListRecreated;
  std::vector<int>::const_iterator totRecreated = totListRecreated.begin();    

  //if (!chList.size() && totList.size()){
  //
  // Recreate both charge list and ToT list to correct for the IBL ToT overflow (and later for small hits):
      ATH_MSG_VERBOSE("Charge list is not filled ... re-creating it.");
      for ( ; rdosBegin!= rdosEnd &&  tot != totList.end(); ++tot, ++rdosBegin, ++totRecreated ){
           // recreate the charge: should be a method of the calibSvc
        int tot0 = *tot;
        float ch = m_calibSvc->getCharge(*rdosBegin,tot0);
        chListRecreated.push_back(ch);
        totListRecreated.push_back(tot0);
      }
      // reset the rdo iterator
      rdosBegin = rdos.begin();
      rdosEnd = rdos.end();
      // and the tot iterator
      tot = totList.begin();
      totRecreated = totListRecreated.begin();
  //}

  // take the original charge list or the recreated one
  // std::vector<float>::const_iterator charge = chList.size() ? chList.begin() : chListRecreated.begin();
  // std::vector<float>::const_iterator chargeEnd = chList.size() ? chList.end() : chListRecreated.end();

  // Always use recreated charge and ToT lists:
     std::vector<float>::const_iterator charge = chListRecreated.begin();
     std::vector<float>::const_iterator chargeEnd = chListRecreated.end();
     tot = totListRecreated.begin();
     std::vector<int>::const_iterator totEnd = totListRecreated.end();
 

  InDetDD::SiLocalPosition sumOfWeightedPositions(0,0,0);
  double sumOfTot=0;

  int rowMin = 999;
  int rowMax = 0;
  int colMin = 999;
  int colMax = 0;

  for (; rdosBegin!= rdosEnd && charge != chargeEnd && tot != totEnd; ++rdosBegin, ++charge, ++tot)
  {

    Identifier rId =  *rdosBegin;
    int row = pixelID.phi_index(rId);
    int col = pixelID.eta_index(rId);

    if(msgLvl(MSG::VERBOSE)) 
    {
      if (!m_useToT){
        ATH_MSG_VERBOSE(" Adding pixel row: " << row << " col: " << col << " charge: " << *charge << " tot " << *tot );
      }
      else
      {
        ATH_MSG_VERBOSE(" Adding pixel row: " << row << " col: " << col << " tot " << *tot );
      }
    }

    InDetDD::SiLocalPosition siLocalPosition
        (design->positionFromColumnRow(col,row)); 
    

    if (!m_useToT)
    {
      sumOfWeightedPositions += (*charge)*siLocalPosition;
      sumOfTot += (*charge);
    }
    else
    {
      sumOfWeightedPositions += ((double)(*tot))*siLocalPosition;
      sumOfTot += (double)(*tot);
    }
    

    if (row < rowMin){ 
      rowMin = row; 
    }
    
    if (row > rowMax){
      rowMax = row;
    }
    
    if (col < colMin){
      colMin = col;
    }
    
    if (col > colMax){
      colMax = col;
    }
  }
  sumOfWeightedPositions /= sumOfTot;
  
  //what you want to know is simple:
  //just the row and column of this average position!

  InDetDD::SiCellId cellIdWeightedPosition=design->cellIdOfPosition(sumOfWeightedPositions);

  if (!cellIdWeightedPosition.isValid())
  {
    ATH_MSG_WARNING(" Weighted position is on invalid CellID." );
  }

  int columnWeightedPosition=cellIdWeightedPosition.etaIndex();
  int rowWeightedPosition=cellIdWeightedPosition.phiIndex();

  ATH_MSG_VERBOSE(" weighted pos row: " << rowWeightedPosition << " col: " << columnWeightedPosition );
  
  int centralIndexX=(sizeX-1)/2;
  int centralIndexY=(sizeY-1)/2;


  if (abs(rowWeightedPosition-rowMin)>centralIndexX ||
      abs(rowWeightedPosition-rowMax)>centralIndexX)
  {
    ATH_MSG_VERBOSE(" Cluster too large rowMin" << rowMin << " rowMax " << rowMax << " centralX " << centralIndexX);
    delete input;
    input=0;
    return 0;
  }
  
  if (abs(columnWeightedPosition-colMin)>centralIndexY ||
      abs(columnWeightedPosition-colMax)>centralIndexY)
  {
    ATH_MSG_VERBOSE(" Cluster too large colMin" << colMin << " colMax " << colMax << " centralY " << centralIndexY);
    delete input;
    input=0;
    return 0;
  }
  
  std::vector<std::vector<float> > matrixOfToT;
  std::vector<float> vectorOfPitchesY;

  for (int a=0;a<sizeX;a++)
  {
    std::vector<float> Yvector;
    for (int b=0;b<sizeY;b++)
    {
      Yvector.push_back(0);
    }
    input->matrixOfToT.push_back(Yvector);
  }
  for (int b=0;b<sizeY;b++)
  {
    input->vectorOfPitchesY.push_back(0.4);
  }

  rdosBegin = rdos.begin();
  //charge = chList.size() ? chList.begin() : chListRecreated.begin();
  //chargeEnd = chList.size() ? chList.end() : chListRecreated.end();
  //tot = totList.begin();    
  charge = chListRecreated.begin();
  chargeEnd = chListRecreated.end();
  tot = totListRecreated.begin();    

  ATH_MSG_VERBOSE(" Putting together the n. " << rdos.size() << " rdos into a matrix." );

  Identifier pixidentif=pCluster.identify();

  input->etaModule=(int)pixelID.eta_module(pixidentif);

  input->ClusterPixLayer=(int)pixelID.layer_disk(pixidentif);
  input->ClusterPixBarrelEC=(int)pixelID.barrel_ec(pixidentif);

  for (; charge != chargeEnd && rdosBegin!= rdosEnd; ++rdosBegin, ++charge, ++tot)
  {
    Identifier rId =  *rdosBegin;
    int absrow = pixelID.phi_index(rId)-rowWeightedPosition+centralIndexX;
    int abscol = pixelID.eta_index(rId)-columnWeightedPosition+centralIndexY;

    if(msgLvl(MSG::VERBOSE)) 
    {
      if (!m_useToT)
      {
        ATH_MSG_VERBOSE(" phi Index: " << pixelID.phi_index(rId) << " absrow: " << absrow << " eta Idx: " << pixelID.eta_index(rId) << " abscol: " << abscol << " charge " << *charge );
      }
      else
      {
        ATH_MSG_VERBOSE(" phi Index: " << pixelID.phi_index(rId) << " absrow: " << absrow << " eta Idx: " << pixelID.eta_index(rId) << " abscol: " << abscol << " tot " << *tot );
      }
    }
    

    if (absrow <0 || absrow > sizeX)
    {
      ATH_MSG_WARNING(" problem with index: " << absrow << " min: " << 0 << " max: " << sizeX);
      delete input;
      input=0;
      return 0;
    }
    if (abscol <0 || abscol > sizeY)
    {
      ATH_MSG_WARNING(" problem with index: " << abscol << " min: " << 0 << " max: " << sizeY);
      delete input;
      input=0;
      return 0;
    }
    InDetDD::SiCellId cellId = element->cellIdFromIdentifier(*rdosBegin);
    InDetDD::SiDiodesParameters diodeParameters = design->parameters(cellId);
    double pitchY = diodeParameters.width().xEta();

    ATH_MSG_VERBOSE(" PitchY: " << pitchY );

    if (!m_useToT)
    {
      input->matrixOfToT[absrow][abscol]=*charge;
    }
    else
    {
      input->matrixOfToT[absrow][abscol]=(double)(*tot);
      
      // No IBL if running on Run 1 data (except certain simulation samples).
      // For Run 2, consider IBL.
      if(! m_doRunI){
	
      	if ( input->ClusterPixLayer==0 && input->ClusterPixBarrelEC==0)
      	  {
      	    input->matrixOfToT[absrow][abscol]*=3;
      	  }
      }
    
    } 
    

    
    if (fabs(pitchY-0.4)>1e-5)
      {
	input->vectorOfPitchesY[abscol]=pitchY;

      }
  }//end iteration on rdos
  
  ATH_MSG_VERBOSE(" eta module: " << input->etaModule );
  ATH_MSG_VERBOSE(" Layer number: " << input->ClusterPixLayer << " Barrel / endcap: " << input->ClusterPixBarrelEC );
    
  const Amg::Vector2D& prdLocPos    = pCluster.localPosition();
  InDetDD::SiLocalPosition centroid(prdLocPos);
  Amg::Vector3D globalPos = element->globalPosition(centroid);
  Amg::Vector3D my_track = globalPos-beamSpotPosition;
  
  Amg::Vector3D my_normal = element->normal();
  Amg::Vector3D my_phiax = element->phiAxis();
  Amg::Vector3D my_etaax = element->etaAxis();
  float trkphicomp = my_track.dot(my_phiax);
  float trketacomp = my_track.dot(my_etaax);
  float trknormcomp = my_track.dot(my_normal);
  double bowphi = atan2(trkphicomp,trknormcomp);
  double boweta = atan2(trketacomp,trknormcomp);
  tanl = element->getTanLorentzAnglePhi();
  if(bowphi > M_PI/2) bowphi -= M_PI;
  if(bowphi < -M_PI/2) bowphi += M_PI;
  int readoutside = design->readoutSide();
  double angle = atan(tan(bowphi)-readoutside*tanl);
  input->phi=angle;

  ATH_MSG_VERBOSE(" Angle theta bef corr: " << boweta );
  
  if (boweta>TMath::Pi()/2.) boweta-=TMath::Pi();
  if (boweta<-TMath::Pi()/2.) boweta+=TMath::Pi();

  input->theta=boweta;
  
  ATH_MSG_VERBOSE(" Angle phi: " << angle << " theta: " << boweta );

  input->rowWeightedPosition=rowWeightedPosition;
  input->columnWeightedPosition=columnWeightedPosition;

  ATH_MSG_VERBOSE(" RowWeightedPosition: " << rowWeightedPosition << " ColWeightedPosition: " << columnWeightedPosition );

  return input;

}//end create NNinput function


  TTrainedNetwork* NnClusterizationFactory::retrieveNetwork(std::unique_ptr<TFile> & ifile, const std::string & folder)
  {
    
    std::vector<TH1*> retrievedHistos;

    // the information about the layers    
    TH1* histolayers = (TH1*) ifile->Get((folder+m_layerInfoHistogram).c_str());

    if (!histolayers)
    {
        ATH_MSG_ERROR("Could not get histogram " << folder+m_layerInfoHistogram << " from file: " << m_configTTrainedNetwork);
        return 0;
    }
    retrievedHistos.push_back(histolayers);
    
    std::string InputInfoHistogram("InputsInfo");
    TH2* histoinputs = (TH2*) ifile->Get((folder+InputInfoHistogram).c_str());
    if (!histoinputs) {
      ATH_MSG_ERROR("Could not get histogram " << folder+InputInfoHistogram << " from file: " << m_configTTrainedNetwork);
      return 0;        
    }
    retrievedHistos.push_back(histoinputs);

    // retrieve the number of hidden layers from the LayerInfo histogram
    Int_t nHidden = histolayers->GetNbinsX()-2;
    ATH_MSG_VERBOSE(" Retrieving calibration: " << folder << " for NN with: " << nHidden << " hidden layers.");

    // loop and retrieve the hidden layer histograms
    for (Int_t i=0; i<nHidden+1; ++i)
    {
      // weights histograms
      TString weightName(m_layerPrefix);
      weightName+=i;
      weightName+=m_weightIndicator;
      std::string weightNameStr(weightName.Data());

      TH2* myWeightHisto = (TH2*) ifile->Get((folder+weightNameStr).c_str());

      // get them from the cool HistSvc
      if (!myWeightHisto)
      {
          ATH_MSG_ERROR(" Cannot retrieve histogram: " << weightNameStr);
      } else {
          ATH_MSG_VERBOSE(" Retrieved histo: " << weightNameStr << " for network : " << folder << 
                          " the first bin content of the weight 2d histo is: " << myWeightHisto->GetBinContent(1,1));
      }
      // threshold histograms to be retrieved
      TString thresholdName(m_layerPrefix);
      thresholdName+=i;
      thresholdName+=m_thresholdIndicator;
      std::string thresholdNameStr(thresholdName.Data());

      TH1* myThresholdHisto = (TH1*) ifile->Get((folder+thresholdNameStr).c_str());

      if (!myThresholdHisto)
      {
          ATH_MSG_ERROR(" Cannot retrieve histogram: " << thresholdNameStr);
      } else {
          ATH_MSG_VERBOSE(" Retrieved histo: " << thresholdNameStr << " for network : " << folder );
      }
      // 
      retrievedHistos.push_back(myWeightHisto);
      retrievedHistos.push_back(myThresholdHisto);

    }//end iterating on hidden layers
    
  
     
    TTrainedNetwork* NN=m_networkToHistoTool->fromHistoToTrainedNetwork(retrievedHistos);

   
    ATH_MSG_VERBOSE(folder << " " <<  NN->getnInput());

    if (NN==0)
    {
      ATH_MSG_ERROR(" TTrained Network results in 0 pointer! No network loaded...");
    }
    return NN;
  }


  StatusCode NnClusterizationFactory::nnSetup(){
    
    // Open input file
    std::unique_ptr< TFile > ifile( TFile::Open( m_configTTrainedNetwork.c_str(), "READ" ) );

    // is a memory leak for now : NNs need to be deleted
    clearCache(m_NetworkEstimateImpactPoints);
    clearCache(m_NetworkEstimateImpactPointErrorsX);
    clearCache(m_NetworkEstimateImpactPointErrorsY);
    
    //now read all Histograms
    if (m_loadTTrainedNetworks)
    {
      ATH_MSG_VERBOSE("Loading 10 networks for number estimate, position estimate and error PDF estimate (with track info)");
      m_NetworkEstimateNumberParticles=retrieveNetwork(ifile,"NumberParticles/");
      m_NetworkEstimateImpactPoints.push_back(retrieveNetwork(ifile,"ImpactPoints1P/"));
      m_NetworkEstimateImpactPoints.push_back(retrieveNetwork(ifile,"ImpactPoints2P/"));
      m_NetworkEstimateImpactPoints.push_back(retrieveNetwork(ifile,"ImpactPoints3P/"));
      m_NetworkEstimateImpactPointErrorsX.push_back(retrieveNetwork(ifile,"ImpactPointErrorsX1/"));
      m_NetworkEstimateImpactPointErrorsX.push_back(retrieveNetwork(ifile,"ImpactPointErrorsX2/"));
      m_NetworkEstimateImpactPointErrorsX.push_back(retrieveNetwork(ifile,"ImpactPointErrorsX3/"));
      m_NetworkEstimateImpactPointErrorsY.push_back(retrieveNetwork(ifile,"ImpactPointErrorsY1/"));
      m_NetworkEstimateImpactPointErrorsY.push_back(retrieveNetwork(ifile,"ImpactPointErrorsY2/"));
      m_NetworkEstimateImpactPointErrorsY.push_back(retrieveNetwork(ifile,"ImpactPointErrorsY3/"));
    }

    ifile->Close();

    return StatusCode::SUCCESS;        
  }
  
  void  NnClusterizationFactory::clearCache(std::vector<TTrainedNetwork*>& ttnn){
      for (size_t ittnn = 0; ittnn < ttnn.size(); delete ttnn[ittnn], ++ittnn);
      ttnn.clear();
  }
  

}//end InDet namespace
 
