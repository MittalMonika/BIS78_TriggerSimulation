/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKD3PDMAKER_SCTBYTESTREAMERRORFILLERTOOL_H
#define TRACKD3PDMAKER_SCTBYTESTREAMERRORFILLERTOOL_H
 

#include "D3PDMakerUtils/BlockFillerTool.h"
#include "InDetIdentifier/SCT_ID.h"
#include "SCT_ConditionsTools/ISCT_ByteStreamErrorsTool.h"
#include "SCT_Cabling/ISCT_CablingTool.h"
#include "SCT_Cabling/SCT_OnlineId.h"
#include "xAODEventInfo/EventInfo.h"

#include <vector>
 
 
namespace D3PD {
 
 
/**
 * @brief Block filler tool for byte stream error state from ByteStreamErrorsSvc.
 */
class SCTByteStreamErrorFillerTool
  : public BlockFillerTool<xAOD::EventInfo>
{
public:
   /**
    * @brief Standard Gaudi tool constructor.
    * @param type The name of the tool type.
    * @param name The tool name.
    * @param parent The tool's Gaudi parent.
    */
    SCTByteStreamErrorFillerTool (const std::string& type,
				  const std::string& name,
				  const IInterface* parent);
 
 

    virtual StatusCode initialize() override;

    /// Book variables for this block.
    virtual StatusCode book() override;

 
   /**
    * @brief Fill one block --- type-safe version.
    *
    * This is called once per object.  The caller
    * is responsible for arranging that all the pointers for booked variables
    * are set appropriately upon entry.
    */
    virtual StatusCode fill (const xAOD::EventInfo& p) override;
 
 
   private:

   const SCT_ID*       m_sctid;
   ToolHandle<ISCT_ByteStreamErrorsTool> m_byteStreamErrTool{this, "SCT_ByteStreamErrorsTool", "SCT_ByteStreamErrorsTool", "Tool to retrieve SCT ByteStream Errors"};
   ToolHandle<ISCT_CablingTool> m_cabling{this, "SCT_CablingTool", "SCT_CablingTool", "Tool to retrieve SCT Cabling"};


   int *m_totalNumErrors;
   std::vector<int> *m_scterr_bec;
   std::vector<int> *m_scterr_layer;
   std::vector<int> *m_scterr_eta;
   std::vector<int> *m_scterr_phi;
   std::vector<int> *m_scterr_side;
   std::vector<int> *m_scterr_rodid;
   std::vector<int> *m_scterr_channel;
   std::vector<int> *m_scterr_type;

  };

 
} // namespace D3PD

#endif
