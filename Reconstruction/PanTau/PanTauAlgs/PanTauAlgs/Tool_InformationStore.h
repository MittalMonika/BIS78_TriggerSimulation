/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//  Header file for class Tool_InformationStore
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Tool to store information needed in PanTau Algorithms
///////////////////////////////////////////////////////////////////
// limbach@physik.uni-bonn.de
///////////////////////////////////////////////////////////////////

#ifndef PANTAUALGS_TOOL_INFORMATIONSTORE_H
#define PANTAUALGS_TOOL_INFORMATIONSTORE_H


//! C++
#include <map>
#include <string>

//! ASG
#include "AsgTools/AsgTool.h"

//! PanTau
#include "PanTauAlgs/ITool_InformationStore.h"


namespace PanTau {

/** @class Tool_InformationStore
    Tool to store information needed in PanTau Algorithms
    @author Christian Limbach (limbach@physik.uni-bonn.de)
*/
  class Tool_InformationStore : public asg::AsgTool, virtual public PanTau::ITool_InformationStore {
    
    ASG_TOOL_CLASS1(Tool_InformationStore, PanTau::ITool_InformationStore)
    
    typedef std::map<std::string, std::string>                  MapString;
    typedef std::map<std::string, std::vector<std::string> >    MapVecString;
    typedef std::map<std::string, int>                          MapInt;
    typedef std::map<std::string, double>                       MapDouble;
    typedef std::map<std::string, std::vector<double> >         MapVecDouble;
    
    
    
    public:
        
        Tool_InformationStore(const std::string &name);
        virtual ~Tool_InformationStore ();
        
        virtual StatusCode initialize();
//         virtual StatusCode finalize  ();
        
        virtual StatusCode getInfo_Int(std::string varName,     int& value);
        virtual StatusCode getInfo_Double(std::string varName,  double& value);
        virtual StatusCode getInfo_VecDouble(std::string varName,  std::vector<double>& value);
        virtual StatusCode getInfo_String(std::string varName,  std::string& value);
        virtual StatusCode getInfo_VecString(std::string varName,  std::vector<std::string>& value);
        
        StatusCode  dumpMaps() const;
        
       
    private:
        
        //! named strings, ints etc... for configuration
        MapString       m_Infos_String;
        MapVecString    m_Infos_VecString;
        MapInt          m_Infos_Int;
        MapDouble       m_Infos_Double;
        MapVecDouble    m_Infos_VecDouble;
        
        //!other information
        // input containers
        std::string                       m_Name_Container_eflowRec;
        std::string                       m_Name_Container_TauRec;
        std::string                       m_Name_Container_Tracks;
        
       
};


} // end of namespace PanTau
#endif // PANTAUALGS_TAUIMPACTPARAMETEREXTRACTIONTOOL_H
 
