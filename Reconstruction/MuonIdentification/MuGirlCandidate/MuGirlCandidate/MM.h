/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUGIRL_MM_H
#define MUGIRL_MM_H

#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonPrepRawData/MMPrepDataContainer.h"   // needed because MMPrepDataContainer is not included bt the MuonPrepDataContainer.h

#include "MuGirlCandidate/Technology.h"

namespace MuGirlNS
{

    class MM : public Technology
    {
    public:
        MM(CandidateTool* pMuGirl, const std::string& sPrepDataCollection);
        virtual ~MM();
        virtual const MuonGM::MuonReadoutElement* readoutElement(const Identifier& id) const;
        virtual int stationID(const Identifier& id) const;
        virtual int stationNameID(const std::string& name) const;
        virtual StatusCode retrievePrepData();
        virtual unsigned prepData(Chamber* pChamber, PrepDataList& array);
        virtual Amg::Vector3D hitPosition(const Trk::PrepRawData* pPrepData);
        virtual bool isEtaHit(const Trk::PrepRawData* pPrepData);
        virtual void buildSegments(Candidate* pCand, ChamberList& chambers, double dQOverP);

        virtual std::vector<IdentifierHash> retrieveAvailableCollections() const;

    private:
        const Muon::MMPrepDataContainer* m_pPrepDataContainer;    /**< The PrepData container */
    };
   
    inline MM::~MM()
    {
    }

}

#endif //MUGIRL_MDT_H

