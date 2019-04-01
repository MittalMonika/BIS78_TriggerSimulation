#include <functional>

#include "AthenaMonitoring/HistogramFiller/HistogramFactory.h"

class MockHistogramFactory : public Monitored::HistogramFactory {
  public:
    MockHistogramFactory()
      : Monitored::HistogramFactory(ServiceHandle<ITHistSvc>("", "") ,"") {}

    std::function<TNamed*(const Monitored::HistogramDef& def)> mock_create;
    TNamed* create(const Monitored::HistogramDef& def) override {
      return mock_create ? mock_create(def) : nullptr;
    }
};