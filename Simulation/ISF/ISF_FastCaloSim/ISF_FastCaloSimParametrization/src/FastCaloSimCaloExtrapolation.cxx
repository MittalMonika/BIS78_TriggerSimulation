/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "FastCaloSimCaloExtrapolation.h"

#include "ISF_FastCaloSimEvent/TFCSTruthState.h"


#include "TrkParameters/TrackParameters.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "HepPDT/ParticleData.hh"
#include "HepPDT/ParticleDataTable.hh"

#include "CaloTrackingGeometry/ICaloSurfaceHelper.h"
#include "CaloTrackingGeometry/ICaloSurfaceBuilder.h"
#include "CaloDetDescr/ICaloCoordinateTool.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "GaudiKernel/ListItem.h"

FastCaloSimCaloExtrapolation::FastCaloSimCaloExtrapolation(const std::string& t, const std::string& n, const IInterface* p) :
  AthAlgTool(t,n,p),
  //m_CaloBoundaryR(1148.0),
  //m_CaloBoundaryZ(3549.5),
  //m_calomargin(100),
  //m_extrapolator("TimedExtrapolator"),
  //m_caloEntranceName(""),
  //m_caloSurfaceHelper("CaloSurfaceHelper"),
  //m_CaloGeometryHelper("FastCaloSimGeometryHelper"),
  m_caloEntrance(nullptr)
{
  declareInterface<IFastCaloSimCaloExtrapolation>(this);

  m_surfacelist.resize(0);
  m_surfacelist.push_back(CaloCell_ID_FCS::PreSamplerB);
  m_surfacelist.push_back(CaloCell_ID_FCS::PreSamplerE);
  m_surfacelist.push_back(CaloCell_ID_FCS::EME1);
  m_surfacelist.push_back(CaloCell_ID_FCS::EME2);
  m_surfacelist.push_back(CaloCell_ID_FCS::FCAL0);

  declareProperty("CaloBoundaryR",                  m_CaloBoundaryR);
  declareProperty("CaloBoundaryZ",                  m_CaloBoundaryZ);
  declareProperty("CaloMargin",                     m_calomargin);
  declareProperty("Surfacelist",                    m_surfacelist );
  declareProperty("Extrapolator",                   m_extrapolator );
  declareProperty("CaloEntrance",                   m_caloEntranceName );
  declareProperty("CaloSurfaceHelper",              m_caloSurfaceHelper );
  declareProperty("CaloGeometryHelper",             m_CaloGeometryHelper );
}

FastCaloSimCaloExtrapolation::~FastCaloSimCaloExtrapolation()
{
}

StatusCode FastCaloSimCaloExtrapolation::initialize()
{
  ATH_MSG_INFO( "Initializing FastCaloSimCaloExtrapolation" );

  // Get CaloGeometryHelper
  ATH_CHECK(m_CaloGeometryHelper.retrieve());

  // Get PDG table
  IPartPropSvc* p_PartPropSvc=nullptr;

  ATH_CHECK(service("PartPropSvc",p_PartPropSvc));
  if(!p_PartPropSvc)
    {
      ATH_MSG_ERROR("could not find PartPropService");
      return StatusCode::FAILURE;
    }

  m_particleDataTable = (HepPDT::ParticleDataTable*) p_PartPropSvc->PDT();
  if(!m_particleDataTable)
    {
      ATH_MSG_ERROR("PDG table not found");
      return StatusCode::FAILURE;
    }
  //#########################

  // Get TimedExtrapolator
  if(!m_extrapolator.empty())
    {
      ATH_CHECK(m_extrapolator.retrieve());
      ATH_MSG_INFO("Extrapolator retrieved "<< m_extrapolator);
    }

  // Get CaloSurfaceHelper
  ATH_CHECK(m_caloSurfaceHelper.retrieve());

  ATH_MSG_INFO("m_CaloBoundaryR "<<m_CaloBoundaryR<<" m_caloEntranceName "<<m_caloEntranceName);

  return StatusCode::SUCCESS;

}

StatusCode FastCaloSimCaloExtrapolation::finalize()
{
  ATH_MSG_INFO( "Finalizing FastCaloSimCaloExtrapolation" );
  return StatusCode::SUCCESS;
}


void FastCaloSimCaloExtrapolation::extrapolate(TFCSExtrapolationState& result,const TFCSTruthState* truth)
{

  //UPDATE EXTRAPOLATION
  ATH_MSG_INFO("Start FastCaloSimCaloExtrapolation::extrapolate");
  std::vector<Trk::HitInfo>* hitVector = caloHits(truth);
  ATH_MSG_INFO("Done FastCaloSimCaloExtrapolation::extrapolate: caloHits");

  //////////////////////////////////////
  // Start calo extrapolation
  // First: get entry point into first calo sample
  //////////////////////////////////////
  ATH_MSG_INFO("FastCaloSimCaloExtrapolation::extrapolate:*** Get calo surface ***");
  get_calo_surface(result,hitVector);

  ATH_MSG_INFO("FastCaloSimCaloExtrapolation::extrapolate:*** Do extrapolation to ID-calo boundary ***");
  extrapolate_to_ID(result,truth,hitVector);

  ATH_MSG_INFO("FastCaloSimCaloExtrapolation::extrapolate:*** Do extrapolation ***");
  extrapolate(result,truth,hitVector);

  ATH_MSG_INFO("FastCaloSimCaloExtrapolation::extrapolate: Truth extrapolation done");

  for(std::vector<Trk::HitInfo>::iterator it = hitVector->begin();it < hitVector->end();++it)
    {
      if((*it).trackParms)
        {
          delete (*it).trackParms;
          (*it).trackParms=0;
        }
    }
  delete hitVector;
  ATH_MSG_INFO("Done FastCaloSimCaloExtrapolation::extrapolate");
}

std::vector<Trk::HitInfo>* FastCaloSimCaloExtrapolation::caloHits(const TFCSTruthState* truth) const
{
  // Start calo extrapolation
  ATH_MSG_INFO ("[ fastCaloSim transport ] processing particle "<<truth->pdgid() );

  std::vector<Trk::HitInfo>* hitVector =  new std::vector<Trk::HitInfo>;

  int     pdgId    = truth->pdgid();
  double  charge   = HepPDT::ParticleID(pdgId).charge();

  // particle Hypothesis for the extrapolation

  Trk::ParticleHypothesis pHypothesis = m_pdgToParticleHypothesis.convert(pdgId,charge);

  ATH_MSG_INFO ("particle hypothesis "<< pHypothesis );

  // geantinos not handled by PdgToParticleHypothesis - fix there
  if ( pdgId == 999 ) pHypothesis = Trk::geantino;

  Amg::Vector3D pos = Amg::Vector3D( truth->vertex().X(), truth->vertex().Y(), truth->vertex().Z());

  Amg::Vector3D mom(truth->X(),truth->Y(),truth->Z());

  ATH_MSG_INFO( "[ fastCaloSim transport ] x from position eta="<<pos.eta()<<" phi="<<pos.phi()<<" d="<<pos.mag()<<" pT="<<mom.perp() );

  // input parameters : curvilinear parameters
  Trk::CurvilinearParameters inputPar(pos,mom,charge);

  // stable vs. unstable check : ADAPT for FASTCALOSIM
  //double freepath = ( !m_particleDecayHelper.empty()) ? m_particleDecayHelper->freePath(isp) : - 1.;
  double freepath = -1.;
  //ATH_MSG_VERBOSE( "[ fatras transport ] Particle free path : " << freepath);
  // path limit -> time limit  ( TODO : extract life-time directly from decay helper )
  double tDec = freepath > 0. ? freepath : -1.;
  int decayProc = 0;

  /* uncomment if unstable particles used by FastCaloSim
  // beta calculated here for further use in validation
  double mass = m_particleMasses.mass[pHypothesis];
  double mom = isp.momentum().mag();
  double beta = mom/sqrt(mom*mom+mass*mass);

  if ( tDec>0.)
  {
  tDec = tDec/beta/CLHEP::c_light + isp.timeStamp();
  decayProc = 201;
  }
  */

  Trk::TimeLimit timeLim(tDec,0.,decayProc);        // TODO: set vertex time info

  // prompt decay ( uncomment if unstable particles used )
  //if ( freepath>0. && freepath<0.01 ) {
  //  if (!m_particleDecayHelper.empty()) {
  //    ATH_MSG_VERBOSE( "[ fatras transport ] Decay is triggered for input particle.");
  //    m_particleDecayHelper->decay(isp);
  //  }
  //  return 0;
  //}

  // presample interactions - ADAPT FOR FASTCALOSIM
  Trk::PathLimit pathLim(-1.,0);
  //if (absPdg!=999 && pHypothesis<99) pathLim = m_samplingTool->sampleProcess(mom,isp.charge(),pHypothesis);

  Trk::GeometrySignature nextGeoID=Trk::Calo;

  // first extrapolation to reach the ID boundary

  ATH_MSG_INFO( "[ fastCaloSim transport ] before calo entrance ");

  // get CaloEntrance if not done already
  if(!m_caloEntrance)
    {
      m_caloEntrance = m_extrapolator->trackingGeometry()->trackingVolume(m_caloEntranceName);

      if(!m_caloEntrance)
        ATH_MSG_WARNING("CaloEntrance not found");
      else
        ATH_MSG_INFO("CaloEntrance found");
    }

  ATH_MSG_INFO( "[ fastCaloSim transport ] after calo entrance ");

  const Trk::TrackParameters* caloEntry = 0;

  if(m_caloEntrance && m_caloEntrance->inside(pos,0.001) && !m_extrapolator->trackingGeometry()->atVolumeBoundary(pos,m_caloEntrance,0.001))
    {
      std::vector<Trk::HitInfo>* dummyHitVector = 0;
      if( charge==0 )
        {
          caloEntry = m_extrapolator->transportNeutralsWithPathLimit(inputPar,pathLim,timeLim,
                                                                     Trk::alongMomentum,pHypothesis,dummyHitVector,nextGeoID,m_caloEntrance);

        }
      else
        {
          caloEntry = m_extrapolator->extrapolateWithPathLimit(inputPar,pathLim,timeLim,
                                                               Trk::alongMomentum,pHypothesis,dummyHitVector,nextGeoID,m_caloEntrance);
        }
    }
  else
    caloEntry=&inputPar;

  ATH_MSG_INFO( "[ fastCaloSim transport ] after calo caloEntry ");

  if(caloEntry)
    {
      const Trk::TrackParameters* eParameters = 0;

      // save Calo entry hit (fallback info)
      hitVector->push_back(Trk::HitInfo(caloEntry->clone(),timeLim.time,nextGeoID,0.));

      ATH_MSG_DEBUG( "[ fastCaloSim transport ] starting Calo transport from position eta="<<caloEntry->position().eta()<<" phi="<<caloEntry->position().phi()<<" d="<<caloEntry->position().mag() );

      if(charge==0)
        {
          eParameters = m_extrapolator->transportNeutralsWithPathLimit(*caloEntry,pathLim,timeLim,
                                                                       Trk::alongMomentum,pHypothesis,hitVector,nextGeoID);
        }
      else
        {
          eParameters = m_extrapolator->extrapolateWithPathLimit(*caloEntry,pathLim,timeLim,
                                                                 Trk::alongMomentum,pHypothesis,hitVector,nextGeoID);
        }

      // save Calo exit hit (fallback info)
      if(eParameters) hitVector->push_back(Trk::HitInfo(eParameters,timeLim.time,nextGeoID,0.));
      //delete eParameters;   // HitInfo took ownership
    } //if caloEntry

  if(msgLvl(MSG::DEBUG))
    {
      std::vector<Trk::HitInfo>::iterator it = hitVector->begin();
      while (it < hitVector->end() )
        {
          int sample=(*it).detID;
          Amg::Vector3D hitPos = (*it).trackParms->position();
          ATH_MSG_DEBUG(" HIT: layer="<<sample<<" sample="<<sample-3000<<" eta="<<hitPos.eta()<<" phi="<<hitPos.phi()<<" d="<<hitPos.mag());
          it++;
        }
    }

  ATH_MSG_INFO("CHECK BEFORE");
  std::vector<Trk::HitInfo>::iterator it = hitVector->begin();
  while(it < hitVector->end() )
    {
      int sample=(*it).detID;
      Amg::Vector3D hitPos = (*it).trackParms->position();
      ATH_MSG_INFO(" HIT: layer="<<sample<<" sample="<<sample-3000<<" eta="<<hitPos.eta()<<" phi="<<hitPos.phi()<<" d="<<hitPos.mag());
      it++;
    }

  int again=1;
  while(again)
    {
      again=0;
      std::vector<Trk::HitInfo>::iterator it = hitVector->begin();
      int ele=0;
      while(it < hitVector->end())
        {
          int sample=(*it).detID;
          //Amg::Vector3D hitPos = (*it).trackParms->position();
          //std::cout<<" HIT: layer="<<sample<<" sample="<<sample-3000<<" eta="<<hitPos.eta()<<" phi="<<hitPos.phi()<<" d="<<hitPos.mag()<<std::endl;
          //delete the entry from the hit vector if sample is<3000:
          if(sample<3000)
            {
              //std::cout<<"deleting element "<<ele<<std::endl;
              hitVector->erase(it);
              again=1;
              it=hitVector->end();
            }
          ele++;
          it++;
        }
    }

  ATH_MSG_INFO("CHECK AFTER");
  std::vector<Trk::HitInfo>::iterator it2 = hitVector->begin();
  while(it2 < hitVector->end() )
    {
      int sample=(*it2).detID;
      Amg::Vector3D hitPos = (*it2).trackParms->position();
      ATH_MSG_INFO(" HIT: layer="<<sample<<" sample="<<sample-3000<<" eta="<<hitPos.eta()<<" phi="<<hitPos.phi()<<" d="<<hitPos.mag());
      it2++;
    }


  return hitVector;
}

//#######################################################################
void FastCaloSimCaloExtrapolation::extrapolate(TFCSExtrapolationState& result,const TFCSTruthState* truth,std::vector<Trk::HitInfo>* hitVector)
{
  ATH_MSG_DEBUG("Start extrapolate()");

  double ptruth_eta=truth->Eta();
  double ptruth_phi=truth->Phi();
  double ptruth_pt =truth->Pt();
  double ptruth_p  =truth->P();
  int    pdgid     =truth->pdgid();

  //////////////////////////////////////
  // Start calo extrapolation
  //////////////////////////////////////

  std::vector< std::vector<double> > eta_safe(3);
  std::vector< std::vector<double> > phi_safe(3);
  std::vector< std::vector<double> > r_safe(3);
  std::vector< std::vector<double> > z_safe(3);
  for(int subpos=SUBPOS_MID;subpos<=SUBPOS_EXT;++subpos)
    {
      eta_safe[subpos].resize(CaloCell_ID_FCS::MaxSample,-999.0);
      phi_safe[subpos].resize(CaloCell_ID_FCS::MaxSample,-999.0);
      r_safe[subpos].resize(CaloCell_ID_FCS::MaxSample,-999.0);
      z_safe[subpos].resize(CaloCell_ID_FCS::MaxSample,-999.0);
    }

  // only continue if inside the calo
  if( fabs(result.CaloSurface_eta())<6 )
    {
      // now try to extrpolate to all calo layers, that contain energy
      ATH_MSG_DEBUG("Calo position for particle id "<<pdgid<<", trutheta= " << ptruth_eta <<", truthphi= "<<ptruth_phi<<", truthp="<<ptruth_p<<", truthpt="<<ptruth_pt);
      for(int sample=CaloCell_ID_FCS::FirstSample;sample<CaloCell_ID_FCS::MaxSample;++sample)
        {
          for(int subpos=SUBPOS_MID;subpos<=SUBPOS_EXT;++subpos)
            {
              if(get_calo_etaphi(result,hitVector,sample,subpos))
                ATH_MSG_DEBUG( "Result in sample "<<sample<<"."<<subpos<<": eta="<<result.eta(sample,subpos)<<" phi="<<result.phi(sample,subpos)<<" r="<<result.r(sample,subpos)<<" z="<<result.z(sample,subpos)<<" (ok="<<result.OK(sample,subpos)<<")");
              else
                ATH_MSG_DEBUG( "Extrapolation to sample "<<sample<<" failed (ok="<<result.OK(sample,subpos)<<")");
            } //for pos
        } //for sample
    } //inside calo
  else
    ATH_MSG_WARNING( "Ups. Not inside calo. result.CaloSurface_eta()="<<result.CaloSurface_eta());

  ATH_MSG_DEBUG("End extrapolate()");
}

void FastCaloSimCaloExtrapolation::extrapolate_to_ID(TFCSExtrapolationState& result,const TFCSTruthState* /*truth*/,std::vector<Trk::HitInfo>* hitVector)
{
  ATH_MSG_DEBUG("Start extrapolate_to_ID()");

  Amg::Vector3D hitpos(0,0,0);
  Amg::Vector3D hitmom(0,0,0);
  if(rz_cylinder_get_calo_etaphi(hitVector,m_CaloBoundaryR,m_CaloBoundaryZ,hitpos,hitmom))
    {
      ATH_MSG_DEBUG("BOUNDARY ID-CALO eta="<<hitpos.eta()<<" phi="<<hitpos.phi()<<" r="<<hitpos.perp()<<" z="<<hitpos[Amg::z]<<" theta="<<hitpos.theta()<<" ; momentum eta="<<hitmom.eta()<<" phi="<<hitmom.phi()<<" theta="<<hitmom.theta());
      result.set_IDCaloBoundary_eta(hitpos.eta());
      result.set_IDCaloBoundary_phi(hitpos.phi());
      result.set_IDCaloBoundary_r(hitpos.perp());
      result.set_IDCaloBoundary_z(hitpos[Amg::z]);
    }
  else
    {
      ATH_MSG_DEBUG("Extrapolation to IDCaloBoundary failed");
      result.set_IDCaloBoundary_eta(-999.);
      result.set_IDCaloBoundary_phi(-999.);
      result.set_IDCaloBoundary_r(0);
      result.set_IDCaloBoundary_z(0);
    }

  TVector3 vec(hitpos[Amg::x],hitpos[Amg::y],hitpos[Amg::z]);

  //get the tangentvector on this interaction point:
  //GlobalMomentum* mom=params_on_surface_ID->TrackParameters::momentum().unit() ;
  //Trk::GlobalMomentum* trackmom=params_on_surface_ID->Trk::TrackParameters::momentum();
  if(hitmom.mag()>0)
    {
      //angle between vec and trackmom:
      TVector3 Trackmom(hitmom[Amg::x],hitmom[Amg::y],hitmom[Amg::z]);
      double angle3D=Trackmom.Angle(vec); //isn't this the same as TVector3 vec?
      ATH_MSG_DEBUG("    3D ANGLE "<<angle3D);

      double angleEta=vec.Theta()-Trackmom.Theta();
      ATH_MSG_DEBUG("    ANGLE dTHEA"<<angleEta);

      result.set_IDCaloBoundary_AngleEta(angleEta);
      result.set_IDCaloBoundary_Angle3D(angle3D);
    }
  else
    {
      result.set_IDCaloBoundary_AngleEta(-999.);
      result.set_IDCaloBoundary_Angle3D(-999.);
    }

  ATH_MSG_DEBUG("End extrapolate_to_ID()");

} //extrapolate_to_ID

bool FastCaloSimCaloExtrapolation::get_calo_surface(TFCSExtrapolationState& result,std::vector<Trk::HitInfo>* hitVector)
{
  ATH_MSG_DEBUG("Start get_calo_surface()");

  result.set_CaloSurface_sample(CaloCell_ID_FCS::noSample);
  result.set_CaloSurface_eta(-999);
  result.set_CaloSurface_phi(-999);
  result.set_CaloSurface_r(0);
  result.set_CaloSurface_z(0);
  double min_calo_surf_dist=1000;

  for(unsigned int i=0;i<m_surfacelist.size();++i)
    {

      int sample=m_surfacelist[i];
      std::vector<Trk::HitInfo>::iterator it = hitVector->begin();

      while (it != hitVector->end() && it->detID != (3000+sample) )
        it++;

      if(it==hitVector->end()) continue;

      Amg::Vector3D hitPos = (*it).trackParms->position();

      //double offset = 0.;
      double etaCalo = hitPos.eta();

      if(fabs(etaCalo)<900)
        {
          double phiCalo = hitPos.phi();
          double distsamp =deta(sample,etaCalo);

          if(distsamp<min_calo_surf_dist && min_calo_surf_dist>=0)
            {
              //hitVector is ordered in r, so if first surface was hit, keep it
              result.set_CaloSurface_sample(sample);
              result.set_CaloSurface_eta(etaCalo);
              result.set_CaloSurface_phi(phiCalo);
              double rcalo=rent(sample,etaCalo);
              double zcalo=zent(sample,etaCalo);
              result.set_CaloSurface_r(rcalo);
              result.set_CaloSurface_z(zcalo);
              min_calo_surf_dist=distsamp;
              msg(MSG::DEBUG)<<" r="<<rcalo<<" z="<<zcalo;

              if(distsamp<0)
                {
                  msg(MSG::DEBUG)<<endmsg;
                  break;
                }
            }
          msg(MSG::DEBUG)<<endmsg;
        }
      else
        msg(MSG::DEBUG)<<": eta > 900, not using this"<<endmsg;
    }

  if(result.CaloSurface_sample()==CaloCell_ID_FCS::noSample)
    {
      // first intersection with sensitive calo layer
      std::vector<Trk::HitInfo>::iterator it = hitVector->begin();

      while( it < hitVector->end() && (*it).detID != 3 )
        it++;   // to be updated

      if (it==hitVector->end())
        return false;  // no calo intersection, abort

      Amg::Vector3D surface_hitPos = (*it).trackParms->position();

      result.set_CaloSurface_eta(surface_hitPos.eta());
      result.set_CaloSurface_phi(surface_hitPos.phi());
      result.set_CaloSurface_r(surface_hitPos.perp());
      result.set_CaloSurface_z(surface_hitPos[Amg::z]);

      double pT=(*it).trackParms->momentum().perp();
      if(TMath::Abs(result.CaloSurface_eta())>4.9 || pT<500 || (TMath::Abs(result.CaloSurface_eta())>4 && pT<1000) )
        ATH_MSG_DEBUG("only entrance to calo entrance layer found, no surface : eta="<<result.CaloSurface_eta()<<" phi="<<result.CaloSurface_phi()<<" r="<<result.CaloSurface_r()<<" z="<<result.CaloSurface_z()<<" pT="<<pT);
      else
        ATH_MSG_WARNING("only entrance to calo entrance layer found, no surface : eta="<<result.CaloSurface_eta()<<" phi="<<result.CaloSurface_phi()<<" r="<<result.CaloSurface_r()<<" z="<<result.CaloSurface_z()<<" pT="<<pT);
    } //sample
  else
    {
      ATH_MSG_DEBUG("entrance to calo surface : sample="<<result.CaloSurface_sample()<<" eta="<<result.CaloSurface_eta()<<" phi="<<result.CaloSurface_phi()<<" r="<<result.CaloSurface_r()<<" z="<<result.CaloSurface_z()<<" deta="<<min_calo_surf_dist);
    }

  ATH_MSG_DEBUG("End get_calo_surface()");
  return true;
}

//UPDATED
bool FastCaloSimCaloExtrapolation::get_calo_etaphi(TFCSExtrapolationState& result,std::vector<Trk::HitInfo>* hitVector, int sample,int subpos)
{

  result.set_OK(sample,subpos,false);
  result.set_eta(sample,subpos,result.CaloSurface_eta());
  result.set_phi(sample,subpos,result.CaloSurface_phi());
  result.set_r(sample,subpos,rpos(sample,result.CaloSurface_eta(),subpos));
  result.set_z(sample,subpos,zpos(sample,result.CaloSurface_eta(),subpos));

  double distsamp =deta(sample,result.CaloSurface_eta());
  double lrzpos =rzpos(sample,result.CaloSurface_eta(),subpos);
  double hitdist=0;
  bool best_found=false;
  double best_target=0;

  std::vector<Trk::HitInfo>::iterator it = hitVector->begin();

  while( it!= hitVector->end() && it->detID != (3000+sample) )
    it++;

  //while ((*it).detID != (3000+sample) && it < hitVector->end() )  it++;

  if(it!=hitVector->end())
    {

      Amg::Vector3D hitPos1 = (*it).trackParms->position();
      int sid1=(*it).detID;
      int sid2=-1;
      Amg::Vector3D hitPos;
      Amg::Vector3D hitPos2;

      std::vector<Trk::HitInfo>::iterator itnext = it;
      ++itnext;
      if(itnext!=hitVector->end())
        {
          hitPos2 = (*itnext).trackParms->position();
          sid2=(*itnext).detID;
          double eta_avg=0.5*(hitPos1.eta()+hitPos2.eta());
          double t;

          if(isCaloBarrel(sample))
            {
              double r=rpos(sample,eta_avg,subpos);
              double r1=hitPos1.perp();
              double r2=hitPos2.perp();
              t=(r-r1)/(r2-r1);
              best_target=r;
            }
          else
            {
              double z=zpos(sample,eta_avg,subpos);
              double z1=hitPos1[Amg::z];
              double z2=hitPos2[Amg::z];
              t=(z-z1)/(z2-z1);
              best_target=z;
            }
          hitPos=t*hitPos2+(1-t)*hitPos1;

        }
      else
        {
          hitPos=hitPos1;
          hitPos2=hitPos1;
        }

      double etaCalo = hitPos.eta();
      double phiCalo = hitPos.phi();
      result.set_OK(sample,subpos,true);
      result.set_eta(sample,subpos,etaCalo);
      result.set_phi(sample,subpos,phiCalo);
      result.set_r(sample,subpos,hitPos.perp());
      result.set_z(sample,subpos,hitPos[Amg::z]);
      hitdist=hitPos.mag();
      lrzpos=rzpos(sample,etaCalo,subpos);
      distsamp=deta(sample,etaCalo);
      best_found=true;

      ATH_MSG_DEBUG("extrapol with layer hit: id="<<sid1<<" -> "<<sid2<<" target r/z="<<best_target<<" r1="<<hitPos1.perp()<<" z1="<<hitPos1[Amg::z]<<
                    " r2="<<hitPos2.perp()<<" z2="<<hitPos2[Amg::z]<<" result.r="<<result.r(sample,subpos)<<" result.z="<<result.z(sample,subpos));

    }

  if(!best_found)
    {
      it = hitVector->begin();
      double best_dist=0.5;
      bool best_inside=false;
      int best_id1,best_id2;
      Amg::Vector3D best_hitPos=(*it).trackParms->position();
      while (it < hitVector->end()-1 )
        {
          Amg::Vector3D hitPos1 = (*it).trackParms->position();
          int sid1=(*it).detID;
          it++;
          Amg::Vector3D hitPos2 = (*it).trackParms->position();
          int sid2=(*it).detID;
          double eta_avg=0.5*(hitPos1.eta()+hitPos2.eta());
          double t;
          double tmp_target=0;
          if(isCaloBarrel(sample))
            {
              double r=rpos(sample,eta_avg,subpos);
              double r1=hitPos1.perp();
              double r2=hitPos2.perp();
              t=(r-r1)/(r2-r1);
              tmp_target=r;
            }
          else
            {
              double z=zpos(sample,eta_avg,subpos);
              double z1=hitPos1[Amg::z];
              double z2=hitPos2[Amg::z];
              t=(z-z1)/(z2-z1);
              tmp_target=z;
            }
          Amg::Vector3D hitPos=t*hitPos2+(1-t)*hitPos1;
          double dist=TMath::Min(TMath::Abs(t-0),TMath::Abs(t-1));
          bool inside=false;
          if(t>=0 && t<=1) inside=true;
          if(!best_found || inside)
            {
              if(!best_inside || dist<best_dist)
                {
                  best_dist=dist;
                  best_hitPos=hitPos;
                  best_inside=inside;
                  best_found=true;
                  best_id1=sid1;
                  best_id2=sid2;
                  best_target=tmp_target;
                }
            }
          else
            {
              if(!best_inside && dist<best_dist)
                {
                  best_dist=dist;
                  best_hitPos=hitPos;
                  best_inside=inside;
                  best_found=true;
                  best_id1=sid1;
                  best_id2=sid2;
                  best_target=tmp_target;
                }
            }
          ATH_MSG_DEBUG("extrapol without layer hit: id="<<sid1<<" -> "<<sid2<<" dist="<<dist<<" mindist="<<best_dist<<
                        " t="<<t<<" best_inside="<<best_inside<<" target r/z="<<tmp_target<<
                        " r1="<<hitPos1.perp()<<" z1="<<hitPos1[Amg::z]<<" r2="<<hitPos2.perp()<<" z2="<<hitPos2[Amg::z]<<
                        " re="<<hitPos.perp()<<" ze="<<hitPos[Amg::z]<<
                        " rb="<<best_hitPos.perp()<<" zb="<<best_hitPos[Amg::z]);
          if(best_found)
            {
              double etaCalo = best_hitPos.eta();
              result.set_OK(sample,subpos,true);
              result.set_eta(sample,subpos,etaCalo);
              result.set_phi(sample,subpos,best_hitPos.phi());
              result.set_r(sample,subpos,best_hitPos.perp());
              result.set_z(sample,subpos,best_hitPos[Amg::z]);
              hitdist=best_hitPos.mag();
              lrzpos=rzpos(sample,etaCalo,subpos);
              distsamp=deta(sample,etaCalo);
            }
        } //while hit vector

      if(best_found)
        {
          ATH_MSG_DEBUG("extrapol without layer hit: id="<<best_id1<<" -> "<<best_id2<<" mindist="<<best_dist<<
                        " best_inside="<<best_inside<<" target r/z="<<best_target<<
                        " rb="<<best_hitPos.perp()<<" zb="<<best_hitPos[Amg::z] );
        }
    }

  if(isCaloBarrel(sample))
    lrzpos*=cosh(result.eta(sample,subpos));
  else
    lrzpos= fabs(lrzpos/tanh(result.eta(sample,subpos)));

  result.set_d(sample,subpos,lrzpos);
  result.set_detaBorder(sample,subpos,distsamp);

  ATH_MSG_DEBUG("Final TTC result for sample "<<sample<<" subpos="<<subpos<<" OK() "<<result.OK(sample,subpos)<<" eta="<<result.eta(sample,subpos)<<" phi="<<result.phi(sample,subpos)<<" dCalo="<<result.d(sample,subpos)<<" dist(hit)="<<hitdist);

  return result.OK(sample,subpos);
}

//UPDATED
bool FastCaloSimCaloExtrapolation::rz_cylinder_get_calo_etaphi(std::vector<Trk::HitInfo>* hitVector, double cylR, double cylZ, Amg::Vector3D& pos, Amg::Vector3D& mom)
{

  bool best_found=false;
  double best_dist=10000;
  bool best_inside=false;
  int best_id1,best_id2;

  std::vector<Trk::HitInfo>::iterator it = hitVector->begin();
  Amg::Vector3D best_hitPos=(*it).trackParms->position();
  for(int rz=0;rz<=1;++rz) {
    it = hitVector->begin();
    while (it < hitVector->end()-1 ) {
      Amg::Vector3D hitPos1 = (*it).trackParms->position();
      Amg::Vector3D hitMom1 = (*it).trackParms->momentum();
      int sid1=(*it).detID;
      it++;
      Amg::Vector3D hitPos2 = (*it).trackParms->position();
      Amg::Vector3D hitMom2 = (*it).trackParms->momentum();
      int sid2=(*it).detID;

      double t;
      if(rz==1) {
        double r=cylR;
        double r1=hitPos1.perp();
        double r2=hitPos2.perp();
        t=(r-r1)/(r2-r1);
      } else {
        double z=cylZ;
        double z1=hitPos1[Amg::z];
        double z2=hitPos2[Amg::z];
        t=(z-z1)/(z2-z1);
      }
      Amg::Vector3D hitPos=t*hitPos2+(1-t)*hitPos1;

      double dist=hitPos.mag();
      bool inside=false;
      if(t>=-0.001 && t<=1.001) inside=true;

      if(!best_found || inside) {
        if(!best_inside || dist<best_dist) {
          best_dist=dist;
          best_hitPos=hitPos;
          best_inside=inside;
          best_found=true;
          best_id1=sid1;
          best_id2=sid2;
          mom=t*hitMom2+(1-t)*hitMom1;
        }
      } else {
        if(!best_inside && dist<best_dist) {
          best_dist=dist;
          best_hitPos=hitPos;
          best_inside=inside;
          best_found=true;
          best_id1=sid1;
          best_id2=sid2;
          mom=t*hitMom2+(1-t)*hitMom1;
        }
      }
      ATH_MSG_DEBUG(" extrapol without layer hit: id="<<sid1<<" -> "<<sid2<<" dist="<<dist<<" bestdist="<<best_dist<<
                    " t="<<t<<" inside="<<inside<<" best_inside="<<best_inside<<
                    " r1="<<hitPos1.perp()<<" z1="<<hitPos1[Amg::z]<<" r2="<<hitPos2.perp()<<" z2="<<hitPos2[Amg::z]<<
                    " re="<<hitPos.perp()<<" ze="<<hitPos[Amg::z]<<
                    " rb="<<best_hitPos.perp()<<" zb="<<best_hitPos[Amg::z]
                    );
    }
  }

  if(best_found) {
    ATH_MSG_DEBUG(" extrapol to r="<<cylR<<" z="<<cylZ<<": id="<<best_id1<<" -> "<<best_id2<<" dist="<<best_dist<<
                  " best_inside="<<best_inside<<
                  " rb="<<best_hitPos.perp()<<" zb="<<best_hitPos[Amg::z]
                  );
  }
  pos=best_hitPos;


  return best_found;
}


bool FastCaloSimCaloExtrapolation::isCaloBarrel(int sample) const
{
  return GetCaloGeometry()->isCaloBarrel(sample);
}

double FastCaloSimCaloExtrapolation::deta(int sample,double eta) const
{
  return GetCaloGeometry()->deta(sample,eta);
}

void FastCaloSimCaloExtrapolation::minmaxeta(int sample,double eta,double& mineta,double& maxeta) const
{
  GetCaloGeometry()->minmaxeta(sample,eta,mineta,maxeta);
}

double FastCaloSimCaloExtrapolation::rmid(int sample,double eta) const
{
  return GetCaloGeometry()->rmid(sample,eta);
}

double FastCaloSimCaloExtrapolation::zmid(int sample,double eta) const
{
  return GetCaloGeometry()->zmid(sample,eta);
}

double FastCaloSimCaloExtrapolation::rzmid(int sample,double eta) const
{
  return GetCaloGeometry()->rzmid(sample,eta);
}

double FastCaloSimCaloExtrapolation::rent(int sample,double eta) const
{
  return GetCaloGeometry()->rent(sample,eta);
}

double FastCaloSimCaloExtrapolation::zent(int sample,double eta) const
{
  return GetCaloGeometry()->zent(sample,eta);
}

double FastCaloSimCaloExtrapolation::rzent(int sample,double eta) const
{
  return GetCaloGeometry()->rzent(sample,eta);
}

double FastCaloSimCaloExtrapolation::rext(int sample,double eta) const
{
  return GetCaloGeometry()->rext(sample,eta);
}

double FastCaloSimCaloExtrapolation::zext(int sample,double eta) const
{
  return GetCaloGeometry()->zext(sample,eta);
}

double FastCaloSimCaloExtrapolation::rzext(int sample,double eta) const
{
  return GetCaloGeometry()->rzext(sample,eta);
}

double FastCaloSimCaloExtrapolation::rpos(int sample,double eta,int subpos) const
{
  return GetCaloGeometry()->rpos(sample,eta,subpos);
}

double FastCaloSimCaloExtrapolation::zpos(int sample,double eta,int subpos) const
{
  return GetCaloGeometry()->zpos(sample,eta,subpos);
}

double FastCaloSimCaloExtrapolation::rzpos(int sample,double eta,int subpos) const
{
  return GetCaloGeometry()->rzpos(sample,eta,subpos);
}
