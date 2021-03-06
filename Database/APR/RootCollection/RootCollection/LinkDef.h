/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#if ROOT_VERSION_CODE < ROOT_VERSION(5,99,0)
#pragma link C++ class std::pair< std::string, std::string >+;
#pragma link C++ class std::vector< std::pair< std::string, std::string > >+;
#endif 
#pragma link C++ class AttributeListLayout+;

#endif
