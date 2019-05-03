/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SYSTEMATICS_HANDLES__SYS_DECORATION_HANDLE_H
#define SYSTEMATICS_HANDLES__SYS_DECORATION_HANDLE_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/MsgStream.h>
#include <PATInterfaces/SystematicSet.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <string>
#include <type_traits>
#include <unordered_map>

class StatusCode;

namespace CP
{
  class SystematicSet;

  /// \brief a data handle for reading systematics varied input data

  template<typename T> class SysDecorationHandle final : public ISysHandleBase
  {
    //
    // public interface
    //

    /// \brief standard constructor
  public:
    template<typename T2>
    SysDecorationHandle (T2 *owner, const std::string& propertyName,
                             const std::string& propertyValue,
                             const std::string& propertyDescription);


    /// \brief whether we have a name configured
  public:
    bool empty () const noexcept;

    /// \brief !empty()
  public:
    explicit operator bool () const noexcept;


    /// \brief get the name we retrieve from the event store
  public:
    const std::string& getName (const CP::SystematicSet& sys) const;


    /// \brief retrieve the object decoration for the given systematic
  public:
    const T& get (const SG::AuxElement& object,
                  const CP::SystematicSet& sys) const;


    /// \brief set the object decoration for the given systematic
  public:
    void set (SG::AuxElement& object, const T& value,
              const CP::SystematicSet& sys) const;



    //
    // inherited interface
    //

  public:
    virtual std::string getInputAffecting () const override;



    //
    // private interface
    //

    /// \brief the input name we use
  private:
    std::string m_inputName;

    /// \brief the regular expression for affecting systematics
  private:
    std::string m_affectingRegex {".*"};

    /// \brief the cache of names we use
  private:
    mutable std::unordered_map<CP::SystematicSet,std::tuple<std::string,SG::AuxElement::ConstAccessor<T>,SG::AuxElement::Accessor<T> > > m_dataCache;

    /// \brief get the data for the given systematics
  private:
    const std::tuple<std::string,SG::AuxElement::ConstAccessor<T>,SG::AuxElement::Accessor<T> >&
    getData (const CP::SystematicSet& sys) const;


    /// \brief the message stream we use
  private:
    MsgStream *m_msg {nullptr};

    /// \brief helper for message macros
  private:
    MsgStream& msg( ) const;

    /// \brief helper for message macros
  private:
    MsgStream& msg( const MSG::Level lvl ) const;
  };
}

#include "SysDecorationHandle.icc"

#endif
