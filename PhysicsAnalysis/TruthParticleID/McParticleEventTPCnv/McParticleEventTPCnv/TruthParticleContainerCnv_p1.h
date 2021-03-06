///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// TruthParticleContainerCnv_p1.h 
// Header file for class TruthParticleContainerCnv_p1
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef MCPARTICLEEVENTTPCNV_TRUTHPARTICLECONTAINERCNV_P1_H 
#define MCPARTICLEEVENTTPCNV_TRUTHPARTICLECONTAINERCNV_P1_H 

// STL includes

// HepMC / CLHEP includes

// Gaudi includes

// AthenaPoolCnvSvc includes
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// McParticleEvent includes
#define private public
#define protected public
#include "McParticleEvent/TruthParticleContainer.h"
#undef private
#undef protected

// McParticleEventTPCnv includes
#include "McParticleEventTPCnv/TruthParticleContainer_p1.h"

// Forward declaration
class MsgStream;
class StoreGateSvc;
class ITruthParticleCnvTool;

class TruthParticleContainerCnv_p1 : public T_AthenaPoolTPCnvBase<
                                              TruthParticleContainer, 
				              TruthParticleContainer_p1
                                            >  
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /** Default constructor: 
   */
  TruthParticleContainerCnv_p1();

  /** Constructor with parameters: 
   */
  TruthParticleContainerCnv_p1( StoreGateSvc* storeGate,
				ITruthParticleCnvTool* cnvTool );

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /** Method creating the transient representation of @c TruthParticleContainer
   *  from its persistent representation @c TruthParticleContainer_p1
   */
  virtual void persToTrans( const TruthParticleContainer_p1* persObj, 
			    TruthParticleContainer* transObj, 
			    MsgStream &log ) ;

  /** Method creating the persistent representation @c TruthParticleContainer_p1
   *  from its transient representation @c TruthParticleContainer
   */
  virtual void transToPers( const TruthParticleContainer* transObj, 
			    TruthParticleContainer_p1* persObj, 
			    MsgStream &log ) ;

  /////////////////////////////////////////////////////////////////// 
  // Protected method: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  /** Pointer to the StoreGateSvc: we need this as we'll have to fetch
   *  the @c McEventCollection which is the parent collection (the
   *  collection from it has been made) of the transient 
   *  @c TruthParticleContainer has been created.
   */
  StoreGateSvc* m_storeGate;

  /** Pointer to the IAlgTool providing the conversion from a 
   *  @c McEventCollection to a @c TruthParticleContainer
   */
  ITruthParticleCnvTool* m_cnvTool;

}; 

/////////////////////////////////////////////////////////////////// 
/// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline TruthParticleContainerCnv_p1::TruthParticleContainerCnv_p1() :
  m_storeGate ( 0 ),
  m_cnvTool   ( 0 )
{}

inline 
TruthParticleContainerCnv_p1::TruthParticleContainerCnv_p1( StoreGateSvc* storeGate,
							    ITruthParticleCnvTool* cnvTool ) :
  m_storeGate ( storeGate ),
  m_cnvTool   ( cnvTool   )
{}

#endif //> MCPARTICLEEVENTTPCNV_TRUTHPARTICLECONTAINERCNV_P1_H
