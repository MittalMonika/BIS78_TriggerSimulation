/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELDIGITIZATION_FrontEndSimTool_H
#define PIXELDIGITIZATION_FrontEndSimTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandPoisson.h"

#include "SiDigitization/SiChargedDiodeCollection.h"
#include "InDetRawData/InDetRawDataCLASS_DEF.h"

#include "InDetConditionsSummaryService/IInDetConditionsTool.h"
#include "PixelConditionsServices/IPixelCalibSvc.h"
#include "InDetSimEvent/SiTotalCharge.h"

#include "SiDigitization/SiHelper.h"
#include "InDetReadoutGeometry/PixelModuleDesign.h"
#include "InDetReadoutGeometry/SiCellId.h"

#include "CommissionEvent/ComTime.h"

#include "PixelConditionsData/PixelModuleData.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

static const InterfaceID IID_IFrontEndSimTool("FrontEndSimTool", 1, 0);

class FrontEndSimTool:public AthAlgTool,virtual public IAlgTool {

  public:
    FrontEndSimTool( const std::string& type, const std::string& name,const IInterface* parent):
      AthAlgTool(type,name,parent),
      m_pixelCalibSvc("PixelCalibSvc",name),
      m_timeBCN(1),
      m_timeZero(5.0),
      m_timePerBCO(25.0),
      m_comTime(0.0),
      m_useComTime(false),
      m_timeJitter(0.0),
      m_ComTimeKey("ComTime"),
      m_eventStore("StoreGateSvc", name),
      m_BarrelEC(0),
      m_noiseShape({0.0,1.0}),
      m_noiseOccupancy(1e-8),
      m_disableProbability(9e-3)
  {
    declareInterface<FrontEndSimTool>(this);
    declareProperty("PixelCalibSvc",             m_pixelCalibSvc);
	  declareProperty("TimeBCN",                   m_timeBCN,        "Number of BCID");	
	  declareProperty("TimeZero",                  m_timeZero,       "Time zero...?");
	  declareProperty("TimePerBCO",                m_timePerBCO,     "Time per BCO - should be 25ns");
    declareProperty("UseComTime",                m_useComTime,     "Use ComTime for timing");
	  declareProperty("TimeJitter",                m_timeJitter,     "Time jitter");
    declareProperty("BarrelEC",                  m_BarrelEC,       "Index of barrel or endcap");
    declareProperty("NoiseShape",                m_noiseShape,           "Vector containing noise ToT shape");
    declareProperty("NoiseOccupancy",            m_noiseOccupancy);
    declareProperty("DisableProbability",        m_disableProbability);
  }

    static const InterfaceID& interfaceID() { return IID_IFrontEndSimTool; }

    virtual StatusCode initialize() {
      ATH_CHECK(m_pixelConditionsTool.retrieve());

      ATH_CHECK(m_pixelCalibSvc.retrieve());

      ATH_CHECK(m_moduleDataKey.initialize());

      ATH_CHECK(m_ComTimeKey.initialize(m_useComTime));
      if (m_useComTime) {
        SG::ReadHandle<ComTime> comTime(m_ComTimeKey);
        if (comTime.isValid()) {
          m_comTime = comTime->getTime();
          ATH_MSG_DEBUG("Found tool for cosmic/commissioning timing: ComTime");
        } 
        else {
          ATH_MSG_WARNING("Did not find tool needed for cosmic/commissioning timing: ComTime");
        }
      }
      return StatusCode::SUCCESS;
    }

    virtual StatusCode finalize() { return StatusCode::FAILURE; }
    virtual ~FrontEndSimTool() {}
    virtual void process(SiChargedDiodeCollection &chargedDiodes,PixelRDO_Collection &rdoCollection, CLHEP::HepRandomEngine *rndmEngine) = 0;

    void CrossTalk(double crossTalk, SiChargedDiodeCollection &chargedDiodes) const {
      const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());
      SiChargedDiodeMap oldChargedDiodes=chargedDiodes.chargedDiodes();
      for (SiChargedDiodeIterator i_chargedDiode=oldChargedDiodes.begin(); i_chargedDiode!=oldChargedDiodes.end(); ++i_chargedDiode) {
        InDetDD::SiCellId diode=(*i_chargedDiode).second.diode();
        std::vector<InDetDD::SiCellId> neighbours;
        p_design->neighboursOfCell(diode,neighbours);
        for (std::vector<InDetDD::SiCellId>::const_iterator p_neighbour=neighbours.begin(); p_neighbour!=neighbours.end(); ++p_neighbour) {
          const double intersection=p_design->intersectionLength(diode,*p_neighbour);
          // add cross talk only if the intersection is non-zero
          // if the original pixel is at (col,row) then the intersection length is
          // (col+-1, row+-1) : 0 -> diagonal
          // (col   , row+-1) : 0.4 mm (or 0.6 if long pixel) pixel width  = 400um or 600um
          // (col+-1, row   ) : 0.05 mm , pixel height = 50um
          // intersection length is just the length of the contact surface between the two pixels
          if (intersection>0) {
            // create a new charge:
            // Q(new) = Q*L*X
            //   Q = charge of source pixel
            //   L = intersection length [mm]
            //   X = crosstalk factor    [mm-1]
            const SiChargedDiode &siCharge = (*i_chargedDiode).second;
            SiCharge charge(siCharge.charge()*intersection*crossTalk, siCharge.totalCharge().time(), SiCharge::diodeX_Talk, siCharge.totalCharge().particleLink());
            chargedDiodes.add(*p_neighbour,charge);
          }
        }
      }
      return;
    }

    void ThermalNoise(double thermalNoise, SiChargedDiodeCollection &chargedDiodes, CLHEP::HepRandomEngine *rndmEngine) const {
      for (SiChargedDiodeIterator i_chargedDiode=chargedDiodes.begin(); i_chargedDiode!=chargedDiodes.end(); ++i_chargedDiode) {
        SiCharge charge(thermalNoise*CLHEP::RandGaussZiggurat::shoot(rndmEngine),0,SiCharge::noise);
        (*i_chargedDiode).second.add(charge);
      }
      return;
    }

    void RandomNoise(SiChargedDiodeCollection &chargedDiodes, CLHEP::HepRandomEngine *rndmEngine) const {
      const InDetDD::PixelModuleDesign *p_design = static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());
      int nNoise = CLHEP::RandPoisson::shoot(rndmEngine, p_design->numberOfCircuits()*p_design->columnsPerCircuit()*p_design->rowsPerCircuit()*m_noiseOccupancy*static_cast<double>(m_timeBCN)); 
      for (int i=0; i<nNoise; i++) {
        int circuit = CLHEP::RandFlat::shootInt(rndmEngine,p_design->numberOfCircuits());
        int column  = CLHEP::RandFlat::shootInt(rndmEngine,p_design->columnsPerCircuit());
        int row     = CLHEP::RandFlat::shootInt(rndmEngine,p_design->rowsPerCircuit());
        if (row>159 && p_design->getReadoutTechnology()==InDetDD::PixelModuleDesign::FEI3) { row += 8; } // jump over ganged pixels - rowsPerCircuit == 320 above

        InDetDD::SiReadoutCellId roCell(row, p_design->columnsPerCircuit()*circuit+column);
        Identifier noisyID=chargedDiodes.element()->identifierFromCellId(roCell);

        if (roCell.isValid()) {
          InDetDD::SiCellId diodeNoise = roCell;

          double x = CLHEP::RandFlat::shoot(rndmEngine,0.,1.);
          int bin=0;
          for (size_t j=1; j<m_noiseShape.size(); j++) {
            if (x>m_noiseShape[j-1] && x<=m_noiseShape[j]) { bin=j-1; continue; }
          }
          double noiseToTm = bin+1.5;
          double noiseToT = CLHEP::RandGaussZiggurat::shoot(rndmEngine,noiseToTm,1.);

          double chargeShape = m_pixelCalibSvc->getCharge(noisyID,noiseToT);
          chargedDiodes.add(diodeNoise,SiCharge(chargeShape,0,SiCharge::noise));
        }
      }
      return;
    }

    void RandomDisable(SiChargedDiodeCollection &chargedDiodes, CLHEP::HepRandomEngine *rndmEngine) const {
      for (SiChargedDiodeIterator i_chargedDiode=chargedDiodes.begin(); i_chargedDiode!=chargedDiodes.end(); ++i_chargedDiode) {
        if (CLHEP::RandFlat::shoot(rndmEngine)<m_disableProbability) {
          SiHelper::disabled((*i_chargedDiode).second,true,false);
        }
      }
      return;
    }


  private:
    FrontEndSimTool();

  protected:
    ToolHandle<IInDetConditionsTool> m_pixelConditionsTool{this, "PixelConditionsSummaryTool", "PixelConditionsSummaryTool", "Tool to retrieve Pixel Conditions summary"};
    ServiceHandle<IPixelCalibSvc>        m_pixelCalibSvc;

    SG::ReadCondHandleKey<PixelModuleData> m_moduleDataKey{this, "PixelModuleData", "PixelModuleData", "Output key of pixel module"};

    double m_timeBCN;
    double m_timeZero;
    double m_timePerBCO;
    double m_comTime;       /**< cosmics timing ofs */
    bool   m_useComTime;    /**< use ComTime for timing */
    double m_timeJitter; 
    SG::ReadHandleKey<ComTime>   m_ComTimeKey;
    ServiceHandle<StoreGateSvc>  m_eventStore;
    int m_BarrelEC;
    std::vector<double> m_noiseShape;
    double m_noiseOccupancy;
    double m_disableProbability;

    double getG4Time(const SiTotalCharge &totalCharge) const {
      // If there is one single charge, return its time:
      if (totalCharge.chargeComposition().empty()) { return totalCharge.time(); }

      std::list<SiCharge>::const_iterator p_charge=totalCharge.chargeComposition().begin();
      int findfirst = 0;
      SiCharge first = *p_charge;

      // Look for first charge which is not noise
      for ( ; p_charge!=totalCharge.chargeComposition().end() ; p_charge++) {
        if (p_charge->processType()!=SiCharge::noise) { findfirst=1; break; }
      }

      // if all charges were noise, return the time of the highest charge
      if (findfirst == 0) { return totalCharge.time(); }

      // look for the earlist charge among the remaining non-noise charges:
      first = *p_charge;
      p_charge++;

      for ( ; p_charge!=totalCharge.chargeComposition().end() ; p_charge++) {
        if (p_charge->time()<first.time() && p_charge->processType()!=SiCharge::noise) { first=*p_charge; }
      }
      return first.time();
    }

};

#endif // PIXELDIGITIZATION_FrontEndSimTool_H
