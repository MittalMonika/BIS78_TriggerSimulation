/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @authors: Alaettin Serhan Mete, Hasan Ozturk - alaettin.serhan.mete@cern.ch, haozturk@cern.ch
 */

// Framework includes
#include "GaudiKernel/ThreadLocalContext.h"

// PerfMonComps includes
#include "PerfMonMTSvc.h"
#include "PerfMonUtils.h" // borrow from existing code


using json = nlohmann::json; // for convenience

/*
 * Constructor
 */
PerfMonMTSvc::PerfMonMTSvc( const std::string& name,
                            ISvcLocator* pSvcLocator )
  : AthService( name, pSvcLocator ),
    m_eventCounter{0} {

  m_measurement.capture();
  m_snapshotData[0].addPointStart(m_measurement);

  declareProperty( "doEventLoopMonitoring",
                   m_doEventLoopMonitoring = false,
                   "True if event loop monitoring is enabled, false o/w."
                   "Event loop monitoring may cause a decrease in the performance"
                   "due to the usage of locks" );
 
  declareProperty( "printDetailedTables",
                   m_printDetailedTables = false,
                   "Print detailed component-level metrics" );
}

PerfMonMTSvc::~PerfMonMTSvc(){

}

/*
 * Query Interface
 */
StatusCode PerfMonMTSvc::queryInterface( const InterfaceID& riid,
                                         void** ppvInterface ) {
  if( !ppvInterface ) {
    return StatusCode::FAILURE;
  }

  if ( riid == IPerfMonMTSvc::interfaceID() ) {
    *ppvInterface = static_cast< IPerfMonMTSvc* >( this );
    return StatusCode::SUCCESS;
  }

  return AthService::queryInterface( riid, ppvInterface );
}

/*
 * Initialize the Service
 */
StatusCode PerfMonMTSvc::initialize() {

  // TODO: Define this array as a class member !
  // Name the steps whose snapshots will be captured as a whole
  //const std::string snapshotStepNames[3] = {"Initialize","Event_loop","Finalize"};
  m_snapshotStepNames.push_back("Initialize");
  m_snapshotStepNames.push_back("Event Loop");
  m_snapshotStepNames.push_back("Finalize");

  /// Configure the auditor
  if( !PerfMon::makeAuditor("PerfMonMTAuditor", auditorSvc(), msg()).isSuccess()) {
    ATH_MSG_ERROR("Could not register auditor [PerfMonMTAuditor]!");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

/*
 * Finalize the Service
 */
StatusCode PerfMonMTSvc::finalize(){ 
 
  m_measurement.capture();
  m_snapshotData[2].addPointStop(m_measurement);

  report();
  return StatusCode::SUCCESS;
}

/*
 * Start Auditing
 */
void PerfMonMTSvc::startAud( const std::string& stepName,
                             const std::string& compName ) {  
   /*
   * This if statement is temporary. It will be removed.
   * In current implementation the very first thing called is stopAud function
   * for PerfMonMTSvc. There are some components before it. We miss them.
   * It should be fixed.
   */
  if( compName != "AthenaHiveEventLoopMgr" && compName != "PerfMonMTSvc" ){
    startSnapshotAud(stepName, compName);

    if( !isLoop() ) 
      startCompAud_serial(stepName, compName);
    else if( m_doEventLoopMonitoring == true )
      startCompAud_MT(stepName, compName);
     
  }
}

/*
 * Stop Auditing
 */
void PerfMonMTSvc::stopAud( const std::string& stepName,
                            const std::string& compName ) {
 
  if( compName != "AthenaHiveEventLoopMgr" && compName != "PerfMonMTSvc" ){
    stopSnapshotAud(stepName, compName);

    if( !isLoop() ) 
      stopCompAud_serial(stepName, compName);
    else if( m_doEventLoopMonitoring == true )
      stopCompAud_MT(stepName, compName);

  }
}

void PerfMonMTSvc::startSnapshotAud( const std::string& stepName,
                                     const std::string& compName ) {

  // Last thing to be called before the event loop begins
  if( compName == "AthRegSeq" && stepName == "Start") {
    m_measurement.capture();
    m_snapshotData[1].addPointStart(m_measurement);
  }

  // Last thing to be called before finalize step begins
  if ( compName == "AthMasterSeq" && stepName == "Finalize"){
    m_measurement.capture();
    m_snapshotData[2].addPointStart(m_measurement);
  }

}

void PerfMonMTSvc::stopSnapshotAud( const std::string& stepName,
                                    const std::string& compName ) {

  // First thing to be called after the initialize step ends
  if ( compName == "AthMasterSeq" && stepName == "Initialize"){
    m_measurement.capture();
    m_snapshotData[0].addPointStop(m_measurement);
  }


  // First thing to be called after the event loop ends
  if( compName == "AthMasterSeq" && stepName == "Stop") {
    m_measurement.capture();
    m_snapshotData[1].addPointStop(m_measurement);
  }

}


void PerfMonMTSvc::startCompAud_serial( const std::string& stepName,
                                      const std::string& compName) {

  // Current step - component pair. Ex: Initialize-StoreGateSvc 
  PMonMT::StepComp currentState = generate_serial_state (stepName, compName);
  
  // Capture the time
  m_measurement.capture(); 

  /*
   *  Dynamically create a MeasurementData instance for the current step-component pair
   *  This space will be freed after results are reported.
   */   
  m_compLevelDataMap[currentState] = new PMonMT::MeasurementData;
  m_compLevelDataMap[currentState]->addPointStart(m_measurement);

}

void PerfMonMTSvc::stopCompAud_serial( const std::string& stepName,
                                     const std::string& compName) {

  // Capture the time
  m_measurement.capture();

  // Current step - component pair. Ex: Initialize-StoreGateSvc
  PMonMT::StepComp currentState = generate_serial_state (stepName, compName);

  m_compLevelDataMap[currentState]->addPointStop(m_measurement);
}

void PerfMonMTSvc::startCompAud_MT(const std::string& stepName,
                                   const std::string& compName) {


  std::lock_guard<std::mutex> lock( m_mutex_capture );
   
  int eventNumber = getEventNumber();

  PMonMT::StepCompEvent currentState = generate_parallel_state(stepName, compName, eventNumber);
  m_measurement.capture_MT( currentState );    
  m_parallelCompLevelData.addPointStart_MT(m_measurement, currentState);
}

void PerfMonMTSvc::stopCompAud_MT(const std::string& stepName,
                                   const std::string& compName) {


  std::lock_guard<std::mutex> lock( m_mutex_capture );

  int eventNumber = getEventNumber();

  PMonMT::StepCompEvent currentState = generate_parallel_state(stepName, compName, eventNumber);       
  m_measurement.capture_MT( currentState );
  m_parallelCompLevelData.addPointStop_MT(m_measurement, currentState);
}


// Report the results
void PerfMonMTSvc::report() {

  report2Log();
  report2JsonFile();

}


void PerfMonMTSvc::report2Log() { 

  // Header
  report2Log_Description();

  // Detailed tables
  if(m_printDetailedTables) {
    report2Log_Time_Serial();
  
    // If Event Loop Monitoring option is ON
    if(m_doEventLoopMonitoring)
      report2Log_Time_Parallel();

    if(doesDirectoryExist("/proc"))
      report2Log_Mem_Serial();
    else
      ATH_MSG_INFO("There is no /proc/ directory in this machine, therefore memory monitoring is failed!");
  }

  // Summary and system information
  report2Log_Summary();
  report2Log_CpuInfo();
}

void PerfMonMTSvc::report2Log_Description() const {
 
  ATH_MSG_INFO("=======================================================================================");
  ATH_MSG_INFO("                                 PerfMonMTSvc Report                                   ");
  ATH_MSG_INFO("=======================================================================================");
  ATH_MSG_INFO("!!! PLEASE NOTE THAT THIS SERVICE IS CURRENTLY IN R&D PHASE");
  ATH_MSG_INFO("=======================================================================================");
  ATH_MSG_INFO("*** Full set of information can also be found in: PerfMonMTSvc_result.json");
  ATH_MSG_INFO("*** In order to make plots using the results run the following commands:");
  ATH_MSG_INFO("*** $ perfmonmt-plotter PerfMonMTSvc_result.json");
  ATH_MSG_INFO("=======================================================================================");

}

void PerfMonMTSvc::report2Log_Time_Serial() { 

  using boost::format;

  ATH_MSG_INFO("=======================================================================================");
  ATH_MSG_INFO("                             CPU & Wall Time Monitoring                                ");
  ATH_MSG_INFO("                                   (Serial Steps)                                      ");
  ATH_MSG_INFO("=======================================================================================");
  ATH_MSG_INFO("Step             CPU Time [ms]       Wall Time [ms]      Component");

  divideData2Steps_serial(); 
  for(auto vec_itr : m_stdoutVec_serial){
    // Sort the results
    std::vector<std::pair<PMonMT::StepComp , PMonMT::MeasurementData*>> pairs;
    for (auto itr = vec_itr.begin(); itr != vec_itr.end(); ++itr)
      pairs.push_back(*itr);

    sort(pairs.begin(), pairs.end(), [=](std::pair<PMonMT::StepComp , PMonMT::MeasurementData*>& a, std::pair<PMonMT::StepComp , PMonMT::MeasurementData*>& b)
    {
      return a.second->m_delta_cpu + a.second->m_delta_wall > b.second->m_delta_cpu + b.second->m_delta_wall;
    }
    ); 
    for(auto it : pairs){

      ATH_MSG_INFO(format("%1% %|17t|%2% %|37t|%3% %|57t|%4%") % it.first.stepName % it.second->m_delta_cpu % it.second->m_delta_wall % it.first.compName);      

    }
    ATH_MSG_INFO("=======================================================================================");

  }
  
}
void PerfMonMTSvc::report2Log_Time_Parallel() {

  using boost::format;
 
  ATH_MSG_INFO("                             CPU & Wall Time Monitoring                                ");
  ATH_MSG_INFO("                                    (Event Loop)                                       ");
  ATH_MSG_INFO("=======================================================================================");
  ATH_MSG_INFO("Step             CPU Time [ms]       Wall Time [ms]      Component");

  parallelDataAggregator();
  divideData2Steps_parallel(); 

  for(auto vec_itr : m_stdoutVec_parallel){
    std::vector<std::pair<PMonMT::StepComp , PMonMT::Measurement>> pairs;
    for (auto itr = vec_itr.begin(); itr != vec_itr.end(); ++itr)
      pairs.push_back(*itr);

    sort(pairs.begin(), pairs.end(), [=](std::pair<PMonMT::StepComp , PMonMT::Measurement>& a, std::pair<PMonMT::StepComp , PMonMT::Measurement>& b)
    {
      return a.second.cpu_time  > b.second.cpu_time; // sort by cpu times
    }
    );
    for(auto it : pairs){

      ATH_MSG_INFO(format("%1%  %|17t|%2$.2f  %|37t|%3% %|57t|%4% ") % it.first.stepName % it.second.cpu_time % it.second.wall_time % it.first.compName);
    }
    ATH_MSG_INFO("=======================================================================================");

  }

}
void PerfMonMTSvc::report2Log_Mem_Serial() { 

  using boost::format;

  ATH_MSG_INFO("=======================================================================================");
  ATH_MSG_INFO("                                  Memory Monitoring                                    ");
  ATH_MSG_INFO("                                   (Serial Steps)                                      ");
  ATH_MSG_INFO("                                     Units: kB                                         ");
  ATH_MSG_INFO("=======================================================================================");
  ATH_MSG_INFO("Step           Vmem      Rss       Pss       Swap      Component");
                //Initialize     6144      6184      6184      0         AthDictLoaderSvc 

  
  for(auto vec_itr : m_stdoutVec_serial){
    // Sort the results
    std::vector<std::pair<PMonMT::StepComp , PMonMT::MeasurementData*>> pairs;
    for (auto itr = vec_itr.begin(); itr != vec_itr.end(); ++itr)
      pairs.push_back(*itr);

    sort(pairs.begin(), pairs.end(), [=](std::pair<PMonMT::StepComp , PMonMT::MeasurementData*>& a, std::pair<PMonMT::StepComp , PMonMT::MeasurementData*>& b)
    {
      return a.second->m_memMon_delta_map["vmem"] + a.second->m_memMon_delta_map["rss"] + a.second->m_memMon_delta_map["pss"] + a.second->m_memMon_delta_map["swap"] > \
             b.second->m_memMon_delta_map["vmem"] + b.second->m_memMon_delta_map["rss"] + b.second->m_memMon_delta_map["pss"] + b.second->m_memMon_delta_map["swap"];
    }
    ); 
    for(auto it : pairs){

      ATH_MSG_INFO(format("%1% %|15t|%2% %|25t|%3% %|35t|%4% %|45t|%5% %|55t|%6%") % it.first.stepName \
                                                                                   % it.second->m_memMon_delta_map["vmem"]    \
                                                                                   % it.second->m_memMon_delta_map["rss"]     \
                                                                                   % it.second->m_memMon_delta_map["pss"]     \
                                                                                   % it.second->m_memMon_delta_map["swap"]    \
                                                                                   % it.first.compName);      

    }
    ATH_MSG_INFO("=======================================================================================");

  }  
}

void report2Log_Mem_Parallel(){
  // to implement ...
  return;  
}

void PerfMonMTSvc::report2Log_Summary() {

  using boost::format;
 
  ATH_MSG_INFO("                                 Snaphots Summary                                      ");
  ATH_MSG_INFO("=======================================================================================");

  ATH_MSG_INFO(format("%1% %|13t|%2% %|25t|%3% %|37t|%4% %|44t|%5% %|55t|%6% %|66t|%7% %|77t|%8%") % "Step" 
                                                                                                   % "dCPU [s]" 
                                                                                                   % "dWall [s]" 
                                                                                                   % "<CPU>" 
                                                                                                   % "dVMEM [kB]" 
                                                                                                   % "dRSS [kB]" 
                                                                                                   % "dPSS [kB]" 
                                                                                                   % "dSwap [kB]");

  ATH_MSG_INFO("---------------------------------------------------------------------------------------");

  for(unsigned int idx=0; idx<3; idx++)  {
    ATH_MSG_INFO(format("%1% %|13t|%2% %|25t|%3% %|37t|%4$.2f %|44t|%5% %|55t|%6% %|66t|%7% %|77t|%8%") % m_snapshotStepNames[idx] 
                                                                                                        % (m_snapshotData[idx].m_delta_cpu * 0.001) 
                                                                                                        % (m_snapshotData[idx].m_delta_wall * 0.001) 
                                                                                                        % (m_snapshotData[idx].m_delta_cpu/m_snapshotData[idx].m_delta_wall) 
                                                                                                        % m_snapshotData[idx].m_memMon_delta_map["vmem"] 
                                                                                                        % m_snapshotData[idx].m_memMon_delta_map["rss"] 
                                                                                                        % m_snapshotData[idx].m_memMon_delta_map["pss"] 
                                                                                                        % m_snapshotData[idx].m_memMon_delta_map["swap"]);
   }

  ATH_MSG_INFO("***************************************************************************************");

  ATH_MSG_INFO(format( "%1% %|35t|%2% ") % "Number of events processed:" % m_eventCounter);
  ATH_MSG_INFO(format( "%1% %|35t|%2$.0f ") % "CPU usage per event [ms]:" % (m_snapshotData[1].m_delta_cpu / m_eventCounter));
  ATH_MSG_INFO(format( "%1% %|35t|%2$.3f ") % "Events per second:" % (m_eventCounter / m_snapshotData[1].m_delta_wall * 1000.));

  ATH_MSG_INFO("=======================================================================================");
}

void PerfMonMTSvc::report2Log_CpuInfo() const {

  using boost::format;

  ATH_MSG_INFO("                                  System Information                                   ");
  ATH_MSG_INFO("=======================================================================================");

  ATH_MSG_INFO(format( "%1% %|34t|%2% ") % "CPU Model:" % get_cpu_model_info());
  ATH_MSG_INFO(format( "%1% %|35t|%2% ") % "Number of Available Cores:" % get_cpu_core_info());

  ATH_MSG_INFO("=======================================================================================");
}


void PerfMonMTSvc::report2JsonFile() const {

  json j;

  report2JsonFile_Summary(j);
  report2JsonFile_Time_Serial(j);

  if(doesDirectoryExist("/proc"))
    report2JsonFile_Mem_Serial(j);

  if(m_doEventLoopMonitoring)
    report2JsonFile_Time_Parallel(j);
 

  std::ofstream o("PerfMonMTSvc_result.json");
  o << std::setw(4) << j << std::endl;

}

void PerfMonMTSvc::report2JsonFile_Summary(nlohmann::json& j) const {

  // Report snapshot level results
  for(int i = 0; i < 3; i++ ){

    // Clean this part!
    double wall_time = m_snapshotData[i].m_delta_wall;
    double cpu_time =  m_snapshotData[i].m_delta_cpu;

    j["Snapshot_level"][m_snapshotStepNames[i]] = { {"cpu_time", cpu_time}, {"wall_time", wall_time} };

  }

}
void PerfMonMTSvc::report2JsonFile_Time_Serial(nlohmann::json& j) const {

  // Report component level time measurements in serial steps
  for(auto& it : m_compLevelDataMap){

    std::string stepName = it.first.stepName;
    std::string compName = it.first.compName;
    
    double wall_time = it.second->m_delta_wall;
    double cpu_time = it.second->m_delta_cpu;
    
    // nlohmann::json syntax
    j["TimeMon_Serial"][stepName][compName] =  { {"cpu_time", cpu_time}, {"wall_time", wall_time} } ; 

  }
}

void PerfMonMTSvc::report2JsonFile_Time_Parallel(nlohmann::json& j) const {

  // Report component level time measurements in parallel steps
  for(auto& it : m_aggParallelCompLevelDataMap){

    std::string stepName = it.first.stepName;
    std::string compName = it.first.compName;

    double wall_time = it.second.wall_time;
    double cpu_time = it.second.cpu_time;

    j["TimeMon_Parallel"][stepName][compName] = { {"cpu_time", cpu_time}, {"wall_time", wall_time} } ; 

  }
}

void PerfMonMTSvc::report2JsonFile_Mem_Serial(nlohmann::json& j) const{
  
  // Report component level memory measurements in serial steps
  for(auto& it : m_compLevelDataMap){

    std::string stepName = it.first.stepName;
    std::string compName = it.first.compName;
    
    long vmem = it.second->m_memMon_delta_map["vmem"];
    long rss = it.second->m_memMon_delta_map["rss"];
    long pss = it.second->m_memMon_delta_map["pss"];
    long swap = it.second->m_memMon_delta_map["swap"];
    
    // nlohmann::json syntax
    j["MemMon_Serial"][stepName][compName] =  { {"vmem", vmem}, {"rss", rss}, {"pss", pss}, {"swap", swap} } ; 

    // Free the dynamically allocated space
    delete it.second;
  }
}

bool PerfMonMTSvc::isLoop() const {
  int eventNumber = getEventNumber();
  return (eventNumber >= 0) ? true : false;
}

int PerfMonMTSvc::getEventNumber() const {

  auto ctx = Gaudi::Hive::currentContext();
  int eventNumber = ctx.eventID().event_number();
  return eventNumber;
}

PMonMT::StepComp PerfMonMTSvc::generate_serial_state( const std::string& stepName,
                                                      const std::string& compName) const {

  PMonMT::StepComp currentState;
  currentState.stepName = stepName;
  currentState.compName = compName;
  return currentState;
}

PMonMT::StepCompEvent PerfMonMTSvc::generate_parallel_state( const std::string& stepName,
                                                             const std::string& compName,
                                                             const int& eventNumber) const {

  PMonMT::StepCompEvent currentState;
  currentState.stepName = stepName;
  currentState.compName = compName;
  currentState.eventNumber = eventNumber;
  return currentState;
}

void PerfMonMTSvc::parallelDataAggregator(){

  std::map< PMonMT::StepComp, PMonMT::Measurement >::iterator sc_itr;

  for(auto& sce_itr : m_parallelCompLevelData.m_timeMon_delta_map ){

    PMonMT::StepComp currentState = generate_serial_state (sce_itr.first.stepName, sce_itr.first.compName);
 
    // If the current state exists in the map, then aggregate it. o/w create a instance for it.
    sc_itr = m_aggParallelCompLevelDataMap.find(currentState);
    if(sc_itr != m_aggParallelCompLevelDataMap.end()){
      m_aggParallelCompLevelDataMap[currentState].cpu_time += sce_itr.second.cpu_time;
      m_aggParallelCompLevelDataMap[currentState].wall_time += sce_itr.second.wall_time;
    }
    else{
      m_aggParallelCompLevelDataMap[currentState] = sce_itr.second;      
    }
      
  } 

}

void PerfMonMTSvc::divideData2Steps_serial(){
  for(auto it : m_compLevelDataMap){

    if(it.first.stepName == "Initialize")
      m_compLevelDataMap_ini[it.first] = it.second;
    if(it.first.stepName == "Start")
      m_compLevelDataMap_start[it.first] = it.second;
    if(it.first.stepName == "Stop")
      m_compLevelDataMap_stop[it.first] = it.second;
    if(it.first.stepName == "Finalize")
      m_compLevelDataMap_fin[it.first] = it.second; 
    if(it.first.stepName == "preLoadProxy")
      m_compLevelDataMap_plp[it.first] = it.second;
    if(it.first.stepName == "Callback")
      m_compLevelDataMap_cbk[it.first] = it.second;

  }
  m_stdoutVec_serial.push_back(m_compLevelDataMap_ini);
  m_stdoutVec_serial.push_back(m_compLevelDataMap_start);
  m_stdoutVec_serial.push_back(m_compLevelDataMap_stop);
  m_stdoutVec_serial.push_back(m_compLevelDataMap_fin);
  m_stdoutVec_serial.push_back(m_compLevelDataMap_plp);
  m_stdoutVec_serial.push_back(m_compLevelDataMap_cbk);
}

void PerfMonMTSvc::divideData2Steps_parallel(){
  for(auto it : m_aggParallelCompLevelDataMap){

    if(it.first.stepName == "Execute")
      m_aggParallelCompLevelDataMap_evt[it.first] = it.second;
    if(it.first.stepName == "Stop")
      m_aggParallelCompLevelDataMap_stop[it.first] = it.second;
    if(it.first.stepName == "preLoadProxy")
      m_aggParallelCompLevelDataMap_plp[it.first] = it.second;
    if(it.first.stepName == "Callback")
      m_aggParallelCompLevelDataMap_cbk[it.first] = it.second;

  }

  m_stdoutVec_parallel.push_back(m_aggParallelCompLevelDataMap_evt);
  m_stdoutVec_parallel.push_back(m_aggParallelCompLevelDataMap_stop);
  m_stdoutVec_parallel.push_back(m_aggParallelCompLevelDataMap_plp);
  m_stdoutVec_parallel.push_back(m_aggParallelCompLevelDataMap_cbk);
}


std::string PerfMonMTSvc::get_cpu_model_info() const  {

  std::string cpu_model;

  std::ifstream file("/proc/cpuinfo");
  std::string line;
  if(file.is_open()){

    std::string delimiter = ":";
    while(getline( file, line)){

      std::string key = line.substr(0, line.find(delimiter));
      if(key == "model name	"){
        cpu_model = line.substr(line.find(delimiter)+1, line.length());
        break;
      }
    }
    file.close();
    return cpu_model;
  }
  else{
    std::cout << "Unable to open /proc/cpuinfo" << std::endl;
    return "Unable to open /proc/cpuinfo";
  }

}

int PerfMonMTSvc::get_cpu_core_info() const  {

  int logical_core_num = 0;

  std::ifstream file("/proc/cpuinfo");
  std::string line;
  if(file.is_open()){

    std::string delimiter = ":";
    while(getline( file, line)){

      std::string key = line.substr(0, line.find(delimiter));
      if(key == "processor	"){
        logical_core_num++;
      }
    }
    file.close();
    return logical_core_num;
  }
  else{
    std::cout << "Unable to open /proc/cpuinfo" << std::endl;
    return -1;
  }
}
