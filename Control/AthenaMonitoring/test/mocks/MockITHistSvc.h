#ifndef AthenaMonitoring_test_mocks_MockITHistSvc_h
#define AthenaMonitoring_test_mocks_MockITHistSvc_h

#include "GaudiKernel/ITHistSvc.h"

class MockITHistSvc : public ITHistSvc {
  private:
    const std::string m_name = "MockITHistSvc";
  public:   
    virtual const std::string& name() const override { return m_name; };
    Gaudi::StateMachine::State FSMState() const override { return Gaudi::StateMachine::OFFLINE; }
    Gaudi::StateMachine::State targetFSMState() const override { return Gaudi::StateMachine::OFFLINE; }
    StatusCode queryInterface(const InterfaceID&, void**) override { return StatusCode::SUCCESS; }
    StatusCode sysFinalize() override { return StatusCode::SUCCESS; }
    StatusCode sysInitialize() override { return StatusCode::SUCCESS; }
    StatusCode sysReinitialize() override { return StatusCode::SUCCESS; }
    StatusCode sysRestart() override { return StatusCode::SUCCESS; }
    StatusCode sysStart() override { return StatusCode::SUCCESS; }
    StatusCode sysStop() override { return StatusCode::SUCCESS; }
    StatusCode configure() override { return StatusCode::SUCCESS; }
    StatusCode finalize() override { return StatusCode::SUCCESS; }
    StatusCode initialize() override { return StatusCode::SUCCESS; }
    StatusCode reinitialize() override { return StatusCode::SUCCESS; }
    StatusCode restart() override { return StatusCode::SUCCESS; }
    StatusCode start() override { return StatusCode::SUCCESS; }
    StatusCode stop() override { return StatusCode::SUCCESS; }
    StatusCode terminate() override { return StatusCode::SUCCESS; }
    StatusCode deReg(TObject*) override { return StatusCode::SUCCESS; }
    StatusCode deReg(const std::string&) override { return StatusCode::SUCCESS; }
    StatusCode getGraph(const std::string&, TGraph*&) const override { return StatusCode::SUCCESS; }
    StatusCode getHist(const std::string&, TH1*&, size_t) const override { return StatusCode::SUCCESS; }
    StatusCode getHist(const std::string&, TH2*&, size_t) const override { return StatusCode::SUCCESS; }
    StatusCode getHist(const std::string&, TH3*&, size_t) const override { return StatusCode::SUCCESS; }
    StatusCode getShared(const std::string&, LockedHandle<TGraph>&) const override { return StatusCode::SUCCESS; }
    StatusCode getShared(const std::string&, LockedHandle<TH1>&) const override { return StatusCode::SUCCESS; }
    StatusCode getShared(const std::string&, LockedHandle<TH2>&) const override { return StatusCode::SUCCESS; }
    StatusCode getShared(const std::string&, LockedHandle<TH3>&) const override { return StatusCode::SUCCESS; }
    StatusCode getTHists(TDirectory*, TList&, bool) const override { return StatusCode::SUCCESS; }
    StatusCode getTHists(TDirectory*, TList&, bool, bool) override { return StatusCode::SUCCESS; }
    StatusCode getTHists(const std::string&, TList&, bool) const override { return StatusCode::SUCCESS; }
    StatusCode getTHists(const std::string&, TList&, bool, bool) override { return StatusCode::SUCCESS; }
    StatusCode getTTrees(TDirectory*, TList&, bool) const override { return StatusCode::SUCCESS; }
    StatusCode getTTrees(TDirectory*, TList&, bool, bool) override { return StatusCode::SUCCESS; }
    StatusCode getTTrees(const std::string&, TList&, bool) const override { return StatusCode::SUCCESS; }
    StatusCode getTTrees(const std::string&, TList&, bool, bool) override { return StatusCode::SUCCESS; }
    StatusCode getTree(const std::string&, TTree*&) const override { return StatusCode::SUCCESS; }
    StatusCode merge(TObject*) override { return StatusCode::SUCCESS; }
    StatusCode merge(const std::string&) override { return StatusCode::SUCCESS; }
    StatusCode regGraph(const std::string&) override { return StatusCode::SUCCESS; }
    StatusCode regGraph(const std::string&, TGraph*) override { return StatusCode::SUCCESS; }
    StatusCode regGraph(const std::string&, std::unique_ptr<TGraph>) override { return StatusCode::SUCCESS; }
    StatusCode regHist(const std::string&) override { return StatusCode::SUCCESS; }
    StatusCode regHist(const std::string&, TH1*) override { return StatusCode::SUCCESS; }
    StatusCode regHist(const std::string&, std::unique_ptr<TH1>) override { return StatusCode::SUCCESS; }
    StatusCode regHist(const std::string&, std::unique_ptr<TH1>, TH1*) override { return StatusCode::SUCCESS; }
    StatusCode regShared(const std::string&, std::unique_ptr<TGraph>, LockedHandle<TGraph>&) override { return StatusCode::SUCCESS; }
    StatusCode regShared(const std::string&, std::unique_ptr<TH1>, LockedHandle<TH1>&) override { return StatusCode::SUCCESS; }
    StatusCode regShared(const std::string&, std::unique_ptr<TH2>, LockedHandle<TH2>&) override { return StatusCode::SUCCESS; }
    StatusCode regShared(const std::string&, std::unique_ptr<TH3>, LockedHandle<TH3>&) override { return StatusCode::SUCCESS; }
    StatusCode regTree(const std::string&) override { return StatusCode::SUCCESS; }
    StatusCode regTree(const std::string&, TTree*) override { return StatusCode::SUCCESS; }
    StatusCode regTree(const std::string&, std::unique_ptr<TTree>) override { return StatusCode::SUCCESS; }
    bool exists(const std::string&) const override { return false; }
    long unsigned int addRef() override { return 0; }
    long unsigned int release() override { return 0; }
    std::vector<std::string> getGraphs() const override { return {}; }
    std::vector<std::string> getHists() const override { return {}; }
    std::vector<std::string> getTrees() const override { return {}; }
    void setServiceManager(ISvcManager*) override {}
};

#endif /* AthenaMonitoring_test_mocks_MockITHistSvc_h */