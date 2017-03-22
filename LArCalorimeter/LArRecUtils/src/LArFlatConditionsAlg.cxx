#include "LArFlatConditionsAlg.h" 


LArFlatConditionsAlg::LArFlatConditionsAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_readKey("/LAr/ElecCalibFlat"),
  m_writeKey("LArFlatCond","LArFlatCond"),
  m_condSvc("CondSvc",name)
{
  declareProperty("ReadKey",m_readKey);
  declareProperty("WriteKey",m_writeKey);
}

LArFlatConditionsAlg::~LArFlatConditionsAlg() {};

StatusCode LArFlatConditionsAlg::initialize() {

  // CondSvc
  ATH_CHECK( m_condSvc.retrieve() );
  // Read Handles
  ATH_CHECK( m_readKey.initialize() );
  ATH_CHECK( m_writeKey.initialize() );
  // Register write handle
  if (m_condSvc->regHandle(this, m_writeKey, m_writeKey.dbKey()).isFailure()) {
    ATH_MSG_ERROR("unable to register WriteCondHandle " << m_writeKey.fullKey() << " with CondSvc");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}


StatusCode LArFlatConditionsAlg::execute() {
    
  SG::WriteCondHandle<LArHVScaleCorrFlat> writeHandle{m_writeKey};
  
  if (writeHandle.isValid()) {
    writeHandle.updateStore(); //????
    msg(MSG::WARNING) << "Found valid write handle" << endmsg;
    return StatusCode::SUCCESS;
  }  

  SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey};
  const CondAttrListCollection* attr{*readHandle};

  if (attr==nullptr) {
    msg(MSG::ERROR) << "Failed to retrieve CondAttributeListCollection with key " << m_readKey.key() << endmsg;
    return StatusCode::FAILURE;
  }

  std::unique_ptr<LArHVScaleCorrFlat> flat=std::make_unique<LArHVScaleCorrFlat>(attr);
  
  // Define validity of the output cond obbject and record it
  EventIDRange rangeW;
  if(!readHandle.range(rangeW)) {
    ATH_MSG_ERROR("Failed to retrieve validity range for " << readHandle.key());
    return StatusCode::FAILURE;
  }

  if(writeHandle.record(rangeW,flat.release()).isFailure()) {
    ATH_MSG_ERROR("Could not record LArHVScaleCorr with " << writeHandle.key() 
		  << " with EventRange " << rangeW
		  << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

 
  return StatusCode::SUCCESS;
}

