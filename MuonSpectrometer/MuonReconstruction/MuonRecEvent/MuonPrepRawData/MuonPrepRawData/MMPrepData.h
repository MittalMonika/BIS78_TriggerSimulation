/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPREPRAWDATA_MMPREPDATA_H
#define MUONPREPRAWDATA_MMPREPDATA_H

// Base classes
#include "MuonPrepRawData/MuonCluster.h"
#include "TrkSurfaces/Surface.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"

class MMPrepDataContainerCnv;
class IdentifierHash;

namespace Muon 
{

  class MMRdoToPrepDataTool;
    
  /** @brief Class to represent MM measurements. */
  class MMPrepData :   public MuonCluster
  {

    friend class Muon::MMRdoToPrepDataTool;
    
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:

    friend class ::MMPrepDataContainerCnv;

    MMPrepData();
    MMPrepData(const MMPrepData &);
    MMPrepData(MMPrepData &&);
    MMPrepData &operator=(const MMPrepData &);
    MMPrepData &operator=(MMPrepData &&);


    /** @brief Constructor.
	@param RDOId The identifier of the central strip of the cluster
	@param locpos The local coords of the measurement (this object will now own the LocalPostion)
	@param rdoList Vector of all the Identifiers of the strips used in this cluster
	@param locErrMat The error of the measurement (this object will now own the ErrorMatrix)
	@param detEl The pointer to the Detector Element on which this measurement was made (must NOT be zero). Ownership is NOT taken 
	(the pointer is assumed to belong to GeoModel and will not be deleted)
    */
    MMPrepData( const Identifier& RDOId,
		const IdentifierHash &idDE,
		const Amg::Vector2D& locpos,
		const std::vector<Identifier>& rdoList,
		const Amg::MatrixX* locErrMat,
		const MuonGM::MMReadoutElement* detEl);

    /** @full constructor including time and charge */
    MMPrepData( const Identifier& RDOId,
		const IdentifierHash &idDE,
		const Amg::Vector2D& locpos,
		const std::vector<Identifier>& rdoList,
		const Amg::MatrixX* locErrMat,
		const MuonGM::MMReadoutElement* detEl, 
		const int time,
		const int charge );


    /** @brief Destructor: */
    virtual ~MMPrepData();

    /** @brief set microTPC parameters */
    void setMicroTPC(double angle, double chisqProb);

    /** @brief Returns the global position*/
    const Amg::Vector3D& globalPosition() const;

    /** @brief Returns the detector element corresponding to this PRD.
	The pointer will be zero if the det el is not defined (i.e. it was not passed in by the ctor)*/
    const MuonGM::MMReadoutElement* detectorElement() const;

    /** @brief Returns the TDC counts */
    int time() const;

    /** @brief Returns the ADC counts */
    double charge() const;

    /** @brief Returns the microTPC angle */
    double angle() const;

    /** @brief Returns the microTPC chisq Prob. */
    double chisqProb() const;

    /** @brief Dumps information about the PRD*/
    MsgStream&    dump( MsgStream&    stream) const;

    /** @brief Dumps information about the PRD*/
    std::ostream& dump( std::ostream& stream) const;

  private:

    /** @brief Cached pointer to the detector element - should never be zero.*/
    const MuonGM::MMReadoutElement* m_detEl;

    /** @brief measured time */
    int m_time;

    /** @brief measured charge */
    int m_charge;

    /** @angle and chisquare from micro-TPC fit */
    double m_angle;
    double m_chisqProb;

  };

  inline const MuonGM::MMReadoutElement* MMPrepData::detectorElement() const
  {
    return m_detEl;
  }
  // return globalPosition:
  inline const Amg::Vector3D& MMPrepData::globalPosition() const
  {
    if (m_globalPosition==0) m_globalPosition = m_detEl->surface(identify()).Trk::Surface::localToGlobal(localPosition());

    if (m_globalPosition==0) throw Trk::PrepRawDataUndefinedVariable();
    return *m_globalPosition;
  }

  inline int MMPrepData::time() const 
  {
    return m_time;
  }
  
  inline double MMPrepData::charge() const 
  {
    return m_charge;
  }

  inline double MMPrepData::angle() const 
  {
    return m_angle;
  }

  inline double MMPrepData::chisqProb() const 
  {
    return m_chisqProb;
  }

}

#endif // MUONPREPRAWDATA_MMREPDATA_H

