/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 The Mdt detector = a multilayer = MDT in amdb 
 ----------------------------------------------------
 Copyright (C) 2004 by ATLAS Collaboration
 ***************************************************************************/

//<doc><file>	$Id: MdtReadoutElement.cxx,v 1.4 2009-05-16 13:08:13 stefspa Exp $
//<version>	$Name: not supported by cvs2svn $

//<<<<<< INCLUDES                                                       >>>>>>

#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "MuonReadoutGeometry/MuonStation.h"

#include "TrkSurfaces/CylinderBounds.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"


#include "MuonAlignmentData/BLinePar.h"

#include "GaudiKernel/MsgStream.h"
#include <cassert>

// From Dan Levin: MDT 
// linear density of wire: lambda=wireLinearDensity=19.3 [gm/cm^3] * PI*
//(25 *10^-4 )^2 [CLHEP::cm^2] = 378.954 microgram/CLHEP::cm
#define linearDensity 378.954
// From Dan Levin: MDT
// wireTen=350 for most chambers,  285 gm for some NIKHEF chambers (BOL ?),

#define verbose_Bline false
// Typical b-line par  values
// m_bz = 0.01;   // 10 microns
// m_bp = 0.1;    // 100 microns
// m_bn = 0.1;    // 100 microns
// m_sp = 0.001;  // 1 micron
// m_sn = 0.001;  // 1 micron
// m_tw = 0.1;    // 100 microns
// m_pg = 0.1;    // 100 microns
// m_tr = 0.1;    // 100 microns
// m_eg = 1.0e-4; // 100 ppm
// m_ep = 1.0e-5; // 10 ppm
// m_en = 1.0e-5; // 10 ppm

namespace MuonGM {

MdtReadoutElement::MdtReadoutElement(GeoVFullPhysVol* pv, std::string stName,
                                     int zi, int fi, bool is_mirrored,
                                     MuonDetectorManager* mgr)
  : MuonReadoutElement(pv, zi, fi, is_mirrored, mgr),
    m_deformTransfs(0),
    m_BLinePar(0),
    m_elemNormal(0),
    m_tubeSurfaces(0),
    m_tubeTransf(0),
    m_tubeCenter(0),
    m_tubeBounds(0),
    m_backupTubeCenter(0),
    m_backupTubeTransf(0),
    m_backupDeformTransf(0),
    m_associatedSurface(0),
    m_associatedBounds(0)
{
  m_MsgStream = new MsgStream(mgr->msgSvc(),"MuGM:MdtReadoutElement");
  m_multilayer = 0;



  // get the setting of the caching flag from the manager
  setCachingFlag(mgr->cachingFlag());
    
  m_inBarrel     = false;
  if (stName.substr(0,1) == "B") m_inBarrel=true;
  m_descratzneg  = false;
  if (zi<0 && !is_mirrored) m_descratzneg = true;

  setStationName(stName);
  _zsignRO_tubeFrame = 0;


}


MdtReadoutElement::~MdtReadoutElement()
{
  delete m_elemNormal;
  delete m_associatedSurface;
  delete m_associatedBounds;
  //delete m_deformTransf;
   
  if (m_deformTransfs) {
    for (unsigned int tsf = 0; tsf < m_deformTransfs->size(); ++tsf) delete (*m_deformTransfs)[tsf];
    delete m_deformTransfs;
  }

  if (m_tubeSurfaces) {
    for (unsigned int tsf = 0; tsf < m_tubeSurfaces->size(); ++tsf) delete (*m_tubeSurfaces)[tsf];
    delete m_tubeSurfaces;
  }
   
  if (m_tubeTransf) {
    for (unsigned int ttf = 0; ttf < m_tubeTransf->size(); ++ttf) delete (*m_tubeTransf)[ttf];
    delete m_tubeTransf;
  }

  if (m_tubeCenter) {
    for (unsigned int tcf = 0; tcf < m_tubeCenter->size(); ++tcf) delete (*m_tubeCenter)[tcf];
    delete m_tubeCenter;
  }

  if (m_tubeBounds) {
    for (unsigned int tcf = 0; tcf < m_tubeBounds->size(); ++tcf) delete (*m_tubeBounds)[tcf];
    delete m_tubeBounds;
  }

  if (m_backupTubeTransf) {
    for (unsigned int btt = 0; btt < m_backupTubeTransf->size(); ++btt) delete (*m_backupTubeTransf)[btt];
    delete m_backupTubeTransf;
  }

  if (m_backupDeformTransf) {
    for (unsigned int bdt = 0; bdt < m_backupDeformTransf->size(); ++bdt) delete (*m_backupDeformTransf)[bdt];
    delete m_backupDeformTransf;
  }

  if (m_backupTubeCenter) {
    for (unsigned int btc = 0; btc < m_backupTubeCenter->size(); ++btc) delete (*m_backupTubeCenter)[btc];
    delete m_backupTubeCenter;
  }
 
}    

bool MdtReadoutElement::getWireFirstLocalCoordAlongZ(int tubeLayer, double& coord) const
{
  coord = -9999.;
  if (tubeLayer>getNLayers() || tubeLayer<1) return false;
  coord = m_firstwire_x[tubeLayer-1];
  return true;
}
bool MdtReadoutElement::getWireFirstLocalCoordAlongR(int tubeLayer, double& coord) const
{
  coord = -9999.;
  if (tubeLayer>getNLayers() || tubeLayer<1) return false;
  coord = m_firstwire_y[tubeLayer-1];
  return true;
}

double MdtReadoutElement::innerTubeRadius() const 
{
  return manager()->getGenericMdtDescriptor()->innerRadius;
}
double MdtReadoutElement::outerTubeRadius() const 
{
  //  std::cout<<" asking for outer radius; inner first "<<manager()->getGenericMdtDescriptor()->innerRadius<<std::endl;
  //  std::cout<<" asking for outer radius; outer first "<<manager()->getGenericMdtDescriptor()->outerRadius<<std::endl;
  return manager()->getGenericMdtDescriptor()->outerRadius;
}


double MdtReadoutElement::getTubeLengthForCaching(int tubeLayer, int tube) const
{
  //std::cout<<"In getTubeLengthForCaching  "<<std::endl;
  
  const MdtIdHelper* idh = manager()->mdtIdHelper();
  //reLog() << MSG::INFO << "getTubeLenght for Element named " << getStationName() << " with tech. "
  //<< getTechnologyType() <<" DEid = "<<idh->show_to_string(identify())<< " called with: tubeL, tube " << tubeLayer << " "
  //<< tube <<endreq;
  double nominalTubeLength = 0.;
  if (m_inBarrel) nominalTubeLength = m_Ssize;
  else {
    int istep = int((tube-1)/m_ntubesinastep);
    if (istep < 0 || istep >= m_nsteps)
    {
      reLog() << MSG::WARNING << "getTubeLenght for Element named " << getStationName() << " with tech. "
	      << getTechnologyType() <<" DEid = "<<idh->show_to_string(identify())<< " called with: tubeL, tube " << tubeLayer << " "
	      << tube << "; step " << istep << " out of range 0-" << m_nsteps-1
	      << " m_ntubesinastep " << m_ntubesinastep <<endreq;
      istep =  0;
    }
    nominalTubeLength =  m_tubelength[istep];
  }

  //reLog() << MSG::VERBOSE << " nominal Tube length = "<<nominalTubeLength<<endreq;
  //std::cout<<" nominal Tube length = "<<nominalTubeLength<<std::endl; 
  double tlength=nominalTubeLength; 

  if (hasCutouts()){
    if (manager()->MinimalGeoFlag() == 0) {
      //std::cout<<" are we here ... CUTOUTS+minimalgeo==0"<<std::endl;     
      if (reLog().level() <= MSG::VERBOSE)  
	reLog() << MSG::VERBOSE << " MdtReadoutElement " << getStationName() << " stEta " << getStationEta() << " stPhi " << getStationPhi() 
		<< "multilayer "<<getMultilayer()<<" has cutouts, check for real tube length for tubeLayer, tube "<< tubeLayer<<" "<<tube<< endreq;
      PVConstLink cv = getMaterialGeom(); // it is "Multilayer"
      int nGrandchildren = cv->getNChildVols();
      if (reLog().level() <= MSG::VERBOSE) reLog() << MSG::VERBOSE << " nchild = "<<nGrandchildren<<endreq;
      if (nGrandchildren <= 0) return tlength;
      // child vol 0 is foam; 1 to (nGrandchildren-1) should be tubes
      int ii = (tubeLayer-1)*m_ntubesperlayer+tube;
      if (ii>=nGrandchildren) 
	{
	  reLog() << MSG::WARNING << " MdtReadoutElement " << getStationName() << " stEta " << getStationEta()
		  << " stPhi " << getStationPhi() << " multilayer "<<getMultilayer()<<" has cutouts, nChild = "<<nGrandchildren
		  <<" --- getTubeLength is looking for child with index ii="<<ii<<" for tubeL and tube = "<<tubeLayer<<" "<<tube<< endreq;
	  reLog() << MSG::WARNING << "returning nominalTubeLength "<<endreq; 
	  //ii = nGrandchildren-1;
	  return tlength;
	}
      if (ii<=0) 
	{
	  reLog() << MSG::WARNING << " MdtReadoutElement " << getStationName() << " stEta " << getStationEta()
		  << " stPhi " << getStationPhi() << " multilayer "<<getMultilayer()<<" has cutouts, nChild = "<<nGrandchildren
		  <<" --- getTubeLength is looking for child with index ii="<<ii<<" for tubeL and tube = "<<tubeLayer<<" "<<tube<< endreq;
	  reLog() << MSG::WARNING << "returning nominalTubeLength "<<endreq; 
	  return tlength;
	}
      PVConstLink physChild = cv->getChildVol(ii);
      const GeoShape* shape = physChild->getLogVol()->getShape();
      if (shape==0)  return tlength;
      const GeoTube* theTube = dynamic_cast<const GeoTube*> (shape);
      if(theTube!=0) {
	  //theTube->getRMin();
	  //theTube->getRMax();
	  tlength = 2.*theTube->getZHalfLength();
      }
      else reLog() << MSG::WARNING << "PhysChild with index "<<ii<<" out of (tubelayer-1)*m_ntubesperlayer+tube with tl="
		   <<tubeLayer<<" tubes/lay="<<m_ntubesperlayer<<" t="<<tube<<" for  MdtReadoutElement " << getStationName() << " stEta " << getStationEta()
		   << " stPhi " << getStationPhi() << " ml = "<<getMultilayer()<<endreq;
    }
    if (fabs(tlength-nominalTubeLength)>0.1)
    {
     if ( reLog().level() <= MSG::DEBUG ) reLog() << MSG::DEBUG << "Tube "<<tube<<" in tubeLayer = "<<tubeLayer
						  <<" is affected by a cutout: eff. length =  "<<tlength<<" while nominal = "<<nominalTubeLength
						  <<" in station "<< getStationName() << " at Eta/Phi " << getStationEta()
						  << "/" << getStationPhi() << " ml "<<getMultilayer()
						  <<endreq;
    }
    else 
      {
	if (reLog().level() <= MSG::DEBUG)  
	  reLog() << MSG::DEBUG << "Tube "<<tube<<" in tubeLayer = "<<tubeLayer
		  <<" is NOT affected by the cutout: eff. length =  "<<tlength<<" while nominal = "<<nominalTubeLength
		  <<endreq;
      }
  }
  return tlength;
}


double MdtReadoutElement::tubeLength(Identifier id) const
{
  
  const MdtIdHelper* idh = manager()->mdtIdHelper();
  //std::cout<<" in tube lenght for channel "<<idh->show_to_string(id)<<std::endl;
  int layer    = idh->tubeLayer(id);
  int tube     = idh->tube(id);
  return getTubeLength(layer, tube);
}

double MdtReadoutElement::distanceFromRO(Amg::Vector3D x, Identifier id) const
{
  //MsgStream log(m_msgSvc, "MuGM:MdtReadout");
  // x is given in the global reference frame
  const MdtIdHelper* idh = manager()->mdtIdHelper();
  int tubelayer = idh->tubeLayer(id);
  int tube = idh->tube(id);
  Amg::Vector3D cPos = center(tubelayer, tube);
  Amg::Vector3D roPos = ROPos(id);
  // reLog()<<MSG::DEBUG<<" !!!!!!!!!!!!! show mypoint, ro, and tube_centre "<<x<<"/"<<roPos<<"/"<<cPos<<endreq;
    
  Amg::Vector3D c_ro (cPos.x()-roPos.x(),cPos.y()-roPos.y(),cPos.z()-roPos.z());
  Amg::Vector3D x_ro (x.x()-roPos.x(),x.y()-roPos.y(),x.z()-roPos.z());

  double scalprod = c_ro.x()*x_ro.x()+c_ro.y()*x_ro.y()+c_ro.z()*x_ro.z();
  double wlen = getWireLength(tubelayer, tube);
  if (wlen > 10.*CLHEP::mm) 
    scalprod = fabs(2.*scalprod/getWireLength(tubelayer, tube));
  else {
    double xx = x.x();
    double xy = x.y();
    double xz = x.z();
    reLog() << MSG::ERROR
           << " Distance of Point " << xx << " " << xy << " " << xz
           << " from RO side cannot be calculated (=0) since wirelength = "
           << wlen << endreq;
    scalprod = 0.;
  }  
  return scalprod;
}


double 
MdtReadoutElement::distanceFromRO(Amg::Vector3D x, int multilayer, int tubelayer, int tube) const
{
  // x is given in the global reference frame
  const MdtIdHelper* idh = manager()->mdtIdHelper();
  Identifier id = idh->channelID(idh->parentID(identify()), multilayer, tubelayer, tube);
  return distanceFromRO(x, id);
}


int MdtReadoutElement::isAtReadoutSide(Amg::Vector3D GlobalHitPosition, Identifier id) const
{
    //MsgStream log(m_msgSvc, "MuGM:MdtReadout");
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    int tubel = idh->tubeLayer(id);
    int tube  = idh->tube(id);
    double distance = distanceFromRO(GlobalHitPosition, id);
    if (distance < 0) {
        reLog()<<MSG::WARNING
           <<"MdtReadoutElement::isAtReadoutSide GlobalHitPosition"
           <<" appears to be outside the tube volume "<<distance<<endreq;
        return 1;
    }
    else if (distance <= getWireLength(tubel, tube)/2.) return 1;
    else if (distance <  getWireLength(tubel, tube)   ) return -1;
    else 
    {
        reLog()<<MSG::WARNING<<"MdtReadoutElement::isAtReadoutSide "
           <<"GlobalHitPosition appears to be outside the tube volume "
           <<distance<<endreq;
        return -1;
    }
}
int MdtReadoutElement::isAtReadoutSide(Amg::Vector3D GlobalHitPosition, int ml, int tubel, int tube) const
{
    //MsgStream log(m_msgSvc, "MuGM:MdtReadout");
    double distance = distanceFromRO(GlobalHitPosition, ml, tubel, tube);
    if (distance < 0) {
        reLog()<<MSG::WARNING<<"MdtReadoutElement::isAtReadoutSide "
           <<"GlobalHitPosition appears to be outside the tube volume "
           <<distance<<endreq;
        return 1;
    }
    else if (distance <= getWireLength(tubel, tube)/2.) return 1;
    else if (distance <  getWireLength(tubel, tube)   ) return -1;
    else 
    {
        reLog()<<MSG::WARNING<<"MdtReadoutElement::isAtReadoutSide "
           <<"GlobalHitPosition appears to be outside the tube volume "
           <<distance<<endreq;
        return -1;
    }
}

double MdtReadoutElement::RODistanceFromTubeCentre(const Identifier& id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    int ml   = idh->multilayer(id);
    int layer    = idh->tubeLayer(id);
    int tube     = idh->tube(id);
    return RODistanceFromTubeCentre(ml, layer, tube);
}
double MdtReadoutElement::signedRODistanceFromTubeCentre(const Identifier& id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    int ml   = idh->multilayer(id);
    int layer    = idh->tubeLayer(id);
    int tube     = idh->tube(id);
    return signedRODistanceFromTubeCentre(ml, layer, tube);
}
double MdtReadoutElement::RODistanceFromTubeCentre(int multilayer, int tubelayer, int tube) const
{
    // it is a un-signed quantity:
    if (multilayer != m_multilayer) {
        assert(0);
        return -99999.;
    }
    
    return getWireLength(tubelayer, tube)/2.;
}
double MdtReadoutElement::signedRODistanceFromTubeCentre(int multilayer, int tubelayer, int tube) const
{
    // it is a signed quantity:
    // the sign corresponds to the sign of the z coordinate of the RO endplug in the tube
    // reference frame 
    if (multilayer != m_multilayer) {
        assert(0);
        return -99999.;
    }

    int amdb_plus_minus1 = 1;
    //std::cerr<<"MdtReadoutElement::signedRODistanceFromTubeCentre // _zsignRO_tubeFrame = "<<_zsignRO_tubeFrame<<std::endl;
    if ( _zsignRO_tubeFrame == 0 )
    {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        Identifier id = idh->channelID(idh->parentID(identify()), multilayer, tubelayer, tube);
        //std::cerr<<"MdtReadoutElement::signedRODistanceFromTubeCentre // computing for ID = "<<idh->show_to_string(id)<<std::endl;
        const MuonStation*    ms = parentMuonStation();
        //std::cerr<<"MdtReadoutElement::signedRODistanceFromTubeCentre // MuonStation* = "<<ms<<std::endl;
        if (fabs(ms->xAmdbCRO()) > 10.) 
        {
            
            Amg::Vector3D tem = Amg::Vector3D(ms->xAmdbCRO(),0.,0.);
            //std::cerr<<"MdtReadoutElement::signedRODistanceFromTubeCentre // temporary point "<<tem<<std::endl;

            Amg::Transform3D amdbToGlobal = Amg::CLHEPTransformToEigen(ms->getAmdbLRSToGlobal());
            Amg::Vector3D temGlo = amdbToGlobal*tem;
            //std::cerr<<"MdtReadoutElement::signedRODistanceFromTubeCentre // temporary point in glob r.f. "<<temGlo<<std::endl;
            Amg::Vector3D ROtubeFrame = nodeform_globalToLocalCoords(temGlo, id);
            //std::cerr<<"MdtReadoutElement::signedRODistanceFromTubeCentre // ROtubeFrame= "<<ROtubeFrame<<std::endl;
            if (ROtubeFrame.z()<0) _zsignRO_tubeFrame = -1;
            else _zsignRO_tubeFrame = 1;
        }
    }
    if ( _zsignRO_tubeFrame == 0)
    { // if no CRO in a chamber in AMDB (BIS in layout R), use the standard convention for RO-HV side
        int sign = 0;
        if (barrel()) 
        {
            if (sideA())
            {
                if (largeSector()) sign = -1;
                else sign = 1;
            }
            else 
            {
                if (largeSector()) sign = 1;
                else sign = -1;
            }
	    // a special case is BIS in sector 12 
	    if (getStationName().substr(0,3)=="BIS" && getStationPhi()==6) sign = -sign;
        }
        else 
        {
            if (sideA())
            {        
                if (largeSector()) sign = 1;
                else sign = -1;
            }
            else 
            {
                if (largeSector()) sign = -1;
                else sign = 1;
            }
        }
        _zsignRO_tubeFrame = sign;
    }
    amdb_plus_minus1 = _zsignRO_tubeFrame;
    if (amdb_plus_minus1 == 0)
    {
        //MsgStream log(m_msgSvc, "MuGM:MdtReadout");     
        reLog()<<MSG::ERROR<<"Unable to get the sign of RO side; signedRODistancefromTubeCenter returns 0"<<endreq;
    }
    
    return amdb_plus_minus1*getWireLength(tubelayer, tube)/2.;
}

const Amg::Vector3D MdtReadoutElement::tubeFrame_localROPos(int multilayer, int tubelayer, int tube) const
{
    Amg::Vector3D Pro(0., 0., signedRODistanceFromTubeCentre(multilayer, tubelayer, tube));
    return Pro;
}
const Amg::Vector3D MdtReadoutElement::tubeFrame_localROPos(Identifier id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    int ml   = idh->multilayer(id);
    int layer    = idh->tubeLayer(id);
    int tube     = idh->tube(id);
    return tubeFrame_localROPos(ml, layer, tube);
}
const Amg::Vector3D MdtReadoutElement::localROPos(int multilayer, int tubelayer, int tube) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    Identifier id = idh->channelID(idh->parentID(identify()), multilayer, tubelayer, tube);
    return tubeToMultilayerCoords(tubeFrame_localROPos(multilayer, tubelayer, tube) , id);    
}
const Amg::Vector3D MdtReadoutElement::localROPos(Identifier id) const
{
    return tubeToMultilayerCoords(tubeFrame_localROPos(id) , id);
}

const Amg::Vector3D 
MdtReadoutElement::ROPos(int multilayer, int tubelayer, int tube) const
{
  return transform(tubelayer, tube)*tubeFrame_localROPos(multilayer, tubelayer, tube);
}

const Amg::Vector3D 
MdtReadoutElement::ROPos(Identifier id) const
{
  const MdtIdHelper* idh = manager()->mdtIdHelper();
  int ml = idh->multilayer(id);
  int layer = idh->tubeLayer(id);
  int tube = idh->tube(id);
  return ROPos(ml, layer, tube);
}


const Amg::Vector3D 
MdtReadoutElement::localTubePos(int multilayer, int tubelayer, int tube) const
{
  //  std::cerr<<"In localTubepos(ml,tl,t) before nodeform_localtubepos"<<std::endl;
  const Amg::Vector3D idealPos     = nodeform_localTubePos(multilayer, tubelayer, tube);
  //  std::cerr<<"In localTubepos(ml,tl,t) after nodeform_localtubepos -- idealpos ="<<idealPos<<std::endl;
  const Amg::Transform3D toDeform = fromIdealToDeformed(multilayer, tubelayer, tube);
  //  std::cerr<<"In localTubepos(ml,tl,t) after the transform ideal-to-def "<<std::endl;
  return toDeform*idealPos;  
}


const Amg::Vector3D 
MdtReadoutElement::nodeform_localTubePos(int multilayer, int tubelayer, int tube) const
{
    if (multilayer != m_multilayer)
    {
        reLog()<<MSG::WARNING<<"asking WRONG QUESTION: nodeform_localTubePos with args. ml/tl/t = "<<multilayer<<"/"<<tubelayer<<"/"<<tube
               <<" to MdtReadoutElement "<<getStationName()<<"/"<<getTechnologyName()<<" eta/phi "<<getStationEta()<<"/"<<getStationPhi()
               <<" multilayer "<<getMultilayer()<<endreq;
        reLog()<<MSG::WARNING<<"will answer for  args:                                  ml/tl/t = "<<getMultilayer()<<"/"<<tubelayer<<"/"<<tube<<endreq;
        multilayer = m_multilayer;
    }
#ifndef NDEBUG
    if ( reLog().level() <= MSG::VERBOSE )
        reLog()<<MSG::VERBOSE<<" Computing LocalTubePos for "
               <<getStationName()<<"/"<<getTechnologyName()<<" eta/phi "<<getStationEta()<<"/"<<getStationPhi()
               <<" ml/tl/t "<<multilayer<<"/"<<tubelayer<<"/"<<tube<<endreq;
#endif

    double xtube = 0.;
    double ytube = 0.;
    double ztube = 0.;
    if (m_inBarrel) {
      xtube = -m_Rsize/2. + m_firstwire_y[tubelayer-1];
      ztube = -m_Zsize/2. + m_firstwire_x[tubelayer-1] + (tube-1)*m_tubepitch;
    } else {
      xtube = -m_Zsize/2. + m_firstwire_y[tubelayer-1];
      ztube = -m_Rsize/2. + m_firstwire_x[tubelayer-1] + (tube-1)*m_tubepitch;
    }

    if (hasCutouts()) {
      if (manager()->MinimalGeoFlag() == 0) {
        if ( reLog().level() <= MSG::DEBUG ) reLog() << MSG::DEBUG << " MdtReadoutElement " << getStationName() << " stEta " << getStationEta()
						     << " stPhi " << getStationPhi() << " has cutouts, check for real position of tubes " << endreq;
        PVConstLink cv = getMaterialGeom(); // it is "Multilayer"
        int nGrandchildren = cv->getNChildVols();
        // child vol 0 is foam; 1 to (nGrandchildren-1) should be tubes
        int ii = (tubelayer-1)*m_ntubesperlayer+tube;
        HepGeom::Transform3D tubeTrans = cv->getXToChildVol(ii);
        PVConstLink tv = cv->getChildVol(ii);
        double maxtol = 0.0000001;

        if (std::abs(xtube - tubeTrans[0][3]) > maxtol ||
            //std::abs(ytube - tubeTrans[1][3]) > maxtol && std::abs(tubeTrans[1][3]) > maxtol ||
            std::abs(ztube - tubeTrans[2][3]) > maxtol) {
          reLog()<<MSG::ERROR << "taking localTubepos from RAW geoModel!!! MISMATCH IN local Y-Z (amdb) for a MDT with cutouts "
                 << endreq << ": from tube-id and pitch, tube pos = " << xtube
                 << ", " << ytube << ", " << ztube
                 << " but geoModel gives " << tubeTrans[0][3]
                 << ", " << tubeTrans[1][3] << ", " << tubeTrans[2][3]
                 << endreq
                 << " for tube " << tube << " tube layer " << tubelayer
                 << " multilayer " << multilayer << endreq
                 << " There are " << nGrandchildren << " child volumes & "
                 << m_ntubesperlayer*m_nlayers << " tubes expected."
                 << " There should be " << m_nlayers << " layers and "
                 << m_ntubesperlayer << " tubes per layer."
                 <<endreq;
        }
        if (tubeTrans[1][3]> maxtol)
        {
            if ( reLog().level() <= MSG::DEBUG ) 
	      reLog()<<MSG::DEBUG << "This a tube with cutout stName/Eta/Phi/ml/tl/t = "<<getStationName()<<"/"<<getStationEta()<<"/"<<getStationPhi()<<"/"
		     <<getMultilayer()<<"/"<<tubelayer<<"/"<<tube<<endreq;
            if (std::abs(m_cutoutShift - tubeTrans[1][3]) > maxtol) // check only for tubes actually shifted 
            {
                reLog()<<MSG::ERROR << "taking localTubepos from RAW geoModel!!! MISMATCH IN local X (amdb) for a MDT with cutouts "
                       << endreq << ": from tube-id/pitch/cutout  tube pos = " << xtube
                       << ", " << m_cutoutShift  << ", " << ztube
                       << " but geoModel gives " << tubeTrans[0][3]
                       << ", " << tubeTrans[1][3] << ", " << tubeTrans[2][3]
                       << endreq
                       << " for tube " << tube << " tube layer " << tubelayer
                       << " multilayer " << multilayer << endreq
                       << " There are " << nGrandchildren << " child volumes & "
                       << m_ntubesperlayer*m_nlayers << " tubes expected."
                       << " There should be " << m_nlayers << " layers and "
                       << m_ntubesperlayer << " tubes per layer."
                       <<endreq;
            }
        }
        
        Amg::Vector3D x(tubeTrans[0][3],tubeTrans[1][3],tubeTrans[2][3]);
        if (tube > m_ntubesperlayer || tubelayer > m_nlayers) {
          x = Amg::Vector3D(xtube,ytube,ztube);
        }
        return x;
      }
    }
    return Amg::Vector3D(xtube,ytube,ztube);
}


const Amg::Vector3D MdtReadoutElement::localTubePos(Identifier id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    int ml   = idh->multilayer(id);
    int layer    = idh->tubeLayer(id);
    int tube     = idh->tube(id);

    //std::cerr<<" localTubePos for Id = "<<idh->show_to_string(id)<<std::endl;
    return localTubePos(ml, layer, tube);
}
const Amg::Vector3D MdtReadoutElement::nodeform_localTubePos(Identifier id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    int ml   = idh->multilayer(id);
    int layer    = idh->tubeLayer(id);
    int tube     = idh->tube(id);

    //std::cerr<<" localTubePos for Id = "<<idh->show_to_string(id)<<std::endl;
    return nodeform_localTubePos(ml, layer, tube);
}
const Amg::Vector3D MdtReadoutElement::nodeform_tubePos(int multilayer, int tubelayer, int tube) const
{
    //MsgStream log(m_msgSvc, "MuGM:MdtReadout");
    if (multilayer != m_multilayer) assert(0);

    //reLog()<<MSG::DEBUG<<" MdtReadoutElement::tubePos(ml,tl,t) going to look for local coord.s"<<endreq;
    const Amg::Vector3D lp = nodeform_localTubePos(multilayer, tubelayer, tube);
    //reLog()<<MSG::DEBUG<<" MdtReadoutElement::tubePos(ml,tl,t) going to look for det transform"<<endreq;    
    //const Amg::Transform3D mdtTrans = getMaterialGeom()->getAbsoluteTransform();
    const Amg::Transform3D mdtTrans = transform();
    // reLog()<<MSG::INFO<<" MdtReadoutElement::tubePos(ml,tl,t) got localP and trans "<<endreq;
    
    return mdtTrans * lp;

}
const Amg::Vector3D MdtReadoutElement::tubePos(int multilayer, int tubelayer, int tube) const
{
    if (multilayer != m_multilayer) assert(0);

    if ( reLog().level() <= MSG::VERBOSE ) {
      reLog()<<MSG::VERBOSE<<"in tubePos-- id "<<(manager()->mdtIdHelper())->show_to_string(identify())
       <<" ml, tl, t = "<<multilayer<<", "<<tubelayer<<", "<<tube<<endreq;
      reLog()<<MSG::VERBOSE<<" MdtReadoutElement::tubePos(ml,tl,t) going to look for local coord.s"<<endreq;    
    }
    const Amg::Vector3D lp = localTubePos(multilayer, tubelayer, tube);
    if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<" MdtReadoutElement::tubePos(ml,tl,t) going to look for det transform"<<endreq;
    const Amg::Transform3D mdtTrans = absTransform();
    if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<" MdtReadoutElement::tubePos(ml,tl,t) got localP and trans "<<endreq;
    
    return mdtTrans * lp;

}
const Amg::Vector3D MdtReadoutElement::nodeform_tubePos(Identifier id) const
{
    //MsgStream log(m_msgSvc, "MuGM:MdtReadout");
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    int ml   = idh->multilayer(id);
    int layer    = idh->tubeLayer(id);
    int tube     = idh->tube(id);
#ifndef NDEBUG
    if ( reLog().level() <= MSG::VERBOSE ) 
      {
	reLog()<<MSG::VERBOSE<<" Computing tubePos for     id  = "<<idh->show_to_string(id)<<endreq;
	reLog()<<MSG::VERBOSE<<" in MdtReadoutElement with id  = "<<idh->show_to_string(identify())<<endreq;
      }
#endif
    
    return nodeform_tubePos(ml, layer, tube);
}
const Amg::Vector3D MdtReadoutElement::tubePos(Identifier id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    int ml   = idh->multilayer(id);
    int layer    = idh->tubeLayer(id);
    int tube     = idh->tube(id);
#ifndef NDEBUG
    if ( reLog().level() <= MSG::VERBOSE ) 
      {
	reLog()<<MSG::VERBOSE<<" Computing tubePos for     id  = "<<idh->show_to_string(id)<<endreq;
	reLog()<<MSG::VERBOSE<<" in MdtReadoutElement with id  = "<<idh->show_to_string(identify())<<endreq;
      }
#endif
    
    return tubePos(ml, layer, tube);
}

const Amg::Vector3D MdtReadoutElement::tubeToMultilayerCoords(Amg::Vector3D x, Identifier id) const
{
    const Amg::Vector3D tp = nodeform_localTubePos(id);
    const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
    const Amg::Transform3D toDeform = fromIdealToDeformed(id);
    return toDeform*xfp*Amg::Vector3D( x.x(), -x.z(), x.y() );
}
const Amg::Vector3D MdtReadoutElement::nodeform_tubeToMultilayerCoords(Amg::Vector3D x, Identifier id) const
{
    const Amg::Vector3D tp = nodeform_localTubePos(id);
    const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
    return xfp*Amg::Vector3D( x.x(), -x.z(), x.y() );
}
const Amg::Transform3D  MdtReadoutElement::tubeToMultilayerTransf(Identifier id) const
{
    const Amg::Vector3D tp = nodeform_localTubePos(id);
    const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
    const Amg::Transform3D toDeform = fromIdealToDeformed(id);
    return toDeform*xfp*Amg::AngleAxis3D(90.*CLHEP::deg,Amg::Vector3D(1.,0.,0.));
}
const Amg::Transform3D  MdtReadoutElement::nodeform_tubeToMultilayerTransf(Identifier id) const
{
    const Amg::Vector3D tp = nodeform_localTubePos(id);
    const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
    return xfp*Amg::AngleAxis3D(90.*CLHEP::deg,Amg::Vector3D(1.,0.,0.));
}

const Amg::Vector3D 
MdtReadoutElement::multilayerToTubeCoords(Amg::Vector3D x, Identifier id) const
{
  const Amg::Vector3D tp = nodeform_localTubePos(id);
  const Amg::Translation3D xfp(-tp.x(), -tp.y(), -tp.z());
  const Amg::Transform3D fromDeform = fromIdealToDeformed(id).inverse();
  return Amg::AngleAxis3D(-90.*CLHEP::deg,Amg::Vector3D(1.,0.,0.))*xfp*fromDeform*x;
}

const Amg::Vector3D 
MdtReadoutElement::nodeform_multilayerToTubeCoords(Amg::Vector3D x, Identifier id) const
{
  const Amg::Vector3D tp = nodeform_localTubePos(id);
  const Amg::Translation3D xfp(-tp.x(), -tp.y(), -tp.z());
  return Amg::AngleAxis3D(-90.*CLHEP::deg,Amg::Vector3D(1.,0.,0.))*xfp*x;
}

const Amg::Transform3D  MdtReadoutElement::multilayerToTubeTransf(Identifier id) const
{
    const Amg::Vector3D tp = nodeform_localTubePos(id);
    const Amg::Translation3D xfp(-tp.x(), -tp.y(), -tp.z());
    const Amg::Transform3D fromDeform = fromIdealToDeformed(id).inverse();
    return Amg::AngleAxis3D(-90.*CLHEP::deg,Amg::Vector3D(1.,0.,0.))*xfp*fromDeform;
}
const Amg::Transform3D  MdtReadoutElement::nodeform_multilayerToTubeTransf(Identifier id) const
{
    const Amg::Vector3D tp = nodeform_localTubePos(id);
    const Amg::Translation3D xfp(-tp.x(), -tp.y(), -tp.z());
    return  Amg::AngleAxis3D(-90.*CLHEP::deg,Amg::Vector3D(1.,0.,0.))*xfp;
}

const Amg::Vector3D 
MdtReadoutElement::localToGlobalCoords(Amg::Vector3D x, Identifier id) const
{
  return transform(id)*x;
}

const Amg::Vector3D MdtReadoutElement::nodeform_localToGlobalCoords(Amg::Vector3D x, Identifier id) const
{
    const Amg::Vector3D tp = nodeform_localTubePos(id);
    const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
    return transform()*xfp*Amg::Vector3D( x.x(), -x.z(), x.y() );
}
const Amg::Transform3D MdtReadoutElement::localToGlobalTransf(Identifier id) const
{
    // a point at z=-L/2 goes at y=+L/2 
    const Amg::Vector3D tp = nodeform_localTubePos(id);
    const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
    const Amg::Transform3D toDeform = fromIdealToDeformed(id);
    return transform()*toDeform*xfp*Amg::AngleAxis3D(90.*CLHEP::deg,Amg::Vector3D(1.,0.,0.));
}
const Amg::Transform3D MdtReadoutElement::localToGlobalTransf(int tubeLayer, int tube) const
{
    // a point at z=-L/2 goes at y=+L/2 
    const Amg::Vector3D tp = nodeform_localTubePos(getMultilayer(), tubeLayer, tube);
    const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
    const Amg::Transform3D toDeform = fromIdealToDeformed(getMultilayer(), tubeLayer, tube);
    return transform()*toDeform*xfp*Amg::AngleAxis3D(90.*CLHEP::deg,Amg::Vector3D(1.,0.,0.));
}
const Amg::Transform3D MdtReadoutElement::nodeform_localToGlobalTransf(Identifier id) const
{
    const Amg::Vector3D tp = nodeform_localTubePos(id);
    const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
    return transform()*xfp*Amg::AngleAxis3D(90.*CLHEP::deg,Amg::Vector3D(1.,0.,0.));
}

const Amg::Transform3D 
MdtReadoutElement::globalToLocalTransf(Identifier id) const
{
  Amg::Transform3D mytransf = transform(id).inverse();
  return mytransf;
}

const Amg::Transform3D 
MdtReadoutElement::nodeform_globalToLocalTransf(Identifier id) const
{
  Amg::Transform3D mytransf = nodeform_localToGlobalTransf(id).inverse();
  return mytransf;
}

const Amg::Vector3D MdtReadoutElement::globalToLocalCoords(Amg::Vector3D x, Identifier id) const
{
    const Amg::Transform3D mytransf = globalToLocalTransf(id);
    Amg::Vector3D xx = mytransf * x;
    return xx;
}
const Amg::Vector3D MdtReadoutElement::nodeform_globalToLocalCoords(Amg::Vector3D x, Identifier id) const
{
    const Amg::Transform3D mytransf = nodeform_globalToLocalTransf(id);
    Amg::Vector3D xx = mytransf * x;
    return xx;
}
const Amg::Vector3D MdtReadoutElement::AmdbLRStubePos(Identifier id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    int ml   = idh->multilayer(id);
    int layer    = idh->tubeLayer(id);
    int tube     = idh->tube(id);
    return AmdbLRStubePos(ml, layer, tube);
}

const Amg::Vector3D MdtReadoutElement::AmdbLRStubePos(int multilayer,
                                                   int tubelayer,
                                                   int tube) const
{
   if (multilayer != m_multilayer) assert(0);
   const Amg::Vector3D tp = localTubePos(multilayer, tubelayer, tube);

   //Have the position in local(GM) MDT coords.
   //Need to translate to Amdb local (szt) Station Coord.s

   // go from local(GM) MDT  to local(GM) MuonStation
   const MuonStation*    ms = parentMuonStation();
   // the method in the following 3 lines gives the same result of Amg::Vector3D tpNativeMuonStation = toParentStation()*tp;
   Amg::Vector3D tpNativeMuonStation = toParentStation()*tp;
   
   Amg::Transform3D xf = Amg::CLHEPTransformToEigen(*ms->getNativeToAmdbLRS());

//    std::cerr<<" Station "<<getStationName()<<" tech "<<getTechnologyName()
//             <<" stPhi = "<<getStationPhi()<<" stEta "<<getStationEta()
//             <<" ml, tl, t = "<<multilayer<<" "<<tubelayer<<" "<<tube
//             <<" muonStation(native) pos, pos1 "<<tpNativeMuonStation<<" "<<tpNativeMuonStation
//             <<" AmdbLRStubePos "<<(*xf)*tpNativeMuonStation<<std::endl;
   
   return xf*tpNativeMuonStation;
}

const Amg::Transform3D& 
MdtReadoutElement::fromIdealToDeformed(const Identifier& id) const
{
  const MdtIdHelper* idh = manager()->mdtIdHelper();
  int ml = idh->multilayer(id);
  int layer = idh->tubeLayer(id);
  int tube = idh->tube(id);
  return fromIdealToDeformed(ml, layer, tube);
}


const Amg::Transform3D & MdtReadoutElement::fromIdealToDeformed(int multilayer, int tubelayer, int tube) const
{
  //MsgStream log(m_msgSvc, "MuGM:MdtReadout");
  //std::cerr<<"fromIdealToDeformed for MdtReadoutElement "
  //<<idh->show_to_string(identify())<<" ml, tl, t = "<<multilayer<<" "<<tubelayer<<" "<<tube<<std::endl;

  const MdtIdHelper* idh = manager()->mdtIdHelper();
  const MuonStation*  ms = parentMuonStation();
  HepGeom::Point3D<double> fpp = ms->getUpdatedBlineFixedPointInAmdbLRS();
  Amg::Vector3D fixedPoint(fpp.x(),fpp.y(),fpp.z());

  if ( 
      ( (!(manager()->applyMdtDeformations())) || (!(ms->hasBLines())) ) && 
      ( (!(manager()->applyMdtAsBuiltParams()))|| (!(ms->hasMdtAsBuiltParams())) )  	
     )
    {
      if (!m_deformTransfs) 
	{
	  m_deformTransfs  = new std::vector<Amg::Transform3D *>( 1 );
	  if (reLog().level() <= MSG::VERBOSE) 
	    reLog() << MSG::VERBOSE <<"RE "<< idh->show_to_string(identify())<<" created vector of deform-trasf with size 1 - pointer "<<(uintptr_t)m_deformTransfs<<endreq;
	}
      Amg::Transform3D * transf =  (*m_deformTransfs)[0];
      if( transf ) return *transf;
      transf = new Amg::Transform3D(Amg::Transform3D::Identity());
      (*m_deformTransfs)[0] = transf;	
      if (reLog().level() <= MSG::VERBOSE)
	reLog() << MSG::VERBOSE <<"RE "<< idh->show_to_string(identify())<<" creating deform-trasf as identity for tLayer, tube "<<tubelayer<<" "<<tube
		<<" globalIndex = "<<0<<" pointer "<<(uintptr_t)transf<<endreq;
	return *transf;
    }
  
   
  int ntot_tubes = m_nlayers * m_ntubesperlayer;
  if (!m_deformTransfs ) {	
      if (reLog().level() <= MSG::VERBOSE) reLog() << MSG::VERBOSE <<"Creating vector of deform-Transformations for RE "<< idh->show_to_string(identify())
	      <<" with "<<ntot_tubes << " components"<<endreq;
      m_deformTransfs  = new std::vector<Amg::Transform3D *>( ntot_tubes );
      for (int j=0; j<ntot_tubes; ++j)  (*m_deformTransfs)[j] = 0;
  }
  int itube = (tubelayer-1)*m_ntubesperlayer + tube - 1;
  if (itube>=ntot_tubes) {
    reLog() << MSG::WARNING <<"global index for tubelayer/tube =  "<<tubelayer<<"/"<<tube<<" is "<<itube<<" >=ntot_tubes ="<<ntot_tubes<<" RESETTING global index to 0"<<endreq;
    itube = 0;
  }
  Amg::Transform3D * transf =  (*m_deformTransfs)[itube];
  if( transf ) {
    if (reLog().level() <= MSG::VERBOSE) reLog() << MSG::VERBOSE <<"RE "<< idh->show_to_string(identify())
						 <<" reusing deform-trasf for tLayer, tube "<<tubelayer<<" "<<tube<<" globalIndex = "<<itube<<endreq;
      return *transf;
  }
  //std::cerr<<"create here the transform in the HEP for m_deformTransf "<<m_deformTransf<<std::endl;
  

  // Chamber parameters

  double width_narrow = m_Ssize;
  double width_wide = m_LongSsize;
  double heightML = m_inBarrel ? m_Zsize : m_Rsize;
  double thicknessML = m_inBarrel ? m_Rsize : m_Zsize;
  double height    = m_inBarrel ? ms->ZsizeMdtStation() : ms->RsizeMdtStation();
  double thickness = m_inBarrel ? ms->RsizeMdtStation() : ms->ZsizeMdtStation();
  if (fabs(height-heightML)>10. && reLog().level() <= MSG::DEBUG)  
    reLog() << MSG::DEBUG <<"RE "<< idh->show_to_string(identify())
	    <<"Different ML and MDTStation length in the precision coord.direction  ---  MultiLayerHeight, MDTstationHeigh "
	    <<heightML<<" "<<height<<" MultiLayerThickness, MDTstationThickness "<<thicknessML<<" "<<thickness<<endreq;
									        

  // Deformation parameters. This needs to be replaced by the info from the B-line object
  //
  // Notice, that only those parameters: sp, sn, tw, pg, tr, eg, keep the tubes straight, 
  // which is implicitly implied in this function. Therefore only these parameters can be
  // left non-zero before calling positionOnDeformedChamber()
  //

  double bz = m_BLinePar->bz(); 
  double bp = m_BLinePar->bp(); 
  double bn = m_BLinePar->bn(); 
  double sp = m_BLinePar->sp();
  double sn = m_BLinePar->sn();
  double tw = m_BLinePar->tw();
  double pg = m_BLinePar->pg();
  double tr = m_BLinePar->tr();
  double eg = m_BLinePar->eg();
  double ep = m_BLinePar->ep(); 
  double en = m_BLinePar->en(); 

  // Calculate transformation from native to AMDB. I took the idea from MdtReadoutElement::AmdbLRStubePos.
  // This code will be deleted after tests are done
  
  Amg::Transform3D toAMDB = Amg::CLHEPTransformToEigen(*ms->getNativeToAmdbLRS())*toParentStation();
  Amg::Transform3D fromAMDB = toAMDB.inverse();

  // Get positions of the wire center and end without deformations

  Amg::Vector3D pt_center   = nodeform_localTubePos(multilayer, tubelayer, tube);
  Amg::Vector3D pt_end_tube = tubeFrame_localROPos(multilayer, tubelayer, tube);
  Amg::Vector3D pt_end = Amg::Vector3D(pt_center.x()+pt_end_tube.x(),
				 pt_center.y()-pt_end_tube.z(),
				 pt_center.z()+pt_end_tube.y());


  
#ifdef TESTBLINES
  {
    std::cerr<<"******************* In fromIdealToDeformed "<<manager()->mdtIdHelper()->show_to_string(identify())<<" ml, tl, t = "<<multilayer<<" "<<tubelayer<<" "<<tube<<std::endl;
    std::cerr<<"in fromIdealToDeformed         : w,lw,h,t ="<<width_narrow<<" "<<width_wide<<" "<<height<<" "<<thickness<<std::endl;
    std::cerr<<"in fromIdealToDeformed (for ML): w,lw,h,t ="<<width_narrow<<" "<<width_wide<<" "<<heightML<<" "<<thicknessML<<std::endl;
      std::cerr<<"Here is tube_center native_Local "<<pt_center<<std::endl;
      std::cerr<<"Here is tube_end    native_Local "<<pt_end<<std::endl;
      Amg::Vector3D pp = toAMDB*pt_center;
      Amg::Vector3D pa = GlobalToAmdbLRSCoords(nodeform_tubePos(multilayer, tubelayer, tube));
      std::cerr<<"Here is toAMDB*tube_center "<<pp<<std::endl;
      std::cerr<<"tube_center in amdb frame  "<<pa<<std::endl;
      std::cerr<<"******************* In fromIdealToDeformed:  moving forward "<<std::endl;
  }
#endif  

  // Get positions after the deformations applied
   Amg::Vector3D localInAmdbFrameCenter_afterAllCorr = Amg::Vector3D(0.,0.,0.);
   Amg::Vector3D localInAmdbFrameEnd_afterAllCorr    = Amg::Vector3D(0.,0.,0.);
   /////////// wire centre 
   Amg::Vector3D localInAmdbFrameCenter_afterBline = 
       posOnDefChamStraightWire(toAMDB*pt_center,width_narrow,width_wide,height,thickness,
 			       bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en, fixedPoint);
   localInAmdbFrameCenter_afterAllCorr = localInAmdbFrameCenter_afterBline;
   // if there are as built parameters ... apply them here 
   localInAmdbFrameCenter_afterAllCorr = afterAsBuiltParamsInAmdbFrame(localInAmdbFrameCenter_afterBline,multilayer,tubelayer,tube);
   Amg::Vector3D my_pt_center_new = fromAMDB*localInAmdbFrameCenter_afterAllCorr;
 
   /////////// wire end point  
   Amg::Vector3D localInAmdbFrameEnd_afterBline = 
       posOnDefChamStraightWire(toAMDB*pt_end,width_narrow,width_wide,height,thickness,
 			       bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en, fixedPoint);
   localInAmdbFrameEnd_afterAllCorr = localInAmdbFrameEnd_afterBline;
   // if there are as built parameters ... apply them here 
   localInAmdbFrameEnd_afterAllCorr = afterAsBuiltParamsInAmdbFrame(localInAmdbFrameEnd_afterBline,multilayer,tubelayer,tube);
   Amg::Vector3D my_pt_end_new = fromAMDB*localInAmdbFrameEnd_afterAllCorr; 


  Amg::Vector3D pt_center_new;
  Amg::Vector3D pt_end_new;
   
  
  if (verbose_Bline)
  {
      std::cerr<<"my_pt_center_new = "<<my_pt_center_new<<std::endl;
      std::cerr<<"my_pt_end_new    = "<<my_pt_end_new<<std::endl;

      const Amg::Vector3D pt_center_amdb(-pt_center.y(), pt_center.x(), pt_center.z()+height/2);
      //std::cerr<<" pt_center_amdb "<<pt_center_amdb<<" going to positionOnDeformedChamber "<<std::endl;
      //const Amg::Vector3D pt_center_new_amdb = positionOnDeformedChamber(pt_center_amdb,width_narrow,width_wide,height,thickness,bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en);
      const Amg::Vector3D pt_center_new_amdb = posOnDefChamStraightWire(pt_center_amdb,width_narrow,width_wide,height,thickness,
								     bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en, fixedPoint);
      //std::cerr<<" pt_center_new_amdb "<<pt_center_new_amdb<<std::endl;
      pt_center_new = Amg::Vector3D(pt_center_new_amdb.y(), -pt_center_new_amdb.x(), pt_center_new_amdb.z()-height/2);  
      //std::cerr<<" pt_center_new "<<pt_center_new<<std::endl;
  
      const Amg::Vector3D pt_end_amdb(-pt_end.y(), pt_end.x(), pt_end.z()+height/2);
      //std::cerr<<" pt_end_amdb "<<pt_end_amdb<<" going to positionOnDeformedChamber"<<std::endl;
      //const Amg::Vector3D pt_end_new_amdb = positionOnDeformedChamber(pt_end_amdb,width_narrow,width_wide,height,thickness,
      //								   bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en);
      const Amg::Vector3D pt_end_new_amdb = posOnDefChamStraightWire(pt_end_amdb,width_narrow,width_wide,height,thickness,
								  bz, bp, bn, sp, sn, tw, pg, tr, eg, ep, en, fixedPoint);
      //std::cerr<<" pt_end_new_amdb "<<pt_end_new_amdb<<std::endl;
      pt_end_new = Amg::Vector3D(pt_end_new_amdb.y(), -pt_end_new_amdb.x(), pt_end_new_amdb.z()-height/2);
      std::cerr<<" BY HAND pt_center_new "<<pt_center_new<<std::endl;
      std::cerr<<" BY HAND pt_end_new    "<<pt_end_new<<std::endl;
  }
  
  pt_center_new = my_pt_center_new;
  pt_end_new = my_pt_end_new;
  

  /*
  log << MSG::INFO << "the_g=" << the_g << " sag_y=" << sag_y << endreq;

  log << MSG::INFO << "Center Native       x,y,z = " << pt_center.x() << "," << pt_center.y() << "," << pt_center.z() << endreq;
  log << MSG::INFO << "Center AMDB         x,y,z = " << pt_center_amdb.x() << "," << pt_center_amdb.y() << "," << pt_center_amdb.z() << endreq;
  log << MSG::INFO << "Center AMDB   after x,y,z = " << pt_center_new_amdb.x() << "," << pt_center_new_amdb.y() << "," << pt_center_new_amdb.z() << endreq;
  log << MSG::INFO << "Center Native after x,y,z = " << pt_center_new.x() << "," << pt_center_new.y() << "," << pt_center_new.z() << endreq;

  log << MSG::INFO << "End Native       x,y,z = " << pt_end.x() << "," << pt_end.y() << "," << pt_end.z() << endreq;
  log << MSG::INFO << "End AMDB         x,y,z = " << pt_end_amdb.x() << "," << pt_end_amdb.y() << "," << pt_end_amdb.z() << endreq;
  log << MSG::INFO << "End AMDB   after x,y,z = " << pt_end_new_amdb.x() << "," << pt_end_new_amdb.y() << "," << pt_end_new_amdb.z() << endreq;
  log << MSG::INFO << "End Native after x,y,z = " << pt_end_new.x() << "," << pt_end_new.y() << "," << pt_end_new.z() << endreq;
  */

  // Calculate deformation. Make sure that the wire length stays the same. 
  // Code in positionOnDeformedChamber does not provide this by default. 
  // Break transformation into translation of the wire center and the rotation of the wire
  // Move to the coordinate system originated at the wire center, then rotate the wire, then 
  // move wire center to the new position

  const Amg::Translation3D to_center(-pt_center.x(),
				 -pt_center.y(),
				 -pt_center.z());
//   std::cerr<<"to_center "<<to_center[0][0]<<" "<<to_center[0][1]<<" "<<to_center[0][2]<<std::endl;
//   std::cerr<<"to_center "<<to_center[1][0]<<" "<<to_center[1][1]<<" "<<to_center[1][2]<<std::endl;
//   std::cerr<<"to_center "<<to_center[2][0]<<" "<<to_center[2][1]<<" "<<to_center[2][2]<<std::endl;
  const Amg::Translation3D from_center(pt_center_new.x(),
				   pt_center_new.y(),
				   pt_center_new.z());
//   std::cerr<<"from_center "<<from_center[0][0]<<" "<<from_center[0][1]<<" "<<from_center[0][2]<<std::endl;
//   std::cerr<<"from_center "<<from_center[1][0]<<" "<<from_center[1][1]<<" "<<from_center[1][2]<<std::endl;
//   std::cerr<<"from_center "<<from_center[2][0]<<" "<<from_center[2][1]<<" "<<from_center[2][2]<<std::endl;
  //std::cerr<<"from_center "<<from_center<<std::endl;
  const Amg::Vector3D old_direction(pt_end.x()-pt_center.x(),
				 pt_end.y()-pt_center.y(),
				 pt_end.z()-pt_center.z());
  //  std::cerr<<"old_direction "<<old_direction<<std::endl;
    const Amg::Vector3D new_direction(pt_end_new.x()-pt_center_new.x(),
				 pt_end_new.y()-pt_center_new.y(),
				 pt_end_new.z()-pt_center_new.z());
  //  std::cerr<<"new_direction "<<new_direction<<std::endl;
  const Amg::Vector3D rotation_vector = old_direction.unit().cross(new_direction.unit());
  //  std::cerr<<"rotation_vector "<<rotation_vector <<std::endl;
  const Amg::AngleAxis3D wire_rotation(asin(rotation_vector.mag()),rotation_vector);
//   std::cerr<<"wire_rotation "<<wire_rotation[0][0]<<" "<<wire_rotation[0][1]<<" "<<wire_rotation[0][2]<<std::endl;
//   std::cerr<<"wire_rotation "<<wire_rotation[1][0]<<" "<<wire_rotation[1][1]<<" "<<wire_rotation[1][2]<<std::endl;
//   std::cerr<<"wire_rotation "<<wire_rotation[2][0]<<" "<<wire_rotation[2][1]<<" "<<wire_rotation[2][2]<<std::endl;
  

  transf=new Amg::Transform3D(from_center*wire_rotation*to_center);
  (*m_deformTransfs)[itube] = transf;
  if ( reLog().level() <= MSG::DEBUG ) 
    reLog() << MSG::DEBUG <<"RE "<< idh->show_to_string(identify())
	    <<" created deform-trasf for tLayer, tube "<<tubelayer<<" "<<tube<<" globalIndex = "<<itube<<endreq;

//   std::cerr<<"(*m_deformTransf) "<<(*m_deformTransf)[0][0]<<" "<<(*m_deformTransf)[0][1]<<" "<<(*m_deformTransf)[0][2]<<std::endl;
//   std::cerr<<"(*m_deformTransf) "<<(*m_deformTransf)[1][0]<<" "<<(*m_deformTransf)[1][1]<<" "<<(*m_deformTransf)[1][2]<<std::endl;
//   std::cerr<<"(*m_deformTransf) "<<(*m_deformTransf)[2][0]<<" "<<(*m_deformTransf)[2][1]<<" "<<(*m_deformTransf)[2][2]<<std::endl;
  
  return *transf;
}

Amg::Vector3D
MdtReadoutElement::posOnDefChamStraightWire( const Amg::Vector3D& locAMDBPos, const BLinePar* bLine, const Amg::Vector3D fixedPoint) const
{
  //MsgStream log(m_msgSvc, "MuGM:MdtReadout:posOnDefCh");
  //reLog() << MSG::ERROR << "This method is supposed to compute " 
  //        << "positionOnDeformedChamber from the BLinePar object; "
  //        << "it is not implemented yet!!!" << endreq;
  //return locAMDBPos;

  double height = m_inBarrel ? m_Zsize : m_Rsize;
  double thickness = m_inBarrel ? m_Rsize : m_Zsize;

  return posOnDefChamStraightWire( locAMDBPos, 
				    m_Ssize, m_LongSsize, height, thickness, 
				    bLine->bz(), bLine->bp(), bLine->bn(), 
				    bLine->sp(),bLine->sn(),
				    bLine->tw(),
				    bLine->pg(),
				    bLine->tr(),
				    bLine->eg(),bLine->ep(),bLine->en(), fixedPoint); 

}

Amg::Vector3D
MdtReadoutElement::positionOnDeformedChamber( const Amg::Vector3D& locAMDBPos, const BLinePar* bLine, const Amg::Vector3D fixedPoint) const
{
  //MsgStream log(m_msgSvc, "MuGM:MdtReadout:posOnDefCh");
  //reLog() << MSG::ERROR << "This method is supposed to compute " 
  //        << "positionOnDeformedChamber from the BLinePar object; "
  //        << "it is not implemented yet!!!" << endreq;
  //return locAMDBPos;

  double height = m_inBarrel ? m_Zsize : m_Rsize;
  double thickness = m_inBarrel ? m_Rsize : m_Zsize;

  return positionOnDeformedChamber( locAMDBPos, 
				    m_Ssize, m_LongSsize, height, thickness, 
				    bLine->bz(), bLine->bp(), bLine->bn(), 
				    bLine->sp(),bLine->sn(),
				    bLine->tw(),
				    bLine->pg(),
				    bLine->tr(),
				    bLine->eg(),bLine->ep(),bLine->en(), fixedPoint); 

}

//   //Correspondence to AMDB parameters -TBM
//   //cpl_x is tr "trapezoidal effect"
//   //cpl_y is sy "Longbeam vertical sagitta"
//   //cpl_z is sz "Longbeam horizontal sagitta"
//   //sag_x is ?? "shearing deformation"
//   //sag_y is so,sv "RO crossplate sag, HV crossplate sag"
//   //sag_z is ?? "different long-beam bow for short/long side"
//   //the_g is tw "common twist angle for HV and RO side"
//   //the_c is tw "torsion along tube axis"
//   //the_m is tw "torsion along tube axis"
//   //tem_g is T, "temperature"
//   //tem_c is ev "HV elongation"
//   //tem_m is eo "RO elongation"

//   /*
//     CPM: mask-side cross plate (=readout side in endcap)
//     CPC: CCD-side cross plate (=HV side in endcap)
//     CPL: lens cross plate (=central cross plate)

//     note that nearly all deformation parameter names are meaningless
//   */

Amg::Vector3D
MdtReadoutElement::posOnDefChamStraightWire( const Amg::Vector3D& locAMDBPos, 
					     double width_narrow, double width_wide, double height, double thickness,
					     double /*bz*/, double /*bp*/, double /*bn*/, double sp, double sn, double tw,
					     double /*pg*/, double /*tr*/, double eg, double ep, double en, 
					     const Amg::Vector3D fixedPoint) const
{
  // S.Spagnolo Feb.6, 2011
  // this version do not implement the effects irrelevant for the wire geometry; used to correct the tube transform
  // the wire is, a part from gravitational sagging effect, a straight line.  
  // MDT deformations like tube bow: bz,bp,bn bend the tube while the wire endpoints are not affected 
  //                                 => the wire keeps it's nominal straight line trajectory but it is not concentric to the tube
  //                                 ==> in this function bz, bp and bn are ignored or set to 0
  // MDT deformations that extend/contract the wire longitudinally (while keeping it straight): 
  //                                 delta_s from eg and tr are irrelevant for the tube geometry 
  //                                 and for the wire trajectory => set to 0 here 
  //                                 (should be applied as a correction to the 
  //                                 tube lenght => tube surface bounds 
  //                                 =++>>>> effect in tracking just through the gravitational sagging TOTALLY NEGLIGIBLE=> ignore)
  // pg is irrelevant for tracking purposes and (at least for the endcaps) is applies to the internal bars only, not to the tubes !!! 
  //                                 =++>>>> IGNORE IT 
  // ep,en: bend the tube by moving (differently) the endplugs ===> the wire straight trajectory is moved w.r.t. the nominal one 
  //                                 in addition the tubes keep their nominal position at the center => the wire is not concentric to the tube 
  //                                 delta_s from ep,en must also be considered for the implementation of the realistic tube trajectory 
  //                                 induced by ep,en
  // tw,sp,sn,pg (for deltaT and deltaZ) are geometrical effects, that impact on tracking and keep the wire straight. 


  Amg::Vector3D deformPos(locAMDBPos);

  //NOTE s0,z0,t0 are the coord. in the amdb frame of this point: the origine of the frame can be different than the fixed point for deformations s0mdt,z0mdt,t0mdt 
  //    (always equal to the point at lowest t,z and s=0 of the MDT stack)
  double s0 = locAMDBPos.x();
  double z0 = locAMDBPos.y();
  double t0 = locAMDBPos.z();
#ifdef TESTBLINES
  std::cerr<<"** In posOnDefChamStraightWire - width_narrow, width_wide, length, thickness, "<<width_narrow<<" "<<width_wide<<" "<<height<<" "<<thickness<<" "<<std::endl;
  std::cerr<<"** In posOnDefChamStraightWire - going to correct for B-line the position of Point at "<<s0<<" "<<z0<<" "<<t0<<" in the amdb-szt frame"<<std::endl;
#endif

  double s0mdt = s0; // always I think ! 
  if (fabs(fixedPoint.x())>0.01) s0mdt = s0-fixedPoint.x();
  double z0mdt = z0; // unless in the D section of this station there's a dy diff. from 0 for the innermost MDT multilayer (sometimes in the barrel)
  if (fabs(fixedPoint.y())>0.01) z0mdt = z0-fixedPoint.y();
  double t0mdt = t0; // unless in the D section of this station there's a dz diff. from 0 for the innermost MDT multilayer (often in barrel) 
  if (fabs(fixedPoint.z())>0.01) t0mdt = t0-fixedPoint.z();
  if (z0mdt<0 || t0mdt<0) reLog()<<MSG::WARNING
				 <<"posOnDefChamStraigthWire: correcting the local position of a point outside the mdt station (2 multilayers) volume -- RE "
				 << manager()->mdtIdHelper()->show_to_string(identify())<<endreq
				 <<"local point: szt="<<s0<<" "<<z0<<" "<<t0
				 <<" fixedPoint "<<fixedPoint<<endreq; 
#ifdef TESTBLINES  
  std::cerr<<"** In posOnDefChamStraightWire - correct for offset of B-line fixed point "<<s0mdt<<" "<<z0mdt<<" "<<t0mdt<<" "<<std::endl;
#endif

  double ds = 0.;
  double dz = 0.;
  double dt = 0.;

  double width_actual =width_narrow+(width_wide-width_narrow)*(z0mdt/height);
  double s_rel=s0mdt/(width_actual/2.);
  double z_rel=(z0mdt-height/2.)/(height/2.);
  double t_rel=(t0mdt-thickness/2.)/(thickness/2.);
#ifdef TESTBLINES  
  std::cerr<<"** In posOnDefChamStraightWire - width_actual, s_rel, z_rel, t_rel  "<<width_actual<<" "<<s_rel<<" "<<z_rel<<" "<<t_rel<<std::endl;
#endif
  

  // sp, sn - cross plate sag out of plane
  if( (sp!=0) || (sn!=0) ) {
    double ztmp = z_rel*z_rel-1;
    dt += 0.5*(sp+sn)*ztmp + 0.5*(sp-sn)*ztmp*s_rel;
  }

  // tw     - twist
  if( tw!=0 ) {
    dt -= tw*s_rel*z_rel;
    dz += tw*s_rel*t_rel*thickness/height;
#ifdef TESTBLINES
    std::cerr<<"** In posOnDefChamStraightWire: tw="<<tw<<" dt, dz "<<dt<<" "<<dz<<std::endl;
#endif
  }

  // eg     - global expansion 
  if( eg!=0 ) {
    double egppm = eg/1000.;
    ds += 0.;      
    dz += z0mdt*egppm;
    dt += t0mdt*egppm;
  }

  // ep, en - local expansion
  // 
  // Imporant note: the chamber height and length, as they denoted in Christoph's talk,
  // correspond to thickness and height parameters of this function; 
  //
  
  if( (ep!=0) || (en!=0) ) {
    ep = ep/1000.;
    en = en/1000.;   
      //double phi = 0.5*(ep+en)*s_rel*s_rel + 0.5*(ep-en)*s_rel;
      //ds += s0*phi;
      //dz += z0*phi-0.5*phi*height;
      //dt += t0*phi-0.5*phi*thickness;
      // compute position of wire end at s> and <0 in the szt frame s,z,tEP,N
    //double s_rel2 = s_rel2*s_rel2;
    //double s_rel3 = s_rel2*s_rel;    
    //double phi   =  0.5*(ep+en)*s_rel2 + 0.5*(ep-en)*s_rel;   
    //double delta =  (ep+en)*s_rel3/6. + 0.25*(ep-en)*s_rel2;   
      double phiEP   =  0.5*(ep+en) + 0.5*(ep-en);    
      double deltaEP =  (ep+en)/6.  + 0.25*(ep-en);
      double phiEN   =  0.5*(ep+en) - 0.5*(ep-en);
      double deltaEN = -(ep+en)/6.  + 0.25*(ep-en);
      double sEP =  0.5*width_actual + 0.5*deltaEP*width_actual;
      double tEP = t0mdt*(1.+phiEP) - 0.5*phiEP*thickness; //assuming t0=0
      double zEP = z0mdt*(1.+phiEP) - 0.5*phiEP*height; //assuming z0=0
      double sEN = -0.5*width_actual + 0.5*deltaEN*width_actual;
      double tEN = t0mdt*(1.+phiEN) - 0.5*phiEN*thickness; //assuming t0=0
      double zEN = z0mdt*(1.+phiEN) - 0.5*phiEN*height; //assuming z0=0
      // compute position of wire at generic s coordinate expanded_srel [0,1] range
      // from linear interpolation of s,z,tEP and s,z,tEN 
      //double delta = s_rel*s_rel*s_rel*(ep+en)/6.  + s_rel*s_rel*0.25*(ep-en);
      //double sNew = s0mdt+0.5*delta*width_actual - sEN; /// this would be where the physical point of the wire at s0mdt in the undeformed case is moved by ep,en
      // Jun23rd, 2011 However for the purpose of defining dt and dz for a point on the wire (straight track connecting two deformed end-points) it is more convenient to define sNew in a different way 
      double sNew = (s_rel+1.)*(sEP-sEN)/2.; // i.e. if the original point was at fraction = distance from EN / (EP-EN), sNew keep fraction unchanged !
      double expanded_srel = sNew/(sEP-sEN);
      double localT = tEN + expanded_srel*(tEP-tEN);
      double localZ = zEN + expanded_srel*(zEP-zEN);
      // compute delta w.r.t. nominal position is t and z 
      double localDt = localT-t0mdt;
      double localDz = localZ-z0mdt;
      dt += localDt;
      dz += localDz;      
  }
  

#ifdef TESTBLINES
  std::cerr<<"posOnDefChamStraighWire: ds,z,t = "<<ds<<" "<<dz<<" "<<dt<<std::endl;
#endif

  deformPos[0] = s0+ds;
  deformPos[1] = z0+dz;
  deformPos[2] = t0+dt;

  return deformPos;
}
Amg::Vector3D
MdtReadoutElement::positionOnDeformedChamber( const Amg::Vector3D& locAMDBPos, 
                                              double width_narrow, double width_wide, double height, double thickness,
                                              double bz, double bp, double bn, double sp, double sn, double tw,
                                              double pg, double tr, double eg, double ep, double en, 
					      const Amg::Vector3D fixedPoint) const
{

  // From Ivan Logashenko, Sep.20, 2007:
  //
  // This is a second version of this function, which follows new definition of 
  // deformation parameters, presented by Christoph Amelung at ATLAS Muon Week, Sep.3-5 2007
  // The original talk can be found at http://amelung.web.cern.ch/amelung/talk-atlas-muon-sep07.pdf
  //
  // The input position locAMDBPos is assumed to be in AMDB (szt) frame
  // This changed from the previous version of the function, where the frame was xyz, 
  // obtained from szt by transformations s -> -s and z <-> t. Therefore the method
  // fromIdealToDeformed() has to be changed also - the additional transformation AMDBToCA 
  // can be dropped now.
  //
  // The deformation parameters:
  // 
  // bz     - tube bow in plane
  // bp, bn - tube bow out of plane
  // sp, sn - cross plate sag out of plane
  // tw     - twist
  // pg     - parallelogram
  // tr     - trapezoid
  // eg     - global expansion 
  // ep, en - local expansion
  //
  // The effect of all parameters are implemented here. But only the following deformations:
  //    sp, sn, tw, pg, tr, eg
  // keep the tubes straight. Therefore, when this function is used in fromIdealToDeformed(), 
  // the deformation parameters are have to be filtered out, and only the subset above should be
  // left non-zero. 
  //

  //MsgStream log(m_msgSvc, "MuGM:MdtReadout:posOnDefCh");
  Amg::Vector3D deformPos(locAMDBPos);

  double s0 = locAMDBPos.x();
  double z0 = locAMDBPos.y();
  double t0 = locAMDBPos.z();

  double s0mdt = s0; // always I think ! 
  if (fabs(fixedPoint.x())>0.01) s0mdt = s0-fixedPoint.x();
  double z0mdt = z0; // unless in the D section of this station there's a dy diff. from 0 for the innermost MDT multilayer (sometimes in the barrel)
  if (fabs(fixedPoint.y())>0.01) z0mdt = z0-fixedPoint.y();
  double t0mdt = t0; // unless in the D section of this station there's a dz diff. from 0 for the innermost MDT multilayer (often in barrel) 
  if (fabs(fixedPoint.z())>0.01) t0mdt = t0-fixedPoint.z();
  if (z0mdt<0 || t0mdt<0) reLog()<<MSG::WARNING
				 <<"positionOnDeformedChamber: correcting the local position of a point outside the mdt station (2 multilayers) volume -- RE "
				 << manager()->mdtIdHelper()->show_to_string(identify())<<endreq
				 <<"local point: szt="<<s0<<" "<<z0<<" "<<t0
				 <<" fixedPoint "<<fixedPoint<<endreq; 


  double ds = 0;
  double dz = 0;
  double dt = 0;

  double width_actual =width_narrow+(width_wide-width_narrow)*(z0mdt/height);
  double s_rel=s0mdt/(width_actual/2.);
  double z_rel=(z0mdt-height/2.)/(height/2.);
  double t_rel=(t0mdt-thickness/2.)/(thickness/2.);

  // bz     - tube bow in plane 

  if( bz!=0 ) {
    dz += bz*(s_rel*s_rel-1);
  }

  // bp, bn - tube bow out of plane

  if( (bp!=0) || (bn!=0) ) {
    double stmp = s_rel*s_rel-1;
    dt += 0.5*(bp+bn)*stmp + 0.5*(bp-bn)*stmp*z_rel;
  }

  // sp, sn - cross plate sag out of plane

  if( (sp!=0) || (sn!=0) ) {
    double ztmp = z_rel*z_rel-1;
    dt += 0.5*(sp+sn)*ztmp + 0.5*(sp-sn)*ztmp*s_rel;
  }

  // tw     - twist

  if( tw!=0 ) {
    dt -= tw*s_rel*z_rel;
    dz += tw*s_rel*t_rel*thickness/height;
  }

  // pg     - parallelogram

  if( pg!=0 ) {
    ds -= pg*z_rel;
  }

  // tr     - trapezoid
  
  if( tr!=0 ) {
    ds -= tr*s_rel*z_rel;
  }

  // eg     - global expansion 

  if( eg!=0 ) {
    ds += s0mdt*eg;
    dz += z0mdt*eg;
    dt += t0mdt*eg;
  }

  // ep, en - local expansion
  // 
  // Imporant note: the chamber height and length, as they denoted in Christoph's talk,
  // correspond to thickness and height parameters of this function; 
  //
  
  if( (ep!=0) || (en!=0) ) {
    double phi   = 0.5*(ep+en)*s_rel*s_rel + 0.5*(ep-en)*s_rel;
    double delta = (ep+en)*s_rel*s_rel*s_rel/6. + 0.25*(ep-en)*s_rel*s_rel;
    ds += 0.5*delta*width_actual; // SS: change implementation Feb6th 2011 according to FB and CA documentation
    dz += z0mdt*phi-0.5*phi*height;
    dt += t0mdt*phi-0.5*phi*thickness;
  }
  
  deformPos[0] = s0+ds;
  deformPos[1] = z0+dz;
  deformPos[2] = t0+dt;

  return deformPos;
}

Amg::Vector3D  
MdtReadoutElement::afterAsBuiltParamsInAmdbFrame(const Amg::Vector3D& locAMDBPos, int /*multilayer*/, int /*tubelayer*/, int /*tube*/) const
{
   
  double ds = 0.;
  double dz = 0.;
  double dt = 0.; 

  if  ( manager()->applyMdtAsBuiltParams() )  
    {
      // compute ds, dz, dt for asBuiltParams
    }
  else 
    {
      // do nothing here 
    }
  
   
  Amg::Vector3D deformPos(locAMDBPos.x()+ds, locAMDBPos.y()+dz, locAMDBPos.z()+dt);
   
  return deformPos;
}


Amg::Vector3D  
MdtReadoutElement::afterAsBuiltParamsInAmdbFrameEIEM(const Amg::Vector3D& locAMDBPos, int multilayer, int tubelayer, int /*tube*/) const
{
 
  double ds = 0.;
  double dz = 0.;
  double dt = 0.;
 
  //adding here hack for as built parameters ############
  // C. Amelung: if you want to do it absolutely correct, you have to move ML1 (i.e. the multilayer closest to the IP) of EIS/EMS towards the IP, and ML2 of EIL/EML away from the IP, and keep the other multilayer fixed, respectively. This is because the multilayers on which alignment sensors are mounted should remain fixed in space, and the multilayers without sensors should move.
  // ad hoc fix dy_pitch = 26.018-26.011mm; Delta_y=51microns
  if ((getStationName().substr(0,3)=="EIS") ||(getStationName().substr(0,3)=="EMS")) 
    {
      // first subtract the extra y pitch for the current tube layer (i.e. (tubeLayer-1)*dypitch)
      double dypitch = 26.018-26.011;
      dt -= (m_nlayers-tubelayer)*dypitch;
      // then check if extra shift is needed for the other multilayer 
      if (multilayer==1)
	{
 	  // add extra Delta_y + (n_layers-1)*dypitch from the first multilayer 
	  double Delta_y = 0.051;
	  dt -= (m_nlayers-1)*dypitch;
	  dt -= Delta_y;	  
	}
    }
  else if ((getStationName().substr(0,3)=="EIL") ||(getStationName().substr(0,3)=="EML"))
    {
      // first add the extra y pitch for the current tube layer (i.e. (tubeLayer-1)*dypitch)
      double dypitch = 26.018-26.011;
      dt += (tubelayer-1)*dypitch;
      // then check if extra shift is needed for the other multilayer 
      if (multilayer==2)
	{
 	  // add extra Delta_y + (n_layers-1)*dypitch from the first multilayer 
	  double Delta_y = 0.051;
	  dt += (m_nlayers-1)*dypitch;
	  dt += Delta_y;	  
	}
    }
  else 
    {
      // do nothing here 
    }
   
  Amg::Vector3D deformPos(locAMDBPos.x()+ds, locAMDBPos.y()+dz, locAMDBPos.z()+dt);
   
  return deformPos;
}


void  MdtReadoutElement::setIdentifier(Identifier id)
{
    m_id = id;
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    IdentifierHash collIdhash = 0;
    IdentifierHash detIdhash = 0;
    int gethash_code = idh->get_module_hash(id, collIdhash);
    if (gethash_code != 0) 
	reLog()<<MSG::WARNING
	       <<"MdtReadoutElement -- collection hash Id NOT computed for id = "
	       <<idh->show_to_string(id)<<endreq;
    m_idhash = collIdhash;
    gethash_code = idh->get_detectorElement_hash(id, detIdhash);
    if (gethash_code != 0) 
	reLog()<<MSG::WARNING
	       <<"MdtReadoutElement --  detectorElement hash Id NOT computed for id = "
	       <<idh->show_to_string(id)<<endreq;
    m_detectorElIdhash = detIdhash;
}





// **************************** interfaces related to Tracking *****************************************************
const Amg::Transform3D &
MdtReadoutElement::transform(const Identifier & id) const 
{
  const MdtIdHelper* idh = manager()->mdtIdHelper();

#ifndef NDEBUG
  if ( !idh->valid(id) ) { // is input id valid ? 
      reLog()<<MSG::WARNING<<"transform requested to RE "
	     <<idh->show_to_string(identify())<<" named "<<getStationName()
	     <<" for invalid tube id "
	     <<idh->show_to_string(id)<<" computing tranform for first tube of this RE"<<endreq;
      return transform(identify());
  }
  if (!containsId(id)) {
      reLog()<<MSG::WARNING<<"transform requested to RE "
	     <<idh->show_to_string(identify())<<" named "<<getStationName()
	     <<" for not contained tube id "
	     <<idh->show_to_string(id)<<" computing tranform for first tube of this RE"<<endreq;
      return transform(identify());
  }
#endif
  int tubeLayer = idh->tubeLayer(id);
  int tube = idh->tube(id);
  return transform(tubeLayer, tube);
}

const Amg::Transform3D &
MdtReadoutElement::transform(int tubeLayer, int tube) const 
{
    //std::cout<<"caching flag = "<<m_caching<<std::endl;
    int ntot_tubes = m_nlayers * m_ntubesperlayer;
    int itube = (tubeLayer-1)*m_ntubesperlayer + tube - 1;
    if( itube >= ntot_tubes ) {
	reLog() << MSG::WARNING 
		<<" transform called with tubeLayer or tube out of range in chamber " 
		<< manager()->mdtIdHelper()->print_to_string(m_id)
		<< " : layer " << tubeLayer << " max " << m_nlayers 
		<< " tube " << tube << " max " << m_ntubesperlayer 
		<<" will compute transform for first tube in this chamber"<< endreq;
	reLog() <<"Please run in DEBUG mode to get extra diagnostic"<<endreq;
      itube = 0;
    }

    if (m_caching >0)
    {
        if (!m_tubeTransf) m_tubeTransf = new std::vector<Amg::Transform3D *>( ntot_tubes );
        Amg::Transform3D * transfPtr = (*m_tubeTransf)[itube];
        if (!transfPtr) {
            //std::cout<<"a new calculation "<<std::endl;
            transfPtr = new Amg::Transform3D(localToGlobalTransf(tubeLayer, tube));
            (*m_tubeTransf)[itube] = transfPtr;
        }
        //else std::cout<<"use old calculation "<<std::endl;
        return *transfPtr;
    }
    else
    {
        if (!m_tubeTransf) m_tubeTransf = new std::vector<Amg::Transform3D *>;
        //std::cout<<"a new calculation "<<std::endl;
        Amg::Transform3D * transfPtr = new Amg::Transform3D(localToGlobalTransf(tubeLayer, tube));
        m_tubeTransf->push_back(transfPtr);
        return *transfPtr;
    }
}

void MdtReadoutElement::restoreTubes() const
{
  if (0==m_backupTubeCenter || 0==m_backupTubeTransf) return;

  for (int i=0;i<(int)(m_backupTubeCenter->size());i++) {
    if (0!=(*m_backupTubeCenter)[i]) {
      delete (*m_tubeCenter)[i];
      (*m_tubeCenter)[i]=(*m_backupTubeCenter)[i];
    }
  }

  for (int i=0;i<(int)(m_backupTubeTransf->size());i++) {
    if (0!=(*m_backupTubeTransf)[i]) {
      delete (*m_tubeTransf)[i];
      (*m_tubeTransf)[i]=(*m_backupTubeTransf)[i];
    }
  }

  for (int i=0;i<(int)(m_backupDeformTransf->size());i++) {
    if (0!=(*m_backupDeformTransf)[i]) {
      delete (*m_deformTransfs)[i];
      (*m_deformTransfs)[i]=(*m_backupDeformTransf)[i];
    }
  }

  delete m_backupTubeCenter;   m_backupTubeCenter=0;  
  delete m_backupTubeTransf;   m_backupTubeTransf=0;
  delete m_backupDeformTransf; m_backupDeformTransf=0;

  return;
}


void MdtReadoutElement::shiftTube(const Identifier& id) const
{
  const MdtIdHelper* idh = manager()->mdtIdHelper();
  int mlayer    = idh->multilayer(id);
  int tubeLayer = idh->tubeLayer(id);
  int tube      = idh->tube(id);  
  int ntot_tubes = m_nlayers * m_ntubesperlayer;
  
  // check for valid tube
  if ( mlayer != getMultilayer() ||
       tubeLayer < 1 || tubeLayer>getNLayers() ||
       tube < 1      || tube > getNtubesperlayer() ) return;
  
  int itube = (tubeLayer-1)*m_ntubesperlayer + tube - 1;
  
  if (0==m_backupTubeCenter) {
    m_backupTubeCenter = new std::vector<Amg::Vector3D *>( ntot_tubes );
    m_backupTubeTransf = new std::vector<Amg::Transform3D *>( ntot_tubes );   
  }

  if (0==m_backupDeformTransf && m_deformTransfs) {
    unsigned int sizeDeformTransf = m_deformTransfs->size();
    m_backupDeformTransf = new std::vector<Amg::Transform3D *>( sizeDeformTransf );    
  }
  
  // new tube transform (must be done before transform() called)
  if (0==(*m_backupTubeTransf)[itube]) { 
    (*m_backupTubeTransf)[itube]=(*m_tubeTransf)[itube];
    (*m_tubeTransf)[itube]= new Amg::Transform3D(localToGlobalTransf(tubeLayer,tube));
  }

  // new tube center
  if (0==(*m_backupTubeCenter)[itube]) {
    
    (*m_backupTubeCenter)[itube]=(*m_tubeCenter)[itube];
    
    // double wireTension = 350.;
    // if (getStationName().substr(0,3) == "BOL") wireTension = 285.;
    (*m_tubeCenter)[itube]= new Amg::Vector3D(transform(tubeLayer,tube)*Amg::Vector3D(0.,0.,0.));
  }

  // new deformation transform
  if (m_deformTransfs) {
    int jtube=(m_deformTransfs->size()==1) ? 0 : itube;
    if (0==(*m_backupDeformTransf)[jtube]) { 
      (*m_backupDeformTransf)[jtube]=(*m_deformTransfs)[jtube];
      (*m_deformTransfs)[jtube]=0;
    }
  }

  return;
}
  
const Trk::SaggedLineSurface&
MdtReadoutElement::surface (int tubeLayer, int tube) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();
    Identifier id = idh->channelID(idh->parentID(identify()), getMultilayer(), tubeLayer, tube);
    
    int ntot_tubes = m_nlayers * m_ntubesperlayer;
    if (!m_tubeSurfaces) m_tubeSurfaces = new std::vector<Trk::SaggedLineSurface *>( ntot_tubes );
    int itube = (tubeLayer-1)*m_ntubesperlayer + tube - 1;
    // consistency checks 
    if( itube >= ntot_tubes ) {
      reLog() << MSG::WARNING <<"surface called with tubeLayer or tube out of range in chamber " 
	      << manager()->mdtIdHelper()->print_to_string(m_id)
              << " : layer " << tubeLayer << " max " << m_nlayers 
	      << " tube " << tube << " max " << m_ntubesperlayer 
	      <<" will compute surface for first tube in this chamber"<< endreq;
      reLog()<<"Please run in DEBUG mode to get extra diagnostic"<<endreq;
      itube = 0;
    }

    if (m_caching >0)
    {
        if (!m_tubeSurfaces) m_tubeSurfaces = new std::vector<Trk::SaggedLineSurface *>( ntot_tubes );
        Trk::SaggedLineSurface * surfacePtr = (*m_tubeSurfaces)[itube];
        if (!surfacePtr) {
	  double wireTension = 350.;
	  if (getStationName().substr(0,3) == "BOL") wireTension = 285.;
            surfacePtr = new Trk::SaggedLineSurface(*this, id, getWireLength(tubeLayer, tube), wireTension, linearDensity);
            (*m_tubeSurfaces)[itube] = surfacePtr;
        }
        return *surfacePtr;
    }
    else
    {
        if (!m_tubeSurfaces) m_tubeSurfaces = new std::vector<Trk::SaggedLineSurface *>;
	double wireTension = 350.;
	if (getStationName().substr(0,3) == "BOL") wireTension = 285.;
        Trk::SaggedLineSurface * surfacePtr =
            new Trk::SaggedLineSurface(*this, id, getWireLength(tubeLayer, tube), wireTension, linearDensity);
        m_tubeSurfaces->push_back(surfacePtr);
        return *surfacePtr;        
    }
}
const Trk::SaggedLineSurface&
MdtReadoutElement::surface (const Identifier& id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();

#ifndef NDEBUG
  if ( !idh->valid(id) ) { // is input id valid ? 
      reLog()<<MSG::WARNING<<"surface requested to RE "
	     <<idh->show_to_string(identify())<<" named "<<getStationName()
	     <<" for invalid tube id "
	     <<idh->show_to_string(id)<<" computing surface for first tube of this RE"<<endreq;
      return surface(identify());
  }
  if (!containsId(id)) {
      reLog()<<MSG::WARNING<<"surface requested to RE "
	     <<idh->show_to_string(identify())<<" named "<<getStationName()
	     <<" for not contained tube id "
	     <<idh->show_to_string(id)<<" computing surface for first tube of this RE"<<endreq;
      return surface(identify());
  }
#endif
    return surface(idh->tubeLayer(id), idh->tube(id));    
}
const Trk::CylinderBounds  &
MdtReadoutElement::bounds(int tubeLayer, int tube) const
{
  //std::cout<<"in bounds "<<std::endl;
  
    int istep = 0;
    int ntot_steps = m_nsteps;

    if (hasCutouts() && manager()->MinimalGeoFlag() == 0) {
      ntot_steps =  m_nlayers * m_ntubesperlayer;
      istep = (tubeLayer-1)*m_ntubesperlayer + tube - 1;
      //std::cout<<"in bounds for tubeLayer, tube "<<tubeLayer<<" "<<tube<<" istep(since hasCutouts) = "<<istep<<std::endl;
    }
    else
      {
	if (!m_inBarrel) istep = int((tube-1)/m_ntubesinastep);
	//std::cout<<"in bounds for tubeLayer, tube "<<tubeLayer<<" "<<tube<<" istep = "<<istep<<std::endl;

	if( istep<0 || istep >= ntot_steps ) {
	  const MdtIdHelper* idh = manager()->mdtIdHelper();
	  reLog() << MSG::WARNING <<"bounds for Element named " << getStationName() << " with tech. "
		  << getTechnologyType() <<" DEid = "<<idh->show_to_string(identify())<< " called with: tubeL, tube " << tubeLayer << " "
		  << tube << "; step " << istep << " out of range 0-" << m_nsteps-1
		  << " m_ntubesinastep " << m_ntubesinastep <<endreq;
	  reLog()<<"Please run in DEBUG mode to get extra diagnostic; setting istep = 0"<<endreq;
	  istep = 0;
	}
      }

    if (m_caching >0)
    {        
        if (!m_tubeBounds) m_tubeBounds = new std::vector<Trk::CylinderBounds *>( ntot_steps );
        Trk::CylinderBounds * boundsPtr = (*m_tubeBounds)[istep];
        if (!boundsPtr) {
	  //std::cout<<" First time this bound is built: entering getTubeLengthForCaching "<<std::endl; 	
	  double tubelength = getTubeLengthForCaching(tubeLayer, tube);
	  //std::cout<<" in bounds: tubeLength = "<<tubelength<<std::endl;
	  boundsPtr = new Trk::CylinderBounds(innerTubeRadius(), 0.5*tubelength - m_deadlength);
	  //std::cout<<" cylinder length set to 0.5*tubelength - m_deadlength  = "<<0.5*tubelength - m_deadlength<<std::endl;  
	  (*m_tubeBounds)[istep] = boundsPtr;
        }
	else 
	  {
	    //std::cout<<"this bound was already computed .... just get it "<<std::endl;
	  }
        return *boundsPtr;
    }
    else
    {
        if (!m_tubeBounds) m_tubeBounds = new std::vector<Trk::CylinderBounds *>;
	double tubelength = getTubeLengthForCaching(tubeLayer, tube);
        Trk::CylinderBounds * boundsPtr = new Trk::CylinderBounds(innerTubeRadius(), 0.5*tubelength - m_deadlength);
        m_tubeBounds->push_back(boundsPtr);
        return *boundsPtr;
    }
}
const Trk::CylinderBounds  &
MdtReadoutElement::bounds(const Identifier& id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();

#ifndef NDEBUG
  if ( !idh->valid(id) ) { // is input id valid ? 
      reLog()<<MSG::WARNING<<"bounds requested to RE "
	     <<idh->show_to_string(identify())<<" named "<<getStationName()
	     <<" for invalid tube id "
	     <<idh->show_to_string(id)<<" computing bounds for first tube in this mdt RE"<<endreq;
      return bounds(identify());
  }
  if (!containsId(id)) {
      reLog()<<MSG::WARNING<<"bounds requested to RE "
	     <<idh->show_to_string(identify())<<" named "<<getStationName()
	     <<" for not contained tube id "
	     <<idh->show_to_string(id)<<" computing bounds for first tube in this mdt RE"<<endreq;
      return bounds(identify());
  }
#endif
    return bounds(idh->tubeLayer(id), idh->tube(id));
}
const Amg::Vector3D&
MdtReadoutElement::center (const Identifier& id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();

#ifndef NDEBUG
  if ( !idh->valid(id) ) { // is input id valid ? 
      reLog()<<MSG::WARNING<<"center requested to RE "
	     <<idh->show_to_string(identify())<<" named "<<getStationName()
	     <<" for invalid tube id "
	     <<idh->show_to_string(id)<<" computing center for first tube in this mdt RE"<<endreq;
      return center(identify());
  }
  if (!containsId(id)) {
      reLog()<<MSG::WARNING<<"center requested to RE "
	     <<idh->show_to_string(identify())<<" named "<<getStationName()
	     <<" for not contained tube id "
	     <<idh->show_to_string(id)<<" computing center for first tube in this mdt RE"<<endreq;
      return center(identify());
  }
#endif
    return center (idh->tubeLayer(id), idh->tube(id));
}
const Amg::Vector3D&
MdtReadoutElement::center (int tubeLayer, int tube) const
{
    int ntot_tubes = m_nlayers * m_ntubesperlayer;
    int itube = (tubeLayer-1)*m_ntubesperlayer + tube - 1;
    // consistency checks 
    if( itube >= ntot_tubes ) {
      reLog() << MSG::WARNING <<"center called with tubeLayer or tube out of range in chamber " 
	      << manager()->mdtIdHelper()->print_to_string(m_id)
              << " : layer " << tubeLayer << " max " << m_nlayers 
	      << " tube " << tube << " max " << m_ntubesperlayer 
	      <<" will compute center for first tube in this chamber"<< endreq;
      reLog()<<"Please run in DEBUG mode to get extra diagnostic"<<endreq;
      itube = 0;
    }


    if (m_caching>0)
    {
        if (!m_tubeCenter) m_tubeCenter = new std::vector<Amg::Vector3D *>( ntot_tubes );
        Amg::Vector3D * centerPtr = (*m_tubeCenter)[itube];
        if (!centerPtr) {
            centerPtr = new Amg::Vector3D(transform(tubeLayer, tube)*Amg::Vector3D(0.,0.,0.));
            (*m_tubeCenter)[itube] = centerPtr;
        }
        return *centerPtr;
    }
    else
    {
        if (!m_tubeCenter) m_tubeCenter = new std::vector<Amg::Vector3D *>;
        Amg::Vector3D * centerPtr = new Amg::Vector3D(transform(tubeLayer, tube)*Amg::Vector3D(0.,0.,0.));
        m_tubeCenter->push_back(centerPtr);
        return *centerPtr;
    }
}
const Amg::Vector3D&
MdtReadoutElement::normal () const
{
    if (!m_elemNormal)
    {
      m_elemNormal = new Amg::Vector3D( transform().linear()*Amg::Vector3D(1.,0.,0.) );
    }
    return *m_elemNormal;
}

const Amg::Vector3D
MdtReadoutElement::tubeNormal (const Identifier& id) const
{
  const MdtIdHelper* idh = manager()->mdtIdHelper();
  return tubeNormal( idh->tubeLayer(id), idh->tube(id) );
}
const Amg::Vector3D
MdtReadoutElement::tubeNormal (int tubeLayer, int tube) const
{
    return transform(tubeLayer, tube).rotation().col(2);
}

const Trk::Surface&
MdtReadoutElement::surface() const
{
    if (!m_associatedSurface){
      Amg::RotationMatrix3D muonTRotation(transform().rotation());
      Amg::RotationMatrix3D surfaceTRotation;
      surfaceTRotation.col(0) = muonTRotation.col(1);
      surfaceTRotation.col(1) = muonTRotation.col(2);
      surfaceTRotation.col(2) = muonTRotation.col(0);

      Amg::Transform3D trans3D(surfaceTRotation);
      trans3D.pretranslate(transform().translation());

      if (MuonReadoutElement::barrel()){
        m_associatedSurface = new Trk::PlaneSurface(new Amg::Transform3D(trans3D),
                                                    MuonReadoutElement::getSsize()/2.,
                                                    MuonReadoutElement::getZsize()/2.);
      } else {
        m_associatedSurface = new Trk::PlaneSurface(new Amg::Transform3D(trans3D),
                                                    MuonReadoutElement::getSsize()/2.,
                                                    MuonReadoutElement::getLongSsize()/2.,
                                                    MuonReadoutElement::getRsize()/2.);      
      }
    }
    return *m_associatedSurface;
}

const Amg::Vector3D&
MdtReadoutElement::center() const
{
    return surface().center();
}

const Trk::SurfaceBounds&
MdtReadoutElement::bounds() const
{
   if (!m_associatedBounds){
    if (MuonReadoutElement::barrel()){
        m_associatedBounds = new Trk::RectangleBounds(MuonReadoutElement::getSsize()/2.,                                               
                                                     MuonReadoutElement::getZsize()/2.);
      } else {
        m_associatedBounds = new Trk::TrapezoidBounds(MuonReadoutElement::getSsize()/2.,
                                                       MuonReadoutElement::getLongSsize()/2.,
                                                       MuonReadoutElement::getRsize()/2.);      
      }
   }
   return (*m_associatedBounds);
}

void MdtReadoutElement::fillBLineCache() const
{
  if (reLog().level() <= MSG::DEBUG)
        reLog()<<MSG::DEBUG<<"Filling BLine cache for ReadoutElement "<<getStationName()<<"/"<<getTechnologyName()
               <<" eta/phi "<<getStationEta()<<"/"<<getStationPhi()
               <<" ml "<<m_multilayer<<endreq;

  for (int tubeL=1; tubeL<=m_nlayers; ++tubeL){
    for (int tube =1; tube<=m_ntubesperlayer; ++tube)
    {
      fromIdealToDeformed(getMultilayer(), tubeL, tube); 
    }
  }
}
void MdtReadoutElement::clearBLineCache() const
{
  if (reLog().level() <= MSG::VERBOSE)
        reLog()<<MSG::VERBOSE<<"Clearing BLine cache for ReadoutElement "<<getStationName()<<"/"<<getTechnologyName()
               <<" eta/phi "<<getStationEta()<<"/"<<getStationPhi()
               <<" ml "<<m_multilayer<<" pointer to vector or deform-transfs "<<(uintptr_t)m_deformTransfs<<endreq;

    if (m_deformTransfs) {
      if (reLog().level() <= MSG::VERBOSE) 
	reLog()<<MSG::VERBOSE<<"non null pointer to the vector of deform-transfs"<<endreq;
      if (!m_deformTransfs->empty())
	{
	  if (reLog().level() <= MSG::VERBOSE) 
	    reLog()<<MSG::VERBOSE<<"vector of deform-transfs has size "<< m_deformTransfs->size()<<endreq;
	  for (unsigned int tsf = 0; tsf < m_deformTransfs->size(); ++tsf) {
	    if (reLog().level() <= MSG::VERBOSE) 
	      reLog()<<MSG::VERBOSE<<"vector of deform-transfs - going to delete component  "<<tsf<< m_deformTransfs->size()<<endreq;
	    delete (*m_deformTransfs)[tsf];
	    (*m_deformTransfs)[tsf] = 0;
	  }    
	}  
	delete m_deformTransfs;
	m_deformTransfs=0;	
    }
}    

void MdtReadoutElement::clearCache() const
{
  // clear base cache
  MuonReadoutElement::clear();

    //MsgStream log(m_msgSvc, "MuGM:MdtReadout");
    if (reLog().level() <= MSG::DEBUG)
        reLog()<<MSG::DEBUG<<"Clearing cache for ReadoutElement "<<getStationName()<<"/"<<getTechnologyName()
               <<" eta/phi "<<getStationEta()<<"/"<<getStationPhi()
               <<" ml "<<m_multilayer<<endreq;


    if (m_associatedSurface)
    {
        delete m_associatedSurface;
        m_associatedSurface=0;
    }
#ifndef NDEBUG
    else 
      {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"no associated surface to be deleted"<<endreq;}
#endif
    if (m_associatedBounds) {
        delete m_associatedBounds;
        m_associatedBounds =0;
    }
#ifndef NDEBUG
    else 
      {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"no associated bounds to be deleted"<<endreq;}
#endif

    if (m_elemNormal) {
        delete m_elemNormal;
        m_elemNormal       =0;
    }
#ifndef NDEBUG
    else {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"no associated normal to be deleted"<<endreq;}
#endif
    
    if (m_tubeSurfaces) 
    {
        if (!m_tubeSurfaces->empty())
        {        
#ifndef NDEBUG
            if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"size of vector m_tubeSurfaces "<<m_tubeSurfaces->size()<<endreq;
#endif
            for (unsigned int i=0; i<m_tubeSurfaces->size(); ++i) 
            {
                if ( (*m_tubeSurfaces)[i] ) 
                {            
                    delete (*m_tubeSurfaces)[i];
                    (*m_tubeSurfaces)[i] = 0;
                }
#ifndef NDEBUG
                else
		  { 
		    if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"no associated surface at index "<<i<<" to be deleted "<<endreq;
		  }
#endif
            }
#ifndef NDEBUG
            if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"size of vector m_tubeSurfaces "<<m_tubeSurfaces->size()<<endreq;
#endif
            m_tubeSurfaces->clear();
        }
#ifndef NDEBUG
        else 
	  {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"Empty vector m_tubeSurfaces"<<endreq;}
#endif
        delete m_tubeSurfaces;
        m_tubeSurfaces =0;
    }
#ifndef NDEBUG
    else  
      {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"Never allocated memory for m_tubeSurfaces"<<endreq;}
#endif

    if (m_tubeTransf) 
    {
        if (!m_tubeTransf->empty()) 
        {            
            for (unsigned int i=0; i<m_tubeTransf->size(); ++i) 
            {
                if ( (*m_tubeTransf)[i] ) 
                {
                    delete (*m_tubeTransf)[i];
                    (*m_tubeTransf)[i] = 0;
                }
#ifndef NDEBUG
                else 
		  {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"no associated transform at index "<<i<<" to be deleted "<<endreq;}
#endif
            }
#ifndef NDEBUG
            if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"size of vector m_tubeTransf is "<<m_tubeTransf->size()<<endreq;
#endif
            m_tubeTransf->clear();
        }
#ifndef NDEBUG
        else 
	  {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"Empty vector m_tubeTransf"<<endreq;}
#endif
        delete m_tubeTransf;
        m_tubeTransf =0;
    }
#ifndef NDEBUG
    else 
    {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"Never allocated memory for m_tubeTransf"<<endreq;}
#endif

    if (m_tubeCenter)
    {
        if (!m_tubeCenter->empty())
        {
            
            for (unsigned int i=0; i<m_tubeCenter->size(); ++i) 
            {
                if ((*m_tubeCenter)[i]) {
                    delete (*m_tubeCenter)[i];
                    (*m_tubeCenter)[i] = 0;
                }
#ifndef NDEBUG
                else 
		  {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"no associated tube center at index "<<i<<" to be deleted "<<endreq;}
#endif
            }
#ifndef NDEBUG
            if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"size of vector m_tubeCenter is "<<m_tubeCenter->size()<<endreq;
#endif
            m_tubeCenter->clear();
        }
#ifndef NDEBUG
        else 
	  {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"Empty vector m_tubeCenter"<<endreq;}
#endif
        delete m_tubeCenter;
        m_tubeCenter = 0;
    }
#ifndef NDEBUG
    else 
      {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"Never allocated memory for m_tubeCenter"<<endreq;}
#endif

    if (m_tubeBounds)
    {
        if (!m_tubeBounds->empty())
        {
            for (unsigned int i=0; i<m_tubeBounds->size(); ++i) 
            {
                if ((*m_tubeBounds)[i]) 
                {
                    delete (*m_tubeBounds)[i];
                    (*m_tubeBounds)[i] = 0;
                }
#ifndef NDEBUG
                else 
		  {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"no associated tube bounds at index "<<i<<" to be deleted "<<endreq;}
#endif
            }
#ifndef NDEBUG
            if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"size of vector m_tubeBounds is "<<m_tubeBounds->size()<<endreq;
#endif
            m_tubeBounds->clear();
        }
#ifndef NDEBUG
       else 
	 {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"Empty vector m_tubeBounds"<<endreq;}
#endif
        delete m_tubeBounds;
        m_tubeBounds = 0;
    }
#ifndef NDEBUG
    else 
      {if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"Never allocated memory for m_tubeBounds"<<endreq;}
#endif


    // reset here the deform-related transforms
    clearBLineCache();


}
void MdtReadoutElement::setBLinePar(BLinePar* bLine) const
{
  if (reLog().level() <= MSG::DEBUG) reLog()<<MSG::DEBUG<<"Setting B-line for "<<getStationName().substr(0,3)<<" at eta/phi "<<getStationEta()<<"/"<<getStationPhi()<<endreq;
  m_BLinePar = bLine;
}

void MdtReadoutElement::fillCache() const
{
    if (reLog().level() <= MSG::DEBUG)
        reLog()<<MSG::DEBUG<<"Filling cache for ReadoutElement "<<getStationName()<<"/"<<getTechnologyName()
               <<" eta/phi "<<getStationEta()<<"/"<<getStationPhi()
               <<" ml "<<getMultilayer()<<endreq;

    Trk::PlaneSurface* tmpSurface = (Trk::PlaneSurface*)&surface();//<! filling m_associatedSurface
    Trk::SurfaceBounds* tmpBounds = NULL;//<! filling m_associatedBounds
    if (MuonReadoutElement::barrel()) tmpBounds = (Trk::RectangleBounds*)&bounds();
    else tmpBounds = (Trk::TrapezoidBounds*)&bounds();
    if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"global Surface / Bounds pointers "<<tmpSurface<<" "<<tmpBounds<<endreq;
    if ( reLog().level() <= MSG::VERBOSE ) reLog()<<MSG::VERBOSE<<"global Normal "<<normal()<<endreq;

    Trk::CylinderBounds* tmpCil = NULL;
    Trk::SaggedLineSurface* tmpSaggL = NULL;
    Amg::Vector3D myPoint; 
    Amg::Transform3D myTransform;
    for(int tl=1; tl<=getNLayers(); ++tl)
    {
        for (int tube=1; tube<=getNtubesperlayer(); ++tube)
        {
            // print in order to compute !!!
	    myTransform = transform(tl, tube); //<! filling m_tubeTransf
	    myPoint     = center(tl, tube);    //<! filling m_tubeCenter
            tmpCil = (Trk::CylinderBounds*)&bounds(tl, tube); //<! filling m_tubeBounds
            tmpSaggL = (Trk::SaggedLineSurface*)&surface(tl, tube);//<! filling m_tubeSurfaces
	    if ( reLog().level() <= MSG::VERBOSE ) {
	      reLog()<<MSG::VERBOSE<<"tubeLayer/tube "<<tl<<" "<<tube<<" transform at origin  "<<myTransform*Amg::Vector3D(0.,0.,0.)<<endreq;
	      reLog()<<MSG::VERBOSE<<"tubeLayer/tube "<<tl<<" "<<tube<<" tube center          "<<myPoint<<endreq;
	      reLog()<<MSG::VERBOSE<<"tubeLayer/tube "<<tl<<" "<<tube<<" tube bounds pointer  "<<tmpCil<<endreq;
	      reLog()<<MSG::VERBOSE<<"tubeLayer/tube "<<tl<<" "<<tube<<" tube surface pointer "<<tmpSaggL<<endreq;
	    }
        }
    }
}

bool MdtReadoutElement::containsId(Identifier id) const
{
    const MdtIdHelper* idh = manager()->mdtIdHelper();

    int mlayer   = idh->multilayer(id);
    if (mlayer != getMultilayer()) return false;
    
    int layer    = idh->tubeLayer(id);
    if (layer <1 || layer> getNLayers()) return false;

    int tube     = idh->tube(id);
    if (tube< 1 || tube > getNtubesperlayer()) return false;

    return true;
}

// **************************** interfaces related to Tracking *****************************************************)

} // namespace MuonGM
