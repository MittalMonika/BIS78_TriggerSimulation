// ROOT include(s):
#include <TError.h>

// EDM include(s):
#include "AsgTools/AnaToolHandle.h"


#include "EgammaAnalysisInterfaces/IElectronPhotonVariableCorrectionTool.h"

#define MSGSOURCE "testElectronPhotonVariableCorrectionTool_DictionaryToolHandle"

int main(/*int argc, char* argv[]*/) //unused variable warnings!!
{
    Info(MSGSOURCE, "Configuring the ElectronPhotonVariableCorrectionTool");
    asg::AnaToolHandle<IElectronPhotonVariableCorrectionTool> myTool("ElectronPhotonVariableCorrectionTool");
    myTool.setProperty("ConfigFile", "ElectronPhotonShowerShapeFudgeTool/ElectronPhotonVariableCorrectionTool_ExampleElectronConf.conf").ignore();
    if(myTool.initialize() != StatusCode::SUCCESS)
    {
        Error(MSGSOURCE, "Unable to initialize the ElectronPhotonVariableCorrectionTool!");
        return 1;
    }
    else
    {
        Info(MSGSOURCE, "Initialized the ElectronPhotonVariableCorrectionTool!");
    }
    

    return 0;
} //end main