/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: TrigMissingETCnvTool.cxx 592539 2014-04-11 10:17:29Z krasznaa $

// System include(s):
#include <vector>

// EDM include(s):
#include "xAODTrigMissingET/TrigMissingETContainer.h"
#include "xAODTrigMissingET/TrigMissingET.h"
#include "TrigMissingEtEvent/TrigMissingETContainer.h"
#include "TrigMissingEtEvent/TrigMissingET.h"

// Local include(s):
#include "TrigMissingETCnvTool.h"

namespace xAODMaker {

  TrigMissingETCnvTool::TrigMissingETCnvTool( const std::string& type,
					      const std::string& name,
					      const IInterface* parent )
    : AthAlgTool( type, name, parent ) {

    // Declare the interface(s) provided by the tool:
    declareInterface< ITrigMissingETCnvTool >( this );
  }

  StatusCode TrigMissingETCnvTool::initialize() {

    // Greet the user:
    ATH_MSG_INFO( "Initializing - Package version: " << PACKAGE_VERSION );

    // Return gracefully:
    return StatusCode::SUCCESS;
  }

  /**
   * This is where the translation actually happens.
   *
   * @param aod The AOD object to take the muon RoIs from
   * @param xaod The xAOD container to fill the RoIs into
   * @returns <code>StatusCode::SUCCESS</code> if all went fine,
   *          something else if not
   */
  StatusCode
  TrigMissingETCnvTool::convert( const TrigMissingETContainer* aod,
                                 xAOD::TrigMissingETContainer* xaod ) {

    // A small sanity check. The output container should really be empty...
    if( xaod->size() ) {
      ATH_MSG_WARNING( "The output xAOD container is not empty (size=="
                       << xaod->size() << ")" );
    }

    TrigMissingETContainer::const_iterator aod_itr = aod->begin();
    TrigMissingETContainer::const_iterator aod_end = aod->end();
    for( ; aod_itr != aod_end; ++aod_itr ) {

       // Create a (single) output xAOD object.
       auto xMET = new xAOD::TrigMissingET();
       xaod->push_back(xMET);

       // Transfer the simple items
       xMET->setEx( ( *aod_itr )->ex() );
       xMET->setEy( ( *aod_itr )->ey() );
       // ...

       // Get the list of components
       unsigned int nComp ( ( *aod_itr )->getNumOfComponents());
       std::vector< std::string > componentNames;
       for (unsigned int i = 0; i < nComp; i++) {
          componentNames.push_back( ( *aod_itr )->getNameOfComponent( i ) );
       }
       xMET->defineComponents( componentNames );
       for (unsigned int i = 0; i < nComp; i++) {
          xMET->setExComponent( i, ( *aod_itr )->getExComponent( i ) );
       }

    }

    // Return gracefully:
    return StatusCode::SUCCESS;
  }

} // namespace xAODMaker
