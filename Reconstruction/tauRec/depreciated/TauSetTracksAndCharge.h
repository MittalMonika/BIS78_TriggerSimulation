/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUREC_TAUSETTRACKSANDCHARGE_H
#define	TAUREC_TAUSETTRACKSANDCHARGE_H

#include "tauRec/TauToolBase.h"
#include "GaudiKernel/ToolHandle.h"

class TauEventData;

/**
 * @brief      Class to set number of tracks and charge of the tau (depreciated!)
 * 
 * @author  Felix Friedrich
 */

class TauSetTracksAndCharge: public TauToolBase
{

    public:

        //-------------------------------------------------------------
        //! Constructor
        //-------------------------------------------------------------
        TauSetTracksAndCharge( const std::string& type,
                const std::string& name,
                const IInterface* parent);

        //-------------------------------------------------------------
        //! Destructor
        //-------------------------------------------------------------
        virtual ~TauSetTracksAndCharge();

        virtual StatusCode execute( TauEventData *data );
        virtual StatusCode initialize();


    private: 
        std::string m_trackContainerName;
};

#endif	/* TAUSETTRACKSANDCHARGE_H */

