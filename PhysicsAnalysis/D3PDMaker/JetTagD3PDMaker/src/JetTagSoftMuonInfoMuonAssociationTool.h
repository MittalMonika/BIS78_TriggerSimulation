/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file D3PDMaker/JetTagD3PDMaker/src/JetTagSoftMuonInfoMuonAssociationTool.h
 * @author Georges Aad
 * @date Oct, 2010
 * @brief association from jets SoftMuonInfo object to Muons
 * 
 */

#ifndef JetTagD3PDMaker_JetTagSoftMuonInfoMuonAssociationTool_H
#define JetTagD3PDMaker_JetTagSoftMuonInfoMuonAssociationTool_H

#include "JetTagFinder.h"
#include "D3PDMakerUtils/MultiAssociationTool.h"
#include "GaudiKernel/ToolHandle.h"

class Jet;

namespace Analysis{
  class SoftMuonInfo;
  class Muon;
}


namespace D3PD {


/**
 * @brief Associate muons in SoftMuonInfo to muon containers
 *
 */

  
class JetTagSoftMuonInfoMuonAssociationTool
  : public MultiAssociationTool<Jet,Analysis::Muon>
{
public:

  /**
   * @brief Standard Gaudi tool constructor.
   * @param type The name of the tool type.
   * @param name The tool name.
   * @param parent The tool's Gaudi parent.
   */
  JetTagSoftMuonInfoMuonAssociationTool (const std::string& type,
                         const std::string& name,
                         const IInterface* parent);


  /// Standard Gaudi initialize method.
  virtual StatusCode initialize();


  /**
   * @brief Start the iteration for a new association.
   * @param p The object from which to associate.
   */
  virtual StatusCode reset (const Jet& p);


  /**
   * @brief Return a pointer to the next element in the association.
   * Return 0 when the association has been exhausted.
   */
  const Analysis::Muon* next();

  /**
   * @brief Create any needed tuple variables.
   *
   * This is called at the start of the first event.
   */

  virtual StatusCode book();


private:
  JetTagFinder m_finder;

  float *m_w; 
  float *m_pTRel;
  float *m_dRJet;

  const Jet* m_jet;

  const Analysis::SoftMuonInfo* m_softMuonInfo;

  int m_muItr;
  int m_muEnd;

  bool m_fillVariables;

};


} // namespace D3PD


#endif // JetTagD3PDMaker_JetTagSoftMuonInfoMuonAssociationTool_H
