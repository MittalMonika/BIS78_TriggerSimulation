/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ****************************************************************************************
//
// To perform cell per cell pedestal shift correction
//  taking the pedestal shift in MeV from the database /CALO/Pedestal/CellPedestal folder
//
// G.Unal    9 may 2008 ....  first version
//
// ****************************************************************************************

#include "CaloCellPedestalCorr.h"

#include "CaloEvent/CaloCell.h"
#include "CaloDetDescr/CaloDetDescrElement.h"

//=== AttributeList
#include "CoralBase/Blob.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
//=== CaloCondBlod
#include "CaloCondBlobObjs/CaloCondBlobFlt.h"
#include "CaloCondBlobObjs/CaloCondUtils.h"

#include "CaloIdentifier/CaloCell_ID.h"

// ======================================================
// Constructor

CaloCellPedestalCorr::CaloCellPedestalCorr(
			     const std::string& type, 
			     const std::string& name, 
			     const IInterface* parent)
  : CaloCellCorrection(type, name, parent),
    m_caloCoolIdTool("CaloCoolIdTool",this),
    m_cellId(nullptr),
    //m_caloLumiBCIDTool(""),
    m_isMC(false)
{
 declareInterface<CaloCellCorrection>(this);
 declareProperty("Luminosity",m_lumi0=0,"Luminosity in 10**33 units");
 declareProperty("CaloCoolIdTool",m_caloCoolIdTool,"Tool for Calo cool Id");
 declareProperty("FolderName",m_folderName="/CALO/Pedestal/CellPedestal");
 declareProperty("LumiFolderName",m_lumiFolderName="/TRIGGER/LUMI/LBLESTONL");
 //declareProperty("LumiBCIDTool",m_caloLumiBCIDTool,"Tool for BCID pileup offset average correction");
 declareProperty("isMC",m_isMC,"Data/MC flag");
}

//========================================================
// Initialize

StatusCode CaloCellPedestalCorr::initialize()
{
  ATH_MSG_INFO( " in CaloCellPedestalCorr::initialize() "  );

  ATH_CHECK(detStore()->retrieve(m_cellId, "CaloCell_ID"));

  if (m_lumi0<0) {
    if (detStore()->contains<CondAttrListCollection>(m_lumiFolderName)) {
      const DataHandle<CondAttrListCollection> lumiData;
      ATH_CHECK( detStore()->regFcn(&CaloCellPedestalCorr::updateLumi, this , lumiData, m_lumiFolderName, true) );
      ATH_MSG_INFO( " Registered a callback for " << m_lumiFolderName << " Cool folder "  );
    }
    m_lumi0=0;
  }


  if (!m_isMC) {
    //=== Register callback for this data handle
    ATH_CHECK( detStore()->regFcn(&CaloCellPedestalCorr::updateMap, this, m_noiseAttrListColl, m_folderName) );
    ATH_MSG_INFO( " registered a callback for " << m_folderName << " folder "  );

    ATH_CHECK(m_caloCoolIdTool.retrieve());
  }

  //if (!m_caloLumiBCIDTool.empty() ) {
  //  ATH_CHECK(m_caloLumiBCIDTool.retrieve());
  //}

  if (!m_caloBCIDAvg.key().empty()) {
    ATH_CHECK(m_caloBCIDAvg.initialize());
  }
  
  
  ATH_MSG_INFO( "CaloCellPedestalCorr initialize() end"  );

  return StatusCode::SUCCESS;

}

//______________________________________________________________________________________
StatusCode
CaloCellPedestalCorr::updateLumi( IOVSVC_CALLBACK_ARGS )
{
  ATH_MSG_INFO( " in updateLumi() "  );

  const CondAttrListCollection* attrListColl = 0;
  ATH_CHECK(detStore()->retrieve(attrListColl, m_lumiFolderName));
  // Loop over collection
  CondAttrListCollection::const_iterator first = attrListColl->begin();
  CondAttrListCollection::const_iterator last  = attrListColl->end();
  for (; first != last; ++first) {
   if ((*first).first == 0) {
     if (msgLvl(MSG::DEBUG)) {
       std::ostringstream attrStr1;
       (*first).second.toOutputStream( attrStr1 );
       ATH_MSG_DEBUG( "ChanNum " << (*first).first << " Attribute list " << attrStr1.str()  );
     }
     const coral::AttributeList& attrList = (*first).second;
     if (attrList["LBAvInstLumi"].isNull()) {
       ATH_MSG_WARNING( " NULL Luminosity information in database ... set it to 0 "  );
       m_lumi0 = 0.;
     } else {
       m_lumi0 = attrList["LBAvInstLumi"].data<float>() *1e-3;  // luminosity (from 10**30 units in db to 10*33 units)
     }
     break;
   }
  }
  if ( !(m_lumi0 == m_lumi0) ) {
    ATH_MSG_WARNING( " Luminosity not a number ?  m_lumi0 " << m_lumi0 << "   ... set it to 0 "  );
    m_lumi0 = 0.;
  }
  ATH_MSG_INFO( " Luminosity " << m_lumi0  );
  return StatusCode::SUCCESS;
}



// ===============================================================================

StatusCode CaloCellPedestalCorr::updateMap(IOVSVC_CALLBACK_ARGS_K(keys) )
{
  msg(MSG::INFO) << " in updateMap ";
  std::list<std::string>::const_iterator itr;
  for (itr=keys.begin(); itr!=keys.end(); ++itr) {
    msg() << *itr << " ";
  }
  msg() << endmsg;

  //=== loop over collection (all cool channels)
  CondAttrListCollection::const_iterator iColl = m_noiseAttrListColl->begin();
  CondAttrListCollection::const_iterator last  = m_noiseAttrListColl->end();
  for (; iColl != last; ++iColl) {
    
    //=== COOL channel number is system id
    unsigned int sysId = static_cast<unsigned int>(iColl->first);
    
    //=== delete old CaloCondBlobFlt (which does not own the blob)
    NoiseBlobMap_t::iterator iOld = m_noiseBlobMap.find(sysId);
    if(iOld != m_noiseBlobMap.end()){
      delete iOld->second;
    }
    
    //=== Get new CaloCondBlobFlt instance, interpreting current BLOB
    const coral::Blob& blob = (iColl->second)["CaloCondBlob16M"].data<coral::Blob>();
    const CaloCondBlobFlt* flt = CaloCondBlobFlt::getInstance(blob);

    //=== store new pointer in map
    m_noiseBlobMap[sysId] = flt;
    
  }//end iColl
  return StatusCode::SUCCESS;
}

// ============================================================================

void CaloCellPedestalCorr::MakeCorrection (CaloCell* theCell,
                                           const EventContext& ctx) const {


  
  float pedestal=0.;

  if (!m_isMC) {
    const unsigned int cellHash=theCell->caloDDE()->calo_hash();
    unsigned int subHash;
    const unsigned int iCool = m_caloCoolIdTool->getCoolChannelId(cellHash,subHash);
    //std::cout << "Got iCool=" << iCool << " subhash=" << subHash << std::endl;
    NoiseBlobMap_t::const_iterator it = m_noiseBlobMap.find (iCool);
    //The following checks would make sense but were omitted to speed up execution:
    //1. it!=m_noiseBlobMap.end() eg, if iCool exists
    //2. subHash < flt->getNChans()
    const CaloCondBlobFlt* const flt = it->second;
    const unsigned int dbGain = CaloCondUtils::getDbCaloGain(theCell->gain());
    pedestal = flt->getCalib(subHash, dbGain, m_lumi0);
  }

  if (!(m_caloBCIDAvg.key().empty())) {
    SG::ReadHandle<CaloBCIDAverage> bcidavgshift(m_caloBCIDAvg,ctx);
    pedestal = pedestal + bcidavgshift->average(theCell->ID());
  }

  theCell->addEnergy(-pedestal);
}
