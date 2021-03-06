// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file D3PDMakerTest/src/ToObj2MultiAssociationTool.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2012
 * @brief Test code: Test MultiAssociationToolMulti.
 */


#ifndef D3PDMAKERTEST_TOOBJ2MULTIASSOCIATIONTOOL_H
#define D3PDMAKERTEST_TOOBJ2MULTIASSOCIATIONTOOL_H


#include "D3PDMakerUtils/MultiAssociationTool.h"
#include "D3PDMakerTest/Obj1.h"


namespace D3PDTest {


class Obj1;
class Obj2;


/**
 * @brief Test code: associate from an Obj1 to a set of Obj2.
 */
class ToObj2MultiAssociationTool
  : public D3PD::MultiAssociationTool<D3PD::Types<Obj1, Obj2>, Obj2>
{
public:
  typedef D3PD::MultiAssociationTool<D3PD::Types<Obj1, Obj2>, Obj2> Base;


  /**
   * @brief Standard Gaudi tool constructor.
   * @param type The name of the tool type.
   * @param name The tool name.
   * @param parent The tool's Gaudi parent.
   */
  ToObj2MultiAssociationTool (const std::string& type,
                              const std::string& name,
                              const IInterface* parent);


  /**
   * @brief Start the iteration for a new association.
   * @param p The object from which to associate.
   */
  virtual StatusCode reset (const Obj1& p);


  /**
   * @brief Start the iteration for a new association.
   * @param p The object from which to associate.
   */
  virtual StatusCode reset (const Obj2& p);


  /**
   * @brief Return a pointer to the next element in the association.
   *
   * Return 0 when the association has been exhausted.
   */
  virtual const Obj2* next ();


private:
  int m_which;

  std::vector<Obj2>::const_iterator m_it;
  std::vector<Obj2>::const_iterator m_end;

  std::vector<Obj2> m_tmpvec;
};


} // namespace D3PDTest



#endif // not D3PDMAKERTEST_TOOBJ2MULTIASSOCIATIONTOOL_H
