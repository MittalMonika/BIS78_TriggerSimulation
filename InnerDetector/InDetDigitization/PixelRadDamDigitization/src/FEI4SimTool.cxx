/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "FEI4SimTool.h"

#include "InDetReadoutGeometry/PixelModuleDesign.h"

#include "SiDigitization/SiHelper.h"
#include "InDetReadoutGeometry/SiReadoutCellId.h"

#include "CLHEP/Random/RandFlat.h"

using namespace RadDam;

FEI4SimTool::FEI4SimTool( const std::string& type, const std::string& name,const IInterface* parent):
  FrontEndSimTool(type,name,parent),
  m_HitDiscConfig(2)
{
  declareProperty("HitDiscConfig", m_HitDiscConfig);
}

FEI4SimTool::~FEI4SimTool() { }

StatusCode FEI4SimTool::initialize() {
  CHECK(FrontEndSimTool::initialize());
  ATH_MSG_DEBUG("FEI4SimTool::initialize()");

  if (m_HitDiscConfig<0 || m_HitDiscConfig>2) {
    ATH_MSG_FATAL("HitDisConfig must be [0-2], but not " << m_HitDiscConfig);
    return StatusCode::FAILURE;
  }
	return StatusCode::SUCCESS;
}

StatusCode FEI4SimTool::finalize() {
  ATH_MSG_DEBUG("FEI4SimTool::finalize()");
	return StatusCode::SUCCESS;
}

void FEI4SimTool::process(SiChargedDiodeCollection &chargedDiodes,PixelRDO_Collection &rdoCollection) {

  const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());
  if (p_design->getReadoutTechnology()!=InDetDD::PixelModuleDesign::FEI4) { return; }

  const PixelID* pixelId = static_cast<const PixelID *>(chargedDiodes.element()->getIdHelper());
  const IdentifierHash moduleHash = pixelId->wafer_hash(chargedDiodes.identify()); // wafer hash
  Identifier moduleID = pixelId->wafer_id(chargedDiodes.element()->identify());

  int maxFEI4SmallHit = 2;
  int overflowToT     = 16;
  if (m_HitDiscConfig==0) { maxFEI4SmallHit=0; overflowToT=14; }
  if (m_HitDiscConfig==1) { maxFEI4SmallHit=1; overflowToT=15; }
  if (m_HitDiscConfig==2) { maxFEI4SmallHit=2; overflowToT=16; }

  int barrel_ec   = pixelId->barrel_ec(chargedDiodes.element()->identify());
  int layerIndex  = pixelId->layer_disk(chargedDiodes.element()->identify());

  SG::ReadCondHandle<PixelChargeCalibCondData> calibData(m_chargeDataKey);

  std::vector<Pixel1RawData*> p_rdo_small_fei4;
  int nSmallHitsFEI4 = 0;
  std::vector<int> row, col;
  const int maxRow = p_design->rowsPerCircuit();
  const int maxCol = p_design->columnsPerCircuit();
  std::vector<std::vector<int>> FEI4Map(maxRow+16,std::vector<int>(maxCol+16));

  for (SiChargedDiodeOrderedIterator i_chargedDiode=chargedDiodes.orderedBegin(); i_chargedDiode!=chargedDiodes.orderedEnd(); ++i_chargedDiode)
  {
    SiChargedDiode& diode = **i_chargedDiode;

    Identifier diodeID = chargedDiodes.getId(diode.diode());
    double charge = diode.charge();

    int circ = m_pixelCabling->getFE(&diodeID,moduleID);
    int type = m_pixelCabling->getPixelType(diodeID);

    // Apply analog threshold, timing simulation
    double th0 = calibData->getAnalogThreshold((int)moduleHash, circ, type);

    double thrand1 = CLHEP::RandGaussZiggurat::shoot(m_rndmEngine);
    double thrand2 = CLHEP::RandGaussZiggurat::shoot(m_rndmEngine);
    double threshold = th0+calibData->getAnalogThresholdSigma((int)moduleHash,circ,type)*thrand1+calibData->getAnalogThresholdNoise((int)moduleHash, circ, type)*thrand2;

    if (charge>threshold) {
      int bunchSim;
      if (diode.totalCharge().fromTrack()) {
        bunchSim = static_cast<int>(floor((getG4Time(diode.totalCharge())+m_timeZero)/m_timePerBCO));
      } 
      else {
        bunchSim = CLHEP::RandFlat::shootInt(m_rndmEngine,m_timeBCN);
      }

      if (bunchSim<0 || bunchSim>m_timeBCN) { SiHelper::belowThreshold(diode,true,true); }
      else                                  { SiHelper::SetBunch(diode,bunchSim); }
    } 
    else {
      SiHelper::belowThreshold(diode,true,true);
    }

    if (barrel_ec==0 && charge<m_BarrelAnalogthreshold.at(layerIndex)) { SiHelper::belowThreshold(diode,true,true); }
    if (barrel_ec!=0 && charge<m_EndcapAnalogthreshold.at(layerIndex)) { SiHelper::belowThreshold(diode,true,true); }

    // charge to ToT conversion
    double tot    = calibData->getToT((int)moduleHash, circ, type, charge);
    double totsig = calibData->getTotRes((int)moduleHash, circ, tot);
    int nToT = static_cast<int>(CLHEP::RandGaussZiggurat::shoot(m_rndmEngine,tot,totsig));

    const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
    if (pixelId->is_dbm(chargedDiodes.element()->identify())) {
      nToT = 8*(charge - 1200. )/(8000. - 1200.);
    }

    if (nToT<1) { nToT=1; }

    // FEI4 HitDiscConfig
    if (nToT==2 && maxFEI4SmallHit==2) { nToT=1; }
    if (nToT>=overflowToT) { nToT=overflowToT; }

    if (barrel_ec==0 && nToT<=m_BarrelToTthreshold.at(layerIndex)) { SiHelper::belowThreshold(diode,true,true); }
    if (barrel_ec!=0 && nToT<=m_EndcapToTthreshold.at(layerIndex)) { SiHelper::belowThreshold(diode,true,true); }

    // Filter events
    if (SiHelper::isMaskOut(diode))  { continue; } 
    if (SiHelper::isDisabled(diode)) { continue; } 

    if (!m_pixelConditionsTool->isActive(moduleHash,diodeID)) {
      SiHelper::disabled(diode,true,true);
      continue;
    }

    int flag  = diode.flag();
    int bunch = (flag>>8)&0xff;

    InDetDD::SiReadoutCellId cellId=diode.getReadoutCell();
    const Identifier id_readout = chargedDiodes.element()->identifierFromCellId(cellId);

    int iirow = cellId.phiIndex();
    int iicol = cellId.etaIndex();
    if (iicol>=maxCol) { iicol=iicol-maxCol; } // FEI4 copy mechanism works per FE.

    // Front-End simulation
    if (bunch>=0 && bunch<m_timeBCN) {
      Pixel1RawData *p_rdo = new Pixel1RawData(id_readout,nToT,bunch,0,bunch);
      if (nToT>maxFEI4SmallHit) {
        rdoCollection.push_back(p_rdo);
        FEI4Map[iirow][iicol] = 2; //Flag for "big hits"
      }
      else {
        p_rdo_small_fei4.push_back(p_rdo);
        row.push_back(iirow);
        col.push_back(iicol);
        FEI4Map[iirow][iicol] = 1; //Flag for low hits
        nSmallHitsFEI4++;
      }
    }
  }

  // Copy mechanism for IBL small hits:
  if (nSmallHitsFEI4>0) {
    bool recorded = false;

    //First case: Record small hits which are in the same Pixel Digital Region than a big hit:
    for (int ismall=0; ismall<nSmallHitsFEI4; ismall++) {
      int rowPDR = row[ismall]/2;
      int colPDR = col[ismall]/2;
      for (int rowBigHit=2*rowPDR; rowBigHit!=2*rowPDR+2 && rowBigHit<maxRow; ++rowBigHit) {
        for (int colBigHit=2*colPDR; colBigHit!=2*colPDR+2 && colBigHit<maxCol; ++colBigHit) {
          ATH_MSG_DEBUG("rowBig = " << rowBigHit << " colBig = " << colBigHit << " Map Content = " << FEI4Map[rowBigHit][colBigHit]);
          if (FEI4Map[rowBigHit][colBigHit]==2 && !recorded) {
            rdoCollection.push_back(p_rdo_small_fei4[ismall]);
            recorded = true;
          }
        }
      }

      // Second case: Record small hits which are phi-neighbours with a big hit:
      if (!recorded && row[ismall]<maxRow-1) {
        if (FEI4Map[row[ismall]+1][col[ismall]]==2) {
          rdoCollection.push_back(p_rdo_small_fei4[ismall]);
          recorded = true;
        }
      }
      if (!recorded && row[ismall]!=0) {
        if (FEI4Map[row[ismall]-1][col[ismall]]==2) {
          rdoCollection.push_back(p_rdo_small_fei4[ismall]);
          recorded = true;
        }
      }
    }
  }

  return;
}


