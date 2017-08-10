//        Copyright Iowa State University 2017.
//                  Author: Nils Krumnack
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (nils.erik.krumnack@cern.ch) for bug
// reports, feature suggestions, praise and complaints.


//
// includes
//

#include <EventLoopComps/AnaAlgorithmConfig.h>

#include <EventLoopComps/AnaAlgorithm.h>
#include <RootCoreUtils/Assert.h>
#include <TInterpreter.h>
#include <regex>

//
// method implementations
//

namespace EL
{
  void AnaAlgorithmConfig :: 
  testInvariant () const
  {
    RCU_INVARIANT (this != nullptr);
  }



  AnaAlgorithmConfig :: 
  AnaAlgorithmConfig ()
  {
    RCU_NEW_INVARIANT (this);
  }



  const std::string& AnaAlgorithmConfig ::
  type () const noexcept
  {
    RCU_READ_INVARIANT (this);
    return m_type;
  }



  void AnaAlgorithmConfig ::
  setType (const std::string val_type) noexcept
  {
    RCU_CHANGE_INVARIANT (this);
    m_type = val_type;
  }



  const std::string& AnaAlgorithmConfig ::
  name () const noexcept
  {
    RCU_READ_INVARIANT (this);
    return m_name;
  }



  void AnaAlgorithmConfig ::
  setName (const std::string val_name) noexcept
  {
    RCU_CHANGE_INVARIANT (this);
    m_name = val_name;
  }



  StatusCode AnaAlgorithmConfig ::
  makeAlgorithm (std::unique_ptr<AnaAlgorithm>& algorithm) const
  {
    RCU_READ_INVARIANT (this);
    using namespace msgAlgorithmConfig;

    std::regex typeExpr ("[A-Za-z_][A-Za-z0-9_]*(::[A-Za-z_][A-Za-z0-9_]*)*");
    if (!std::regex_match (m_type, typeExpr))
    {
      ANA_MSG_ERROR ("type \"" << m_type << "\" does not match format expression");
      return StatusCode::FAILURE;
    }
    std::regex nameExpr ("[A-Za-z_][A-Za-z0-9_]*");
    if (!std::regex_match (m_name, nameExpr))
    {
      ANA_MSG_ERROR ("name \"" << m_name << "\" does not match format expression");
      return StatusCode::FAILURE;
    }
 
    ANA_MSG_DEBUG("Creating tool of type " << m_type);
    EL::AnaAlgorithm *alg = (EL::AnaAlgorithm*)
      (gInterpreter->Calc(("dynamic_cast<EL::AnaAlgorithm*>(new " + m_type + " (\"" + m_name + "\", nullptr))").c_str()));
      // (gInterpreter->Calc(("dynamic_cast<EL::AnaAlgorithm*>(new " + m_type + " (\"" + m_name + "\", (ISvcLocator*) nullptr))").c_str()));
    if (alg == nullptr)
    {
      ANA_MSG_ERROR ("failed to create algorithm of type " << m_type);
      ANA_MSG_ERROR ("make sure you created a dictionary for your algorithm");
      return StatusCode::FAILURE;
    }
    algorithm.reset (alg);

    for (auto& property : m_propertyValues)
    {
      if (!alg->setProperty (property.first, property.second).isSuccess())
      {
        ANA_MSG_ERROR ("failed to set property " << property.first << " with value " << property.second);
        return StatusCode::FAILURE;
      }
    }

    return StatusCode::SUCCESS;
  }
}
