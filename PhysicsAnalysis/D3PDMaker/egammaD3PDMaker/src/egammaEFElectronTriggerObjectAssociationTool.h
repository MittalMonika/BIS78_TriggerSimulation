// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file egammaD3PDMaker/src/egammaEFElectronTriggerObjectAssociationTool.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2009
 * @brief Associate from an egamma to its matching EF electron trigger object.
 */


#ifndef EGAMMAD3PDMAKER_EGAMMAEFELECTRONTRIGGEROBJECTASSOCIATIONTOOL_H
#define EGAMMAD3PDMAKER_EGAMMAEFELECTRONTRIGGEROBJECTASSOCIATIONTOOL_H


// Forward declaration to ensure this overload is visible
// when the TriggerObjectAssociationTool code is instantiated.
#include "egammaEFlessForTriggerAssociation.h"


#include "TriggerD3PDMaker/TriggerObjectAssociationTool.h"
#include "TrigObjectMatching/DistanceFunctor.h"
#include "D3PDMakerUtils/IndexMap.h"
#include "egammaEvent/egamma.h"
#include "egammaEvent/egammaContainer.h"
#include "GaudiKernel/ToolHandle.h"


namespace D3PD {


class ICollectionGetterRegistryTool;


   /**
    *  @short Special metric for matching EF electrons to offline ones
    *
    *         Since the cluster information is missing in MC10a and MC10b,
    *         we can't use the default metric for matching offline electrons
    *         to trigger electrons.
    *
    *         This metric compares the eta and phi position returned by the
    *         EF electron object to the eta and phi of the track associated
    *         to the offline electron.
    *
    * @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
    *
    * $Revision: 364392 $
    * $Date$
    */
   class ElectronEFElectronDistanceFunctor :
      public DistanceFunctor< egamma, egamma > {

   private:
      virtual float calculateDistance( const egamma* ef,
                                       const egamma* offl ) const;

   }; // class ElectronEFElectronDistanceFunctor


/**
 * @brief Associate from an egamma to its matching EF trigger object.
 */
typedef TriggerObjectAssociationTool<egamma, egamma>
  egammaEFElectronTriggerObjectAssociationToolBase;

class egammaEFElectronTriggerObjectAssociationTool
  : public egammaEFElectronTriggerObjectAssociationToolBase
{
public:
  typedef egammaEFElectronTriggerObjectAssociationToolBase Base;


  /**
   * @brief Standard Gaudi tool constructor.
   * @param type The name of the tool type.
   * @param name The tool name.
   * @param parent The tool's Gaudi parent.
   */
  egammaEFElectronTriggerObjectAssociationTool (const std::string& type,
                                                const std::string& name,
                                                const IInterface* parent);


  /**
   * @brief Configure during initialization: type-check.
   * @param tree Our parent for tuple making.
   * @param ti Gives the type of the object being passed to @c resetUntyped.
   *
   * @c configureD3PD should check that the type of the object coming as input
   * (to @c getUntyped)
   * is compatible with what it expects, and raise an error otherwise.
   */
  virtual StatusCode configureD3PD (D3PD::IAddVariable* tree,
                                    const std::type_info& ti);


  /**
   * @brief Compare two objects by their position in a supplied collection.
   * @param p1 First object to compare.
   * @param p2 Second object to compare.
   */
  bool compareInSequence (const egamma* p1,
                          const egamma* p2);



private:
  /// Property: Label of getter for the sequence to define the final
  /// disambiguation of TrigElectrons.  Leave blank to not do this.
  std::string m_targetLabel;

  /// Property: The ICollectionGetterRegistryTool instance.
  ToolHandle<ICollectionGetterRegistryTool> m_registry;

  // Collection used for final object disambiguation.
  IndexMap m_map;

  /// Property: Use special metric designed for MC?
  bool m_useMCMetric;
  /// Special distance metric when running on MC
  ElectronEFElectronDistanceFunctor m_metric;
};


} // namespace D3PD



#endif // not EGAMMAD3PDMAKER_EGAMMAEFELECTRONTRIGGEROBJECTASSOCIATIONTOOL_H
