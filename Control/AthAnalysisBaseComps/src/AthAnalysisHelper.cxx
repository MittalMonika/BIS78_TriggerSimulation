/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "AthAnalysisBaseComps/AthAnalysisHelper.h"

#include "AthContainers/AuxTypeRegistry.h"

#include "GaudiKernel/AttribStringParser.h"
#include "TROOT.h"
#include "boost/algorithm/string/case_conv.hpp"

const std::string AthAnalysisHelper::UNDEFINED = "__UNDEFINED__";

ServiceHandle<IJobOptionsSvc> AthAnalysisHelper::joSvc = ServiceHandle<IJobOptionsSvc>("JobOptionsSvc","AthAnalysisHelper");

//need a constructor, implemented here, so that the dictionary library is linked to 
//the implementation library (see ldd libAthAnalysisBaseCompsDict.so ... needs a link)
AthAnalysisHelper::AthAnalysisHelper() { } 


IAppMgrUI* AthAnalysisHelper::initGaudi(const char* options) {
    IAppMgrUI* theApp = Gaudi::createApplicationMgr();
    //check if already configured 
    if(theApp->FSMState() != Gaudi::StateMachine::OFFLINE) return theApp;
    //set the joboptions 
    SmartIF<IProperty> propMgr(theApp);
    if(strlen(options)) {
      propMgr->setProperty("JobOptionsPath",options); 
    } else {
      propMgr->setProperty( "JobOptionsType", "NONE" ); //no joboptions given
    }
    propMgr->setProperty("EventLoop","MinimalEventLoopMgr"); //using this instead of the default EventLoopMgr means some services (e.g. EventSelector) are not created, which is good! :-)

    //configure and return
    theApp->configure(); 
    propMgr->setProperty("OutputLevel","3"); //INFO
    theApp->initialize();
    return theApp;
}

bool AthAnalysisHelper::toolExists( const std::string& fullName ) {
  ServiceHandle<IToolSvc> toolSvc("ToolSvc","AthAnalysisHelper");
  if(toolSvc.retrieve().isFailure()) return false;
  auto existingTools = toolSvc->getInstances();
  for(auto& toolName : existingTools) {
    if(fullName==toolName) { toolSvc.release().ignore(); return true; }
  }
  toolSvc.release().ignore();
  return false;
}

void AthAnalysisHelper::dumpJobOptionProperties(const std::string& client) {
   ServiceHandle<IJobOptionsSvc> joSvc("JobOptionsSvc","AthAnalysisHelper");
   if(joSvc.retrieve().isFailure()) return;
   std::vector<std::string> clients = joSvc->getClients();
   for(auto& cl : clients) {
      if(cl.find(client)!=0) continue; //must start with client
      auto props = joSvc->getProperties(cl);
      if(!props) continue;
      for(auto prop : *props) {
         std::cout << cl << "." << prop->name() << " = " << prop->toString() << std::endl;
      }
   }
   joSvc.release().ignore();
}

std::string AthAnalysisHelper::getProperty(const std::string& client, const std::string& property) {
   std::string out(UNDEFINED);
   ServiceHandle<IJobOptionsSvc> joSvc("JobOptionsSvc","AthAnalysisHelper");
   if(joSvc.retrieve().isFailure()) return out;
   auto props = joSvc->getProperties(client);
   if(!props) { joSvc.release().ignore(); return out; }
   for(auto prop : *props) {
     if(prop->name()!=property) continue;
     out = prop->toString(); break;
   }
   joSvc.release().ignore();
   return out;
}

void AthAnalysisHelper::printAuxElement(const SG::AuxElement& ae) {
   const SG::auxid_set_t& auxids = ae.getAuxIDs();
   SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
   for( SG::auxid_t aux : auxids) {
      std::string name = reg.getName( aux );
      std::cout << "    " << name << " = ";
      const std::type_info& typeinfo = *reg.getType(aux);
#define PRINT_AE( TYPE ) \
if(typeinfo==typeid(TYPE) && ae.isAvailable<TYPE>(name)) std::cout << ae.auxdata<TYPE>(name) << " (" << #TYPE << ")" << std::endl;

      PRINT_AE( bool )
      else PRINT_AE(uint)
      else PRINT_AE(int)
      else PRINT_AE(float)
      else PRINT_AE(double)
      else PRINT_AE(unsigned char)
      else PRINT_AE(char)
      else PRINT_AE(short)
      else PRINT_AE(unsigned short)
      else PRINT_AE(unsigned long long)
      else std::cout << " (" << System::typeinfoName( typeinfo ) << ")" << std::endl;
      
#undef PRINT_AE
   }
}

void AthAnalysisHelper::dumpProperties(const IProperty& component) {
  for(auto p : component.getProperties()) {
    std::cout << p->name() << " = " << p->toString() << std::endl;
  }
}


TFile* AthAnalysisHelper::getOutputFile(const std::string& streamName) {
  ServiceHandle<IProperty> histSvc("THistSvc","");
  auto& prop = histSvc->getProperty("Output");

  std::vector<std::string> outputs;
  if( Gaudi::Parsers::parse(outputs,prop.toString()).isFailure() ) {
    return nullptr;
  }

  //extract the DATAFILE part of the string
  std::string fileName="";
  for(std::string& output : outputs) {
    if( output.substr(0,output.find(" "))!=streamName ) continue;

    //got here .. means we found the stream ...
    for(auto attrib : Gaudi::Utils::AttribStringParser(output.substr(output.find(" ")+1))) {
      auto TAG = boost::algorithm::to_upper_copy(attrib.tag);
     
      if(TAG=="FILE" || TAG=="DATAFILE") {
	fileName = attrib.value;
	break;
      }

    }
    if(fileName.length()) {
      return static_cast<TFile*>(gROOT->GetListOfFiles()->FindObject(fileName.c_str()));
    }

  }
  return 0;

}
