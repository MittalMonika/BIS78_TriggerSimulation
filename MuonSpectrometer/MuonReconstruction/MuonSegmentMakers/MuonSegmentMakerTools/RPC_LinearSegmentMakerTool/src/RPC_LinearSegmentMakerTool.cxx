/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "RPC_LinearSegmentMakerTool/RPC_LinearSegmentMakerTool.h"
#include "MuonLinearSegmentMakerUtilities/Fit2D.h"
#include "MuonLinearSegmentMakerUtilities/MuonLinearSegmentMakerUtilities.h"
#include "StoreGate/StoreGateSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "TrkRoad/TrackRoad.h"
#include "MuonRIO_OnTrack/RpcClusterOnTrack.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "MuonSegment/MuonSegment.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

RPC_LinearSegmentMakerTool::RPC_LinearSegmentMakerTool(const std::string& type,
                                                       const std::string& name,
                                                       const IInterface* pIID) :
AlgTool(type, name, pIID),
m_pIdHelper(NULL)
{
    declareInterface<Muon::IMuonSegmentMaker>(this);
    declareProperty("OutlierChi2", m_fExclChi2 = 10.0);
}

StatusCode RPC_LinearSegmentMakerTool::initialize()
{
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "Initializing RPC_LinearSegmentMakerTool - package version " << PACKAGE_VERSION << endreq;
    StatusCode sc = StatusCode::SUCCESS;

    // initialize RpcIdHelper
    StoreGateSvc* pDetStore = NULL;
    sc = svcLoc()->service("DetectorStore", pDetStore);
    if (sc.isFailure() || pDetStore == NULL)
    {
        log << MSG::ERROR << "Cannot locate DetectorStore" << endreq;
        return sc;
    }
    sc = pDetStore->retrieve(m_pMuonMgr);
    if (sc.isFailure() || m_pMuonMgr == NULL)
    {
        log << MSG::ERROR << "Cannot retrieve MuonDetectorManager" << endreq;
        return sc;
    }
    m_pIdHelper = m_pMuonMgr->rpcIdHelper();
    return StatusCode::SUCCESS;
}

StatusCode RPC_LinearSegmentMakerTool::Finalize()
{
    return StatusCode::SUCCESS;
}

static std::string point2String(const Amg::Vector3D& pt)
{
    std::ostringstream oss;
    oss << "eta=" << pt.eta() << ",phi=" << pt.phi() << ",perp=" << pt.perp() << ",z=" << pt.z();
    return oss.str();
}

static std::string dir2String(const Amg::Vector3D& dir)
{
    std::ostringstream oss;
    oss << "eta=" << dir.eta() << ",phi=" << dir.phi() << ",perp=" << dir.perp() << ",z=" << dir.z();
    return oss.str();
}

typedef std::pair<std::string, int> LayerID;

std::vector<const Muon::MuonSegment*>*
RPC_LinearSegmentMakerTool::find(const Trk::TrackRoad& road,
                                 const std::vector< std::vector< const Muon::MdtDriftCircleOnTrack* > >&,
                                 const std::vector< std::vector< const Muon::MuonClusterOnTrack* > >& clusters,
                                 bool,double)
{
    MsgStream log(msgSvc(), name());
    if (log.level() <= MSG::DEBUG)
        log << MSG::DEBUG << "RPC_LinearSegmentMakerTool::find" << endreq;

    Muon::Fit2D::PointArray zPoints, phiPoints;
    std::vector<const Muon::MuonClusterOnTrack*> rios;
    for (std::vector<std::vector<const Muon::MuonClusterOnTrack*> >::const_iterator itClusters = clusters.begin();
        itClusters != clusters.end();
        itClusters++)
    {
        const std::vector<const Muon::MuonClusterOnTrack*>& cluster = *itClusters;
        rios.insert(rios.end(), cluster.begin(), cluster.end());
    }

    const MuonGM::RpcReadoutElement* pReadoutElement =
        dynamic_cast<const MuonGM::RpcReadoutElement*>(rios.front()->detectorElement());
    double baseR = pReadoutElement->center().perp();

    std::set<LayerID> zStations, phiStations;
    int iHit = 0;
    for (std::vector<const Muon::MuonClusterOnTrack*>::const_iterator itHit = rios.begin();
        itHit != rios.end();
        itHit++, iHit++)
    {
        const Muon::MuonClusterOnTrack* pHit = *itHit;
        const Amg::MatrixX & errMat = pHit->localCovariance();
        Amg::MatrixX wheight= errMat.inverse();
        double w = wheight(Trk::locX,Trk::locX) /2.0;
        if (m_pIdHelper->measuresPhi(pHit->identify()))
        {
            double rho = pHit->globalPosition().perp();
            w *= rho * rho;
            Muon::Fit2D::Point* pPt = new Muon::Fit2D::Point(iHit,
                                                             pHit->globalPosition().perp(),
                                                             pHit->globalPosition().phi(),
                                                             w,
                                                             (void*)pHit);
            phiPoints.push_back(pPt);
            const MuonGM::RpcReadoutElement* pReadoutElement =
                dynamic_cast<const MuonGM::RpcReadoutElement*>(pHit->detectorElement());
            phiStations.insert(LayerID(pReadoutElement->getStationName(), pReadoutElement->getDoubletR()));
        }
        else
        {
            Muon::Fit2D::Point* pPt = new Muon::Fit2D::Point(iHit,
                                                             pHit->globalPosition().perp(),
                                                             pHit->globalPosition().z(),
                                                             w,
                                                             (void*)pHit);
            zPoints.push_back(pPt);
            const MuonGM::RpcReadoutElement* pReadoutElement =
                dynamic_cast<const MuonGM::RpcReadoutElement*>(pHit->detectorElement());
            zStations.insert(LayerID(pReadoutElement->getStationName(), pReadoutElement->getDoubletR()));
        }
    }
    std::vector<const Muon::MuonSegment*>* pMuonSegs = NULL;
    double dChi2 = 0.0;
    int nDegf = 0;
    double zCov[2][2], phiCov[2][2];
    memset(zCov, 0, sizeof(zCov));
    memset(phiCov, 0, sizeof(phiCov));

    if (!zPoints.empty() && !phiPoints.empty())
    {
        Muon::Fit2D fit;
        Amg::Vector3D pos;
        Amg::Vector3D dir = road.globalDirection();
        Muon::Fit2D::SimpleStats zSimpleStats, phiSimpleStats;
        Muon::Fit2D::LinStats zLinStats, phiLinStats;
        if (zStations.size() == 1 || zPoints.size() < 4)
        {
//            if (stations.size() == 1 && zPoints.size() >= 4)
//                std::cout << "RPC_LinearSegmentMakerTool::find 1 station with "
//                << zPoints.size() <<" eta points" << std::endl;
            fit.fitPoint(zPoints,   m_fExclChi2, log.level() <= MSG::DEBUG, zSimpleStats);
            fit.fitPoint(phiPoints, m_fExclChi2, log.level() <= MSG::DEBUG, phiSimpleStats);
            if (log.level() <= MSG::DEBUG)
            {
                log << MSG::DEBUG << "  Z: " << zSimpleStats.toString() << endreq;
                log << MSG::DEBUG << "Phi: " << phiSimpleStats.toString() << endreq;
            }
            pos[0]=baseR;
            pos[1]=0.;
            pos[2]=zSimpleStats.fMean;
            Amg::setPhi(pos, phiSimpleStats.fMean);
            dChi2 = zSimpleStats.fChi2 + phiSimpleStats.fChi2;
            nDegf = zSimpleStats.n + phiSimpleStats.n - 2;
            zCov[0][0]=(zSimpleStats.fStd)*(zSimpleStats.fStd);
        }
        else
        {
            double z, zErr, phi, phiErr;
            fit.fitLine(zPoints, m_fExclChi2, log.level() <= MSG::DEBUG, zLinStats);
            if (log.level() <= MSG::DEBUG)
                log << MSG::DEBUG << "Z: " << zLinStats.toString() << endreq;
            zLinStats.eval(baseR, z, zErr);
            dChi2 = zLinStats.fChi2;
            nDegf = zLinStats.n - 2;
            memcpy(zCov, zLinStats.fCov, sizeof(zCov));

            if (phiStations.size() == 1 || phiPoints.size() < 4)
            {
                fit.fitPoint(phiPoints, m_fExclChi2, log.level() <= MSG::DEBUG, phiSimpleStats);
                if (log.level() <= MSG::DEBUG)
                    log << MSG::DEBUG << "Phi: " << phiSimpleStats.toString() << endreq;
                phi = phiSimpleStats.fMean;
                phiLinStats.fSlope = 0.0;
                dChi2 += phiSimpleStats.fChi2;
                nDegf += phiSimpleStats.n - 1;
                phiCov[0][0]=(phiSimpleStats.fStd)*(phiSimpleStats.fStd);

            }
            else
            {
                fit.fitLine(phiPoints, m_fExclChi2, log.level() <= MSG::DEBUG, phiLinStats);
                if (log.level() <= MSG::DEBUG)
                    log << MSG::DEBUG << "Phi: " << phiLinStats.toString() << endreq;
                phiLinStats.eval(baseR, phi, phiErr);
                dChi2 += phiLinStats.fChi2;
                nDegf += phiLinStats.n - 2;
                memcpy(phiCov, phiLinStats.fCov, sizeof(phiCov));

            }
            pos[0]=baseR;
            pos[1]=0.;
            pos[2]=z;
            Amg::setPhi(pos, phi);
            double rho1 = baseR + 1.0;
            zLinStats.eval(rho1, z, zErr);
            Amg::Vector3D p1(rho1, 0.0, z);
            phiLinStats.eval(rho1, phi, phiErr);
            Amg::setPhi(p1, phi);
            dir = (p1 - pos).unit();
        }
        // Intersect segment with surface
        double u =
            (pReadoutElement->normal().dot(pReadoutElement->center() - pos)) /
            (pReadoutElement->normal().dot(dir));
        pos += dir * u;

        if (log.level() <= MSG::DEBUG)
        {
            log << MSG::DEBUG << "Segment position " << point2String(pos) << endreq;
            log << MSG::DEBUG << "Segment direction " << dir2String(dir) << endreq;
        }

        const Trk::Surface& surface = pReadoutElement->surface();
        if (log.level() <= MSG::DEBUG)
        {
//             surface.dump(log);
//             const Amg::Vector3D& pos = surface.center();
            log << MSG::DEBUG << "Surface at "
            << " pos: " << point2String(surface.center())
            << " dir: " << dir2String(surface.normal())
            << endreq;
        }
        const Trk::PlaneSurface* pSurface = dynamic_cast<const Trk::PlaneSurface*>(&surface);
        if (pSurface == NULL)
        {
            log << MSG::WARNING << "Cannot get a PlaneSurface from RpcClusterOnTrack" << endreq;
            goto done;
        }
        Amg::Vector2D pSegPos;
        if (!pSurface->globalToLocal(pos, Amg::Vector3D(0., 0., 0.), pSegPos))
        {
            log << MSG::WARNING << "Cannot get LocalPosition from surface for " << point2String(pos) << endreq;
            goto done;
        }
        if (log.level() <= MSG::DEBUG)
            log << MSG::DEBUG << "Segment local position is"
            << " x=" << pSegPos[Trk::locX]
            << ",y=" << pSegPos[Trk::locY] << endreq;
        Trk::LocalDirection pSegDir;
        pSurface->globalToLocalDirection(dir, pSegDir);
        if (log.level() <= MSG::DEBUG)
            log << MSG::DEBUG << "Segment local direction is"
            << " angleXZ=" << pSegDir.angleXZ()
            << ",angleYZ=" << pSegDir.angleYZ() << endreq;

        AmgSymMatrix(4)  pcov;
        pcov(0,0) = zCov[0][0];
        pcov(0,1) = zCov[0][1];
        pcov(1,1) = zCov[1][1];
        pcov(2,2) = phiCov[0][0];
        pcov(2,3) = phiCov[0][1];
        pcov(3,3) = phiCov[1][1];
        pcov(0,2) = 0.0;
        pcov(0,3) = 0.0;
        pcov(1,2) = 0.0;
        pcov(1,3) = 0.0;


        Trk::FitQuality* pFitQuality = new Trk::FitQuality(dChi2, nDegf);
        DataVector<const Trk::MeasurementBase>* pRios = new DataVector<const Trk::MeasurementBase>;
        for (Muon::Fit2D::PointArray::const_iterator itPt = zPoints.begin(); itPt != zPoints.end(); itPt++)
        {
            Muon::Fit2D::Point* pPt = *itPt;
            if (!pPt->bExclude)
            {
                pRios->push_back(static_cast<const Trk::MeasurementBase*>((const Muon::MuonClusterOnTrack*)(pPt->pData))->clone());
            }
        }
        for (Muon::Fit2D::PointArray::const_iterator itPt = phiPoints.begin(); itPt != phiPoints.end(); itPt++)
        {
            Muon::Fit2D::Point* pPt = *itPt;
            if (!pPt->bExclude)
            {
                pRios->push_back(static_cast<const Trk::MeasurementBase*>((const Muon::MuonClusterOnTrack*)(pPt->pData))->clone());
            }
        }
        Muon::MuonSegment* pMuonSeg = new Muon::MuonSegment(pSegPos,
                                                            pSegDir,
                                                            pcov/*new Trk::ErrorMatrix()*/,
                                                            const_cast<Trk::PlaneSurface*>(pSurface->clone()),
                                                            pRios,
                                                            pFitQuality);
        if (log.level() <= MSG::DEBUG)
//            pMuonSeg->dump(log);
            log << MSG::DEBUG << "Created a new Muon::MuonSegment" << endreq;
        pMuonSegs = new std::vector<const Muon::MuonSegment*>(1);
        (*pMuonSegs)[0] = pMuonSeg;
//        delete pSegDir;
    }
    if (log.level() <= MSG::DEBUG)
        log << MSG::DEBUG << "RPC_LinearSegmentMakerTool::find ended" << endreq;
done:
    for (unsigned iZ = 0; iZ < zPoints.size(); iZ++)
        delete zPoints[iZ];
    for (unsigned iPhi = 0; iPhi < phiPoints.size(); iPhi++)
        delete phiPoints[iPhi];
    return pMuonSegs;
}
