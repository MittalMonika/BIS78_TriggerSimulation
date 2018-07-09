/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef LARCELLREC_LARCELLHVCORR_H
#define LARCELLREC_LARCELLHVCORR_H


#include "CaloUtils/CaloCellCorrection.h"
#include "GaudiKernel/ToolHandle.h"
#include "LArElecCalib/ILArHVCorrTool.h"
#include "LArElecCalib/ILArHVScaleCorr.h"
#include "LArElecCalib/ILArCellHVCorrTool.h"
#include "StoreGate/DataHandle.h"  

class CaloCell;

/** @class LArCellHVCorr
    @brief Tool to re-scale cell energies using HVCorr tool

To invoke this tool:
CaloCellMaker.CaloCellMakerToolNames+=["CaloCellContainerCorrectorTool"]
ToolSvc.CaloCellContainerCorrectorTool.CellCorrectionToolNames+=["LArCellHVCorr"]
ToolSvc.CaloCellContainerCorrectorTool.CaloNums=[0] #LAr EM only

*/

class LArCellHVCorr : public CaloCellCorrection, virtual public ILArCellHVCorrTool {

public:
  
  LArCellHVCorr (const std::string& type, const std::string& name, 
		   const IInterface* parent);

  ~LArCellHVCorr();
  virtual StatusCode initialize() override;


  //Implements the CaloCellCorrection interface    
  virtual void MakeCorrection (CaloCell* theCell,
                               const EventContext& ctx) const override;

  //Implements the ILArCellHVCorrTool interface
  virtual float getCorrection(const Identifier id) override;

  StatusCode LoadCalibration(IOVSVC_CALLBACK_ARGS);

  bool updateOnLastCallback() {return m_updateOnLastCallback;}

 private: 
  float getCorrection(const Identifier id) const;

 ToolHandle<ILArHVCorrTool> m_hvCorrTool;
 std::string m_keyHVScaleCorr;
 bool m_undoHVonline;
 bool m_updateOnLastCallback;
 const DataHandle<ILArHVScaleCorr> m_dd_HVScaleCorr;

};

#endif     
