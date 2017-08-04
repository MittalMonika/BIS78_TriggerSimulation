///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// AthAlgTool.h 
// Header file for class AthAlgTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENABASECOMPS_ATHALGTOOL_H 
#define ATHENABASECOMPS_ATHALGTOOL_H 1

// STL includes
#include <string>
#include <type_traits>

// Need to do this very early so parser for VarHandleKey picked up
#include <string>
#include "GaudiKernel/StatusCode.h"
namespace SG {
  class VarHandleKey;
  class VarHandleKeyArray;
}
namespace Gaudi {
  namespace Parsers {
    StatusCode parse(SG::VarHandleKey& v, const std::string& s);
    StatusCode parse(SG::VarHandleKeyArray& v, const std::string& s);
  }
}

// Framework includes
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthMemMacros.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/VarHandleProperty.h"
#include "StoreGate/VarHandleKeyProperty.h"
#include "StoreGate/VarHandleKey.h"
#include "StoreGate/VarHandleKeyArray.h"
#include "StoreGate/VarHandleKeyArrayProperty.h"
#include "AthenaKernel/IUserDataSvc.h"

class AthAlgTool : 
  public ::AlgTool,
  public ::AthMessaging
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  // fwd compat w/ gaudi-21
  using AthMessaging::msg;

  // Copy constructor: 

  /// Constructor with parameters: 
  AthAlgTool( const std::string& type,
              const std::string& name, 
              const IInterface* parent );

  /// Destructor: 
  virtual ~AthAlgTool(); 

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /** @brief The standard @c StoreGateSvc (event store)
   * Returns (kind of) a pointer to the @c StoreGateSvc
   */
  ServiceHandle<StoreGateSvc>& evtStore() const;

  /** @brief The standard @c StoreGateSvc/DetectorStore
   * Returns (kind of) a pointer to the @c StoreGateSvc
   */
  ServiceHandle<StoreGateSvc>& detStore() const;

  /** @brief The standard @c UserDataSvc 
   * Returns (kind of) a pointer to the @c UserDataSvc
   */
  ServiceHandle<IUserDataSvc>& userStore() const;

private:
  // to keep track of VarHandleKeyArrays for data dep registration
  mutable std::vector<SG::VarHandleKeyArray*> m_vhka;

public:
  /////////////////////////////////////////////////////////////////
  //
  //// Enable use of Gaudi::Property<Foo> m_foo {this,"NAME",init,"doc"};
  //   style properties in AthAlgorithms
  //

  template <class T>
  Property& declareProperty(Gaudi::Property<T> &t) {
    return AthAlgTool::declareGaudiProperty(t, 
                                            std::is_base_of<SG::VarHandleKey, T>(),
                                            std::is_base_of<SG::VarHandleKeyArray, T>()
                                            );
  }

private:
  /**
   * @brief specialization for handling Gaudi::Property<SG::VarHandleKey>
   *
   */
  template <class T>
  Property& declareGaudiProperty(Gaudi::Property<T> &hndl, 
                                 std::true_type, std::false_type) {
    std::string doc = hndl.documentation();

    return *AthAlgTool::declareProperty(hndl.name(), hndl.value(), doc);
  }

  /**
   * @brief specialization for handling Gaudi::Property<SG::VarHandleKeyArray>
   *
   */
  template <class T>
  Property& declareGaudiProperty(Gaudi::Property<T> &hndl, 
                                 std::false_type, std::true_type) {
    std::string doc = hndl.documentation();

    return *AthAlgTool::declareProperty(hndl.name(), hndl.value(), doc);

  }

  /**
   * @brief Error: can't be both a VarHandleKey and VarHandleKeyArray
   *
   */
  template <class T>
  Property& declareGaudiProperty(Gaudi::Property<T> &t, std::true_type, std::true_type) {
      ATH_MSG_ERROR("AthAlgTool::declareGaudiProperty: " << t 
                    << " cannot be both a VarHandleKey and VarHandleKeyArray. "
                    << "This should not happen!");
      throw std::runtime_error("AthAlgTool::declareGaudiProperty: cannot be both a VarHandleKey and VarHandleKeyArray (this should not happen)!");
    return AlgTool::declareProperty(t);
  }

  /**
   * @brief specialization for handling everything that's not a
   * Gaudi::Property<SG::VarHandleKey> or <SG::VarHandleKeyArray>
   *
   */
  template <class T>
  Property& declareGaudiProperty(Gaudi::Property<T> &t, std::false_type, std::false_type) {
    return AlgTool::declareProperty(t);
  }

  /////////////////////////////////////////////////////////////////
  //
  //// For automatic registration of Handle data products
  //

public:
  /**
   * @brief Declare a new Gaudi property.
   * @param name Name of the property.
   * @param property Object holding the property value.
   * @param doc Documentation string for the property.
   *
   * This is the version for types that derive from @c SG::VarHandleKey.
   * The property value object is put on the input and output lists as
   * appropriate; then we forward to the base class.
   */
  Property* declareProperty(const std::string& name,
                            SG::VarHandleKey& hndl,
                            const std::string& doc,
                            std::true_type,
                            std::false_type)
  {
    this->declare(hndl);
    hndl.setOwner(this);

    return AlgTool::declareProperty(name,hndl,doc);
  }


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  Property* declareProperty(const std::string& name,
                            SG::VarHandleKeyArray& hndArr,
                            const std::string& doc,
                            std::false_type,
                            std::true_type)
  {

    m_vhka.push_back(&hndArr);

    Property* p =  AlgTool::declareProperty(name, hndArr, doc);
    if (p != 0) {
      p->declareUpdateHandler(&AthAlgTool::updateVHKA, this);
    } else {
      ATH_MSG_ERROR("unable to call declareProperty on VarHandleKeyArray " 
                    << name);
    }

    return p;

  }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  // Since the contents of the VarHandleKeyArrays have not been read 
  // in from the configurables by the time that declareProperty is
  // executed, we must cache them and loop through them later to
  // register the data dependencies.
  //
  // However, we cannot actually call declare() on the key instances
  // until we know that the vector cannot change size anymore --- otherwise,
  // the pointers given to declare() may become invalid.  That basically means
  // that we can't call declare() until the derived class's initialize()
  // completes.  So instead of doing it here (which would be too early),
  // we override sysInitialize() and do it at the end of that.  But,
  // AlgTool::sysInitialize() wants to have the handle lists after initialize()
  // completes in order to do dependency analysis.  It gets these lists
  // solely by calling inputHandles() and outputHandles(), so we can get this
  // to work by overriding those methods and adding in the current contents
  // of the arrays.

  void updateVHKA(Property& /*p*/) {
    // debug() << "updateVHKA for property " << p.name() << " " << p.toString() 
    //         << "  size: " << m_vhka.size() << endmsg;
    for (auto &a : m_vhka) {
      std::vector<SG::VarHandleKey*> keys = a->keys();
      for (auto k : keys) {
        k->setOwner(this);
      }
    }
  }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  /**
   * @brief Declare a new Gaudi property.
   * @param name Name of the property.
   * @param property Object holding the property value.
   * @param doc Documentation string for the property.
   *
   * This is the generic version, for types that do not derive
   * from  @c SG::VarHandleKey.  It just forwards to the base class version
   * of @c declareProperty.
   */
  template <class T>
  Property* declareProperty(const std::string& name,
                            T& property,
                            const std::string& doc,
                            std::false_type,
                            std::false_type)
  {
    return AlgTool::declareProperty(name, property, doc);
  }


  /**
   * @brief Declare a new Gaudi property.
   * @param name Name of the property.
   * @param property Object holding the property value.
   * @param doc Documentation string for the property.
   *
   * This dispatches to either the generic @c declareProperty or the one
   * for VarHandle/Key, depending on whether or not @c property
   * derives from @c SG::VarHandleKey or SG::VarHandleKeyArray.
   */
  template <class T>
  Property* declareProperty(const std::string& name,
                            T& property,
                            const std::string& doc="none")
  {
    return declareProperty (name, property, doc,
                            std::is_base_of<SG::VarHandleKey, T>(),
                            std::is_base_of<SG::VarHandleKeyArray,T>()
                            );
  }


  /**
   * @brief Perform system initialization for a tool.
   *
   * We override this to declare all the elements of handle key arrays
   * at the end of initialization.
   * See comments on updateVHKA.
   */
  virtual StatusCode sysInitialize() override;


  /**
   * @brief Return this tool's input handles.
   *
   * We override this to include handle instances from key arrays
   * if they have not yet been declared.
   * See comments on updateVHKA.
   */
  virtual std::vector<Gaudi::DataHandle*> inputHandles() const override;


  /**
   * @brief Return this tools's output handles.
   *
   * We override this to include handle instances from key arrays
   * if they have not yet been declared.
   * See comments on updateVHKA.
   */
  virtual std::vector<Gaudi::DataHandle*> outputHandles() const override;


  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 
protected: 
  /// callback for output level property 
  void msg_update_handler(Property& outputLevel);
  /// callback to add storeName to ExtraInputs/Outputs data deps
  void extraDeps_update_handler(Property&);

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 

  /// Default constructor: 
  AthAlgTool(); //> not implemented
  AthAlgTool (const AthAlgTool&); //> not implemented
  AthAlgTool& operator= (const AthAlgTool&); //> not implemented

  typedef ServiceHandle<StoreGateSvc> StoreGateSvc_t;
  /// Pointer to StoreGate (event store by default)
  mutable StoreGateSvc_t m_evtStore;

  /// Pointer to StoreGate (detector store by default)
  mutable StoreGateSvc_t m_detStore;

  typedef ServiceHandle<IUserDataSvc> UserDataSvc_t;
  /// Pointer to IUserDataSvc
  mutable UserDataSvc_t m_userStore;

  bool m_varHandleArraysDeclared;
}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline
ServiceHandle<StoreGateSvc>& AthAlgTool::evtStore() const 
{ return m_evtStore; }

inline
ServiceHandle<StoreGateSvc>& AthAlgTool::detStore() const 
{ return m_detStore; }

inline
ServiceHandle<IUserDataSvc>& AthAlgTool::userStore() const 
{ return m_userStore; }

#endif //> ATHENABASECOMPS_ATHALGTOOL_H
