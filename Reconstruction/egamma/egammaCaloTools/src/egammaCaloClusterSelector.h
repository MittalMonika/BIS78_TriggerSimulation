/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EGAMMACALOTOOLS_EGAMMACALOCLUSTERSELECTOR_H
#define EGAMMACALOTOOLS_EGAMMACALOCLUSTERSELECTOR_H

// Package includes
#include "egammaInterfaces/IegammaCaloClusterSelector.h"
#include "CaloEvent/CaloCellContainer.h"
#include "egammaInterfaces/IegammaCheckEnergyDepositTool.h"
#include "egammaInterfaces/IegammaMiddleShape.h"

// Framework includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "CLHEP/Units/SystemOfUnits.h"

// STL includes
#include <string>

using CLHEP::GeV;

/**
 * @class egammaCaloClusterSelector
 * @brief A tool which specifies whether a cluster passes a selection
 **/
class egammaCaloClusterSelector : public extends<AthAlgTool, IegammaCaloClusterSelector> {
public:
  egammaCaloClusterSelector(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~egammaCaloClusterSelector() override;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  bool passSelection(const xAOD::CaloCluster* cluster) const override final;

private:

  /// return the bin number given the EM Et.
  int findETBin(double EMEt) const;

  /** @brief Name of the cluster intput collection*/
  SG::ReadHandleKey<CaloCellContainer>   m_cellsKey {this,
      "CellContainerName", "AllCalo", 
      "Names of containers which contain cells"};

  //
  // The tools
  //
  /** @brief Pointer to the egammaCheckEnergyDepositTool*/
  ToolHandle<IegammaCheckEnergyDepositTool> m_egammaCheckEnergyDepositTool {this, 
      "egammaCheckEnergyDepositTool", "",
      "Optional tool that performs basic checks of viability of cluster"};

  /** @brief Middle shape tool; only used if cuts needing it defined */
  ToolHandle<IegammaMiddleShape> m_egammaMiddleShape {this, 
      "egammaMiddleShapeTool", "egammaMiddleShape/egammamiddleshape",
      "Optional tool that performs cuts on middle shape variables"};

  Gaudi::Property<double>  m_ClusterEtCut {this,
      "ClusterEtCut", 0.0, "Cut on cluster Et"};

  Gaudi::Property<std::vector<double> > m_EMEtRanges {this,
      "EMEtRanges", {1*GeV, 3*GeV}, 
      "EM Et Ranges to consider, with different cuts; Low limit of ranges given"};

  /// For the cuts below, the size must be 0 (meaning not applied)
  /// or equal to the number of Et ranges.
  Gaudi::Property<std::vector<double> >  m_EMFCuts {this,
      "EMFCuts", {}, "Cut on cluster EM fraction, per EM Et bin"};

  Gaudi::Property<std::vector<double> >  m_lateralCuts {this,
      "LateralCuts", {},
      "Cut on cluster LATERAL, i.e., the second transverse moment normalized, per EM Et bin"};

  Gaudi::Property<std::vector<double> >  m_RetaCuts {this,
      "RetaCut", 0.0, "Cut on cluster Reta"};

  // this variable is set at initialize based on the configuration
  bool m_runMiddleShapes;

};

#endif // RECONSTRUCTION/EGAMMA/EGAMMACALOTOOLS_EGAMMACALOCLUSTERSELECTOR_H
