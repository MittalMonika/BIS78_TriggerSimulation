/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVKalVrtFitter/VKalVrtAtlas.h"
#include "TrkVKalVrtCore/TrkVKalVrtCore.h"
//-------------------------------------------------
//
#include<iostream>

namespace Trk {
 extern   
  void cfimp(long int TrkID, long int  ICH, int IFL, double* PAR, double* ERR,
              double* VRT, double* VCOV,
	      double* RIMP, double* RCOV, double*  SIGN, const VKalVrtControlBase * FitCONTROL );

}
//
//__________________________________________________________________________
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


namespace Trk{

  double TrkVKalVrtFitter::VKalGetImpact(const Trk::Track* InpTrk,const Amg::Vector3D& Vertex, const long int Charge,
                                  std::vector<double>& Impact, std::vector<double>& ImpactError){
//
//------ Variables and arrays needed for fitting kernel
//
    double RIMP[5],RCOV[3];
    double SIGNIF=0.;
    std::vector<const Trk::Track*> InpTrkList;
    InpTrkList.push_back(InpTrk);
//
//--Preparation
    Impact.clear(); ImpactError.clear(); 
//

    if(!m_isFieldInitialized)setInitializedField();  //to allow callback for init
//
//------  extract information about selected tracks
//
    int ntrk=0; 
    StatusCode sc = CvtTrkTrack(InpTrkList,ntrk);
    if(sc.isFailure() || ntrk != 1) {    //Something is wrong in conversion
        std::vector<double> tmpImpact(5,1.e10);
        std::vector<double> tmpImpactError(3,1.e20);
        Impact.swap(tmpImpact);
        ImpactError.swap(tmpImpactError);
        return 1.e10; 
     }
    long int vkCharge=m_ich[0];
    if(Charge==0)vkCharge=0;
//
// Target vertex in ref.frame defined by track themself
//
    double VrtInp[3]={Vertex.x() -m_refFrameX, Vertex.y() -m_refFrameY, Vertex.z() -m_refFrameZ}; 
    double VrtCov[6]={0.,0.,0.,0.,0.,0.};
//
//
    Trk::cfimp( 0, vkCharge, 0, &m_apar[0][0], &m_awgt[0][0], &VrtInp[0], &VrtCov[0], &RIMP[0], &RCOV[0], &SIGNIF, m_vkalFitControl);

    Impact.push_back(RIMP[0]);
    Impact.push_back(RIMP[1]);
    Impact.push_back(RIMP[2]);
    Impact.push_back(RIMP[3]);
    Impact.push_back(RIMP[4]);
    ImpactError.push_back(RCOV[0]);
    ImpactError.push_back(RCOV[1]);
    ImpactError.push_back(RCOV[2]);
    return SIGNIF;

  }





  double TrkVKalVrtFitter::VKalGetImpact(const TrackParticleBase* InpTrk,const Amg::Vector3D& Vertex,const long int Charge,
                                  std::vector<double>& Impact, std::vector<double>& ImpactError){
//
//------ Variables and arrays needed for fitting kernel
//
    double RIMP[5],RCOV[3];
    double SIGNIF=0.;
    std::vector<const TrackParticleBase*> InpTrkList;
    InpTrkList.push_back(InpTrk);
//
//--Preparation
    Impact.clear(); ImpactError.clear();
//

    if(!m_isFieldInitialized)setInitializedField();  //to allow callback for init
//
//------  extract information about selected tracks
//
    int ntrk=0; 
    StatusCode sc = CvtTrackParticle(InpTrkList,ntrk);
    if(sc.isFailure() || ntrk != 1) {    //Something is wrong in conversion
        std::vector<double> tmpImpact(5,1.e10);
        std::vector<double> tmpImpactError(3,1.e20);
        Impact.swap(tmpImpact);
        ImpactError.swap(tmpImpactError);
        return 1.e10; 
     }
    long int vkCharge=m_ich[0];
    if(Charge==0)vkCharge=0;
//
// Target vertex in ref.frame defined by track itself
//
    double VrtInp[3]={Vertex.x() -m_refFrameX, Vertex.y() -m_refFrameY, Vertex.z() -m_refFrameZ}; 
    double VrtCov[6]={0.,0.,0.,0.,0.,0.};
//
//
    Trk::cfimp( 0, vkCharge, 0, &m_apar[0][0], &m_awgt[0][0], &VrtInp[0], &VrtCov[0], &RIMP[0], &RCOV[0], &SIGNIF, m_vkalFitControl);

    Impact.push_back(RIMP[0]);
    Impact.push_back(RIMP[1]);
    Impact.push_back(RIMP[2]);
    Impact.push_back(RIMP[3]);
    Impact.push_back(RIMP[4]);
    ImpactError.push_back(RCOV[0]);
    ImpactError.push_back(RCOV[1]);
    ImpactError.push_back(RCOV[2]);
    return SIGNIF;

  }

  double TrkVKalVrtFitter::VKalGetImpact(const xAOD::TrackParticle* InpTrk,const Amg::Vector3D& Vertex,const long int Charge,
                                  std::vector<double>& Impact, std::vector<double>& ImpactError){
//
//------ Variables and arrays needed for fitting kernel
//
    double RIMP[5],RCOV[3];
    double SIGNIF=0.;

    std::vector<const xAOD::TrackParticle*> InpTrkList(1,InpTrk);
//
//--Preparation
    Impact.clear(); ImpactError.clear();
//


    if(!m_isFieldInitialized)setInitializedField();  //to allow callback for init
//
//------  extract information about selected tracks
//
    int ntrk=0; 
    StatusCode sc = CvtTrackParticle(InpTrkList,ntrk);
    if(sc.isFailure() ||  ntrk != 1   )  {       //Something is wrong in conversion
        std::vector<double> tmpImpact(5,1.e10);
        std::vector<double> tmpImpactError(3,1.e20);
        Impact.swap(tmpImpact);
        ImpactError.swap(tmpImpactError);
        return 1.e10;
    }
    long int vkCharge=m_ich[0];
    if(Charge==0)vkCharge=0;
//
// Target vertex in ref.frame defined by track itself
//
    double VrtInp[3]={Vertex.x() -m_refFrameX, Vertex.y() -m_refFrameY, Vertex.z() -m_refFrameZ}; 
    double VrtCov[6]={0.,0.,0.,0.,0.,0.};
//
//
    Trk::cfimp( 0, vkCharge, 0, &m_apar[0][0], &m_awgt[0][0], &VrtInp[0], &VrtCov[0], &RIMP[0], &RCOV[0], &SIGNIF, m_vkalFitControl);

    Impact.push_back(RIMP[0]);
    Impact.push_back(RIMP[1]);
    Impact.push_back(RIMP[2]);
    Impact.push_back(RIMP[3]);
    Impact.push_back(RIMP[4]);
    ImpactError.push_back(RCOV[0]);
    ImpactError.push_back(RCOV[1]);
    ImpactError.push_back(RCOV[2]);
    return SIGNIF;

  }


}

