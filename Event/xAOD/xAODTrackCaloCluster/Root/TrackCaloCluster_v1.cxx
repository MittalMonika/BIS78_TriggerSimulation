// $Id: TrackCaloCluster_v1.cxx $

// Misc includes
#include <bitset>
#include <vector>

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODTrackCaloCluster/versions/TrackCaloCluster_v1.h"

namespace xAOD {

  TrackCaloCluster_v1::TrackCaloCluster_v1() : IParticle(), m_p4(), m_p4Cached( false ) {}
  
  TrackCaloCluster_v1::~TrackCaloCluster_v1() {}
  
  double TrackCaloCluster_v1::e() const {
    return p4().E();
  }
 
  double TrackCaloCluster_v1::rapidity() const {
    return p4().Rapidity();
  }
   
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST(TrackCaloCluster_v1, float, double, pt)
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST(TrackCaloCluster_v1, float, double, eta)
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST(TrackCaloCluster_v1, float, double, phi)
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST(TrackCaloCluster_v1, float, double, m)
  AUXSTORE_PRIMITIVE_GETTER(TrackCaloCluster_v1,    int, taste)
  
  const TrackCaloCluster_v1::FourMom_t& TrackCaloCluster_v1::p4() const {
    // Check if we need to reset the cached object:     
    if( ! m_p4Cached ) {
      m_p4.SetPtEtaPhiM( pt(), eta(), phi(), m() );       
      m_p4Cached = true;     
    }   
    // Return the cached object:     
    return m_p4;
  }

  Type::ObjectType TrackCaloCluster_v1::type() const { 
     return Type::TrackCaloCluster;
  }

  /// set the 4-vec
  void TrackCaloCluster_v1::setP4(float pt, float eta, float phi, float m) {
    setDefiningParameters(pt,eta,phi,m);
  }
    
  void TrackCaloCluster_v1::setDefiningParameters(float pt, float eta, float phi, float m) {     
    static const Accessor< float > acc1( "pt" );     
    acc1( *this ) = pt;     
    static const  Accessor< float > acc2( "eta" );     
    acc2( *this ) = eta;     
    static const  Accessor< float > acc3( "phi" );     
    acc3( *this ) = phi;     
    static const  Accessor< float > acc4( "m" );     
    acc4( *this ) = m;     
    m_p4Cached = false;     
    return;   
  }
  
  AUXSTORE_OBJECT_SETTER_AND_GETTER( TrackCaloCluster_v1,
				     ElementLink< xAOD::TrackParticleContainer >,
				     trackParticleLink, 
				     setTrackParticleLink)
  
  const xAOD::TrackParticle* TrackCaloCluster_v1::trackParticle() const {
    // The accessor:       
    static SG::AuxElement::Accessor< ElementLink< xAOD::TrackParticleContainer > >  acc( "trackPartcleLink" );
    if( !acc.isAvailable( *this ) ) {
      return 0;       
    }
    if( !acc( *this ).isValid() ) {
      return 0;
    }
    return *(acc( *this ));
  }
  
  AUXSTORE_OBJECT_SETTER_AND_GETTER( TrackCaloCluster_v1, 
				     std::vector< ElementLink< xAOD::CaloClusterContainer > >, 
				     caloClusterLinks, 
				     setCaloClusterLinks)
  
  void TrackCaloCluster_v1::setParameters(float pt, float eta, float phi, float m, TrackCaloCluster_v1::Taste taste,
					  const ElementLink<xAOD::TrackParticleContainer> particle, 
					  const std::vector< ElementLink< xAOD::CaloClusterContainer > > clusters) {
    setP4(pt, eta, phi, m);
    
    static Accessor< int > acc( "taste" );     
    acc( *this ) = (int)taste;
    
    setTrackParticleLink(particle);
    setCaloClusterLinks(clusters);
    
  }
  


} // namespace xAOD