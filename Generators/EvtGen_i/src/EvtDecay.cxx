//*****************************************************************************
//
// EvtDecay.cxx
//
// EvtDecay algorithm takes HepMC event generated by Pythia 
// from Storegate and sends all B-hadron weak decays
// to be done in EvtGen, including weak decays of its secondary particles.
// EvtDecay can be used as a TopAlg in conjunction with algorithms Pythia, 
// PythiaB or a SingleParticleGun.
//
// History:
// Original LHCb code by Witold Pokorski and Patric Robbe.
// August 2002: Malte Muller, adopted for ATHENA to work inside algorithm PythiaBModule (only private version within 3.0.0)  
// Sept 2003: James Catmore, adopted for 6.0.3 (not reeased in 6.0.3 ony private version) to work inside PythiaBModule.
// Nov 2003: M.Smizanska,  rewritten a) to be standalone EvtDecay algorithm so it can be combined 
// with any type of Pythia generator b) to match to new LHCb(CDF) code dedicated to LHC, c) to work in 7.3.0.
// EvtDecay first time released in 7.3.0   20.Nov. 2003
// Dec 2003: M.Smizanska: define user's input - decay table 
// Feb 2004 J Catmore, addition of random seed facility. TEMPORARY FIX
// Apr 2006 J Catmore, AtRandmGenSvc facility added to replace above
// Jan 2007 P Reznicek, separated AtRandmGenSvc for EvtGen, code cleanup
//*****************************************************************************

// Header for this module:-
#include "EvtGen_i/EvtDecay.h"
#include "EvtGenBase/EvtVector4R.hh"
#include "EvtGenBase/EvtVector3R.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtParticleFactory.hh"
#include "EvtGen/EvtGen.hh"

// Framework Related Headers:-
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/StoreGateSvc.h"
//#include "StoreGate/DataHandle.h"
#include "GeneratorObjects/McEventCollection.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "AthenaKernel/IAtRndmGenSvc.h"

//#include <string.h>
//#include <stdlib.h>


EvtDecay::EvtDecay(const std::string& name, ISvcLocator* pSvcLocator):Algorithm( name, pSvcLocator ) {

	// These can be used to specify alternative locations or filenames
	// for the EvtGen particle and channel definition files
	
	declareProperty("DecayDecDir", m_DecayDec = "DECAY.DEC");
	declareProperty("PdtTableDir", m_PdtTable = "pdt.table");
	declareProperty("userDecayTableName", m_userDecFileName = "NONE");

	// This is to set polarization for the Lambda_b
	declareProperty("setLambdabSDM", m_PolarizedLambdab = false);
	declareProperty("setLambdabP", m_LambdabPol = 0.0);

	// Random stream name same as for EvtInclusiveDecay
	declareProperty("randomStreamName", m_randomStreamName = "EVTGEN");
	declareProperty("inputKeyName", m_inputKeyName = "GEN_EVENT");

        m_evtAtRndmGen = 0;
	m_targetID = 0;

	m_AtRndmGenSvc = 0;
	m_McEvtColl  = 0;
	m_atRndmGenSvc = 0;
	m_myGen = 0;
	m_sgSvc = 0;
}


StatusCode EvtDecay::initialize() {

	MsgStream log(messageService(), name());
	log << MSG::INFO << "EvtDecay initialize" << endmsg;

	static const bool CREATEIFNOTTHERE(true);
	StatusCode scRndm = service("AtRndmGenSvc", m_AtRndmGenSvc, CREATEIFNOTTHERE);
	if (!scRndm.isSuccess() || 0 == m_AtRndmGenSvc) {
		log << MSG::ERROR << " Could not initialize Random Number Service" << endmsg;
		return scRndm;
	}
	m_evtAtRndmGen = new EvtCLHepRandom(m_AtRndmGenSvc,m_randomStreamName);

	// Open an interface to EvtGen
	m_myGen = new EvtGen(m_DecayDec.c_str(), m_PdtTable.c_str(), m_evtAtRndmGen);
      
	if (m_userDecFileName =="NONE") log << MSG::INFO << "EvtDecay User did not define his Decay table EvtGen will use standart" << endmsg;
	else m_myGen->readUDecay(m_userDecFileName.c_str());

	StatusCode sc = service("StoreGateSvc", m_sgSvc);
	if (sc.isFailure()) {
		log << MSG::ERROR << "Could not find StoreGateSvc" << endmsg;
		return sc;
	}

	return StatusCode::SUCCESS;
}


StatusCode EvtDecay::execute() {
 
	MsgStream log(messageService(), name());
	log << MSG::DEBUG << "EvtDecay executing" << endmsg;

	// Retrieve event from Transient Store (Storegate)
	if ( m_sgSvc->retrieve(m_McEvtColl, m_inputKeyName).isFailure() ) {
		log << MSG::ERROR << "Could not retrieve McEventCollection" << endmsg;
		return StatusCode::FAILURE;
	}
  
	m_targetID = 0;
	McEventCollection::iterator mcItr;
	for( mcItr = m_McEvtColl->begin(); mcItr != m_McEvtColl->end(); mcItr++ ) {

//		std::cout << "Next event -> " << std::endl;
//		std::cout << "EvtDecay: Before 1st Unints conversion -> " << std::endl;
//		(*mcItr)-> print();
    
		// Convert MeV->GeV
		MeVToGeV( (*mcItr) );
//		std::cout << "EvtDecay: After 1st Unints conversion -> " << std::endl;
//		(*mcItr)-> print();       
    
		callEvtGen( (*mcItr) );
    
		// Convert GeV->MeV
		GeVToMeV((*mcItr));
	}
  
	return StatusCode::SUCCESS;
}



// Main procedure, loops over all particles in given event,
// converts them to EvtGenParticles,
// feeds them to EvtGen,
// converts back to HepMC particles and puts them in the event

StatusCode EvtDecay::callEvtGen( HepMC::GenEvent* hepMCevt ) {

	MsgStream log(messageService(), name());

	HepMC::GenEvent::particle_const_iterator itp;
	bool foundGoodB = false;
	for( itp=hepMCevt->particles_begin(); itp!=hepMCevt->particles_end(); ++itp ) {
		   
		// This is the main loop
		// We iterate over particles and we look for ones that should be decayed with EvtGen
		//
		// status == 1     - undecayed Pythia particle (also for particles that are not supposed to decay)
		// status == 999   - particle already decayed by EvtGen
		// status == 899   - particle was supposed to decay by EvtGen - but found no model
		//                   this may be also intentional - such events are then excluded from
		//                   being written into persistent output
	  
		HepMC::GenParticle* hepMCpart=*itp;
		int id = hepMCpart->pdg_id();
		int stat = hepMCpart->status();
//		hepMCpart-> print();
	  
		if ( isGoodB(id, stat) ) {

			foundGoodB = true;
			hepMCpart->set_status(899);
			log << MSG::DEBUG << "Selected good B is " << id <<endmsg;
			EvtId eid=EvtPDL::evtIdFromStdHep(id);

			double en =(hepMCpart->momentum()).e();
			double pex=(hepMCpart->momentum()).px();
			double pey=(hepMCpart->momentum()).py();
			double pez=(hepMCpart->momentum()).pz();

			EvtVector4R p_init(en,pex,pey,pez);

			EvtParticle* part=EvtParticleFactory::particleFactory(eid,p_init);

			if(fabs(id)==5122 && m_PolarizedLambdab) setLambdabSpinDensityMatrix(part,m_LambdabPol);

			m_myGen->generateDecay(part);
			if ( log.level() <= MSG::DEBUG ) part->printTree();
			log << MSG::DEBUG << "Converting particles" << endmsg;

			makeHepMC(part, hepMCevt, hepMCpart);
			if(part->getNDaug()!=0) hepMCpart->set_status(999);  
//			part->printParticle();
			part->deleteTree();
		}
	}

	if ( foundGoodB ) { return StatusCode::SUCCESS; }
	else { return StatusCode::FAILURE; }
}


StatusCode EvtDecay::finalize() {
	MsgStream log(messageService(), name());
	log << MSG::INFO << "EvtDecay finalized" << endmsg;
	return StatusCode::SUCCESS;
}


EvtDecay::~EvtDecay() {
	delete m_myGen;
	delete m_evtAtRndmGen;
}


StatusCode EvtDecay::makeHepMC( EvtParticle* part, HepMC::GenEvent* hEvt, HepMC::GenParticle* hPart ) {  

	MsgStream log(messageService(), name());

	if ( part->getNDaug()!=0 ) {

		double ct=(part->getDaug(0)->get4Pos()).get(0);
		double x =(part->getDaug(0)->get4Pos()).get(1);
		double y =(part->getDaug(0)->get4Pos()).get(2);
		double z =(part->getDaug(0)->get4Pos()).get(3);

		HepMC::GenVertex* end_vtx = new HepMC::GenVertex(CLHEP::HepLorentzVector(x,y,z,ct));
		hEvt->add_vertex( end_vtx );
		end_vtx->add_particle_in(hPart);

		int ndaug=part->getNDaug();

		for( int it=0 ; it<ndaug ; it++ ) {
                
			double e =(part->getDaug(it)->getP4Lab()).get(0);
			double px=(part->getDaug(it)->getP4Lab()).get(1);
			double py=(part->getDaug(it)->getP4Lab()).get(2);
			double pz=(part->getDaug(it)->getP4Lab()).get(3);

			int id=EvtPDL::getStdHep(part->getDaug(it)->getId());
			int status=1;
			if ( part->getDaug(it)->getNDaug()!=0 ) status=999;

			HepMC::GenParticle* prod_part = new HepMC::GenParticle(CLHEP::HepLorentzVector(px,py,pz,e),id,status);
			end_vtx->add_particle_out(prod_part);
			makeHepMC(part->getDaug(it),hEvt,prod_part);
		}
	}

	return StatusCode::SUCCESS;
}


// This returns true if we want the particle with pID to be decayed by EvtGen

bool EvtDecay::isGoodB(const int pID, const int stat ) const {

	switch( m_targetID ) {        

	case 0:
		if( (stat == 1) &&
		   ((abs(pID) == 511) || 
		    (abs(pID) == 521) ||
		    (abs(pID) == 531) ||
		    (abs(pID) == 541) || 
		    (abs(pID) == 5122) ||
		    (abs(pID) == 5132) ||
		    (abs(pID) == 5232) ||
		    (abs(pID) == 5112) ||
		    (abs(pID) == 5212) ||
		    (abs(pID) == 5222) ))
			{ return true; }
		else	{ return false; }
		break;

	case 511:
		if( (stat == 1) && pID == 511 )
			{ return true; }
		else	{ return false; }
		break;

	case -511:
		if( (stat == 1) && pID == -511 )
			{ return true; }
		else	{ return false; }
		break;

	case 521:
		if( (stat == 1) && pID == 521 )
			{ return true; }
		else	{ return false; }
		break;

	case -521:
		if( (stat == 1) && pID == -521 )
			{ return true; }
		else	{ return false; }
		break;
 
	case 531:
		if( (stat == 1) && pID == 531 )
			{ return true; }
		else	{ return false; }
		break;

	case -531:
		if( (stat == 1) && pID == -531 )
			{ return true; }
		else	{ return false; }
		break;

	case 541:
		if( (stat == 1) && pID == 541 )
			{ return true; }
		else	{ return false; }
		break;
      
	case -541:
		if( (stat == 1) && pID == -541 )
			{ return true; }
		else	{ return false; }
		break;

	case 5122:
		if( (stat == 1) && pID == 5122 )
			{ return true; }
		else	{ return false; }
		break;

        case 5132:
		if( (stat == 1) && pID == 5132 )
			{ return true; }
		else	{ return false; }
		break;

	case 5232:
		if( (stat == 1) && pID == 5232 )
			{ return true; }
		else	{ return false; }
		break;

	case 5112:
		if( (stat == 1) && pID == 5112 )
			{ return true; }
		else	{ return false; }
		break;

        case 5212:
		if( (stat == 1) && pID == 5212 )
			{ return true; }
		else	{ return false; }
		break;

        case 5222:
		if( (stat == 1) && pID == 5222 )
			{ return true; }
		else	{ return false; }
		break;

        case -5122:
		if( (stat == 1) && pID == -5122 )
			{ return true; }
		else	{ return false; }
		break;

        case -5132:
		if( (stat == 1) && pID == -5132 )
			{ return true; }
		else	{ return false; }
		break;

        case -5232:
		if( (stat == 1) && pID == -5232 )
			{ return true; }
		else	{ return false; }
		break;

        case -5112:
		if( (stat == 1) && pID == -5112 )
			{ return true; }
		else	{ return false; }
		break;

        case -5212:
		if( (stat == 1) && pID == -5212 )
			{ return true; }
		else	{ return false; }
		break;

        case -5222:
		if( (stat == 1) && pID == -5222 )
			{ return true; }
		else	{ return false; }
		break;

	default:
		return true;
	}
}


// EvtCLHepRandom class implementation, basically CLHEP -> EvtGen random engine interface

EvtCLHepRandom::EvtCLHepRandom(IAtRndmGenSvc* atRndmGenSvc, string streamName)
  : m_atRndmGenSvc(atRndmGenSvc)
  , m_streamName(streamName)
{}

EvtCLHepRandom::~EvtCLHepRandom()
{}

double EvtCLHepRandom::random() {
  CLHEP::HepRandomEngine* engine = m_atRndmGenSvc->GetEngine(m_streamName);
  return CLHEP::RandFlat::shoot(engine);
}


void EvtDecay::MeVToGeV (HepMC::GenEvent* evt) {
	for ( HepMC::GenEvent::particle_iterator p = evt->particles_begin(); p != evt->particles_end(); ++p ) {
//		std::cout << " PDG, BAR " << (*p)->pdg_id() << " " << (*p)->barcode() << std::endl;
	        HepMC::FourVector newMomentum(0.,0.,0.,0.);
		newMomentum.setPx( (*p)->momentum().px() / 1000. );
		newMomentum.setPy( (*p)->momentum().py() / 1000. );
		newMomentum.setPz( (*p)->momentum().pz() / 1000. );
		newMomentum.setE( (*p)->momentum().e() / 1000. );
		(*p)->set_momentum(newMomentum);
	}
}

void EvtDecay::GeVToMeV (HepMC::GenEvent* evt) {
	for ( HepMC::GenEvent::particle_iterator p = evt->particles_begin(); p != evt->particles_end(); ++p ) {
//		std::cout << " PDG, BAR " << (*p)->pdg_id() << " " << (*p)->barcode() << std::endl;
       	        HepMC::FourVector newMomentum(0.,0.,0.,0.);
		newMomentum.setPx( (*p)->momentum().px() * 1000. );
		newMomentum.setPy( (*p)->momentum().py() * 1000. );
		newMomentum.setPz( (*p)->momentum().pz() * 1000. );
		newMomentum.setE( (*p)->momentum().e() * 1000. );
		(*p)->set_momentum(newMomentum);
	}
}
 
 
// Code to setup the Spin Density Matrix of Lambda_b candidates
// Eduard De La Cruz Burelo
// 05/Sept/2006

void EvtDecay::setLambdabSpinDensityMatrix(EvtParticle* part,double Polarization) {

	static EvtId lambdab=EvtPDL::getId("Lambda_b0");

	if(part->getId()==lambdab) { 

		// Set the polarization value
		double pol = Polarization;

		// Calculate the polarization vector
		EvtVector4R pla = part->getP4Lab();
		EvtVector3R prodn = cross( EvtVector3R(0.,0.,1.), EvtVector3R(pla.get(1),pla.get(2),pla.get(3)) );
		EvtVector3R polv = (prodn/prodn.d3mag());
		polv = polv*pol;

		// Calculate the spin density matrix
		EvtSpinDensity rho;
		rho.setDim(2);
		EvtComplex rho00 = EvtComplex(1.+polv.get(2),0.);
		EvtComplex rho01 = EvtComplex(polv.get(0),-polv.get(1));
		EvtComplex rho10 = EvtComplex(polv.get(0),polv.get(1));
		EvtComplex rho11 = EvtComplex(1.-polv.get(2),0.);
		rho.set(0,0,rho00);
		rho.set(0,1,rho01);
		rho.set(1,0,rho10);
		rho.set(1,1,rho11);

		// set the spin density matrix for a moving particle
		part->setSpinDensityForwardHelicityBasis(rho);            
	}
}

