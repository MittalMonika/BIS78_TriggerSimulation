/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: ut_xaodtrigger_trigcomposite_test.cxx 761887 2016-07-14 13:16:16Z tbold $

// System include(s):
#include <cmath>
#include <iostream>

// EDM include(s):
#include "AthLinks/ElementLink.h"
#include "AthLinks/ElementLinkVector.h"

// Local include(s):
#include "xAODTrigger/TrigCompositeContainer.h"
#include "xAODTrigger/TrigCompositeAuxContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"

/// Helper macro for testing the code
#define SIMPLE_ASSERT( EXP )                                         \
   do {                                                              \
      const bool result = EXP;                                       \
      if( ! result ) {                                               \
         std::cerr << __FILE__ << ":" << __LINE__                    \
                   << "  Failed to execute: " << #EXP << std::endl;  \
         return 1;                                                   \
      }                                                              \
   } while( 0 )

/// Helper print operator
template< typename T >
std::ostream& operator<< ( std::ostream& out, const std::vector< T >& vec ) {

   out << "[";
   for( size_t i = 0; i < vec.size(); ++i ) {
      out << vec[ i ];
      if( ( i + 1 ) < vec.size() ) {
         out << ", ";
      }
   }
   out << "]";
   return out;
}

const unsigned uintTestConst = (1<<(sizeof(unsigned)*4-1)) + 41;

int populateObject(xAOD::TrigComposite* obj) {
   // Set some simple properties on it:
   obj->setName( "TestObj" );
   obj->setDetail( "IntValue", 12 );
   obj->setDetail( "UnsignedIntValue", uintTestConst );
   obj->setDetail( "FloatValue", 3.14f );
   obj->setDetail( "IntVecValue", std::vector< int >( { 1, 2, 3 } ) );
   obj->setDetail( "UnsignedIntVecValue", std::vector< unsigned int >( { uintTestConst, 2, 3 } ) );
   obj->setDetail( "FloatVecValue", std::vector< float >( { 1.23, 2.34 } ) );
   std::cout << "Set detail ok." << std::endl;

   // Now test the ElementLink functionality in a basic way:
   obj->setObjectLink( "MuonRoI",
                       ElementLink<xAOD::MuonRoIContainer>( 123, 456 ) );

   // Test the ElementLinkVector functionality in a basic way:
   ElementLinkVector<xAOD::MuonRoIContainer> elementLinks;
   elementLinks.push_back( ElementLink<xAOD::MuonRoIContainer>( 789, 012 ) );
   elementLinks.push_back( ElementLink<xAOD::MuonRoIContainer>( 345, 678 ) );
   obj->addObjectCollectionLinks("ManyMuonRoIs", elementLinks);

   return 0;
}

int testObject(const xAOD::TrigComposite* obj) {
   SIMPLE_ASSERT( obj->hasDetail<int>("IntValue") );
   SIMPLE_ASSERT( obj->hasDetail<unsigned int>("UnsignedIntValue") );
   SIMPLE_ASSERT( obj->hasDetail<std::vector<unsigned int> >("UnsignedIntVecValue") );
   SIMPLE_ASSERT( obj->hasDetail<std::vector<int> >("IntVecValue") );

   // Check them:
   SIMPLE_ASSERT( obj->name() == "TestObj" );

   int intValue = 0;
   SIMPLE_ASSERT( obj->getDetail( "IntValue", intValue ) );
   SIMPLE_ASSERT( intValue == 12 );

   unsigned int unsignedIntValue = 0; // we want to use also the sin bit
   SIMPLE_ASSERT( obj->getDetail( "UnsignedIntValue", unsignedIntValue ) );
   SIMPLE_ASSERT( unsignedIntValue == uintTestConst );

   float floatValue = 0;
   SIMPLE_ASSERT( obj->getDetail( "FloatValue", floatValue ) );
   SIMPLE_ASSERT( std::abs( floatValue - 3.14 ) < 0.001 );

   std::vector< int > intVector;
   SIMPLE_ASSERT( obj->getDetail( "IntVecValue", intVector ) );
   SIMPLE_ASSERT( intVector == std::vector< int >( { 1, 2, 3 } ) );

   std::vector<unsigned int> unsignedIntVector;
   SIMPLE_ASSERT( obj->getDetail("UnsignedIntVecValue", unsignedIntVector) );
   SIMPLE_ASSERT( unsignedIntVector == std::vector<unsigned int>( { uintTestConst, 2, 3 } ) );

   std::vector< float > floatVector;
   SIMPLE_ASSERT( obj->getDetail( "FloatVecValue", floatVector ) );

   // Simply just print the last one:
   std::cout << "FloatVecValue = " << floatVector << std::endl;

   int intValue2 = obj->getDetail<int>("IntValue");
   SIMPLE_ASSERT( intValue2 == 12 );
   unsigned int unsignedIntValue2 = obj->getDetail<unsigned int>("UnsignedIntValue");
   SIMPLE_ASSERT( unsignedIntValue2 == uintTestConst);
   float floatValue2 = obj->getDetail<float>("FloatValue");
   SIMPLE_ASSERT( std::abs( floatValue2 - 3.14 ) < 0.001 );
   std::vector<int> intVector2 =  obj->getDetail<std::vector<int>>( "IntVecValue");
   SIMPLE_ASSERT( intVector2 == std::vector< int >( { 1, 2, 3 } ) );
   std::vector< float > floatVector2 = obj->getDetail<std::vector<float>>( "FloatVecValue");
   std::cout << "Simple getDetail API ok." << std::endl;

   std::vector<unsigned int> unsignedIntVector2 = obj->getDetail<std::vector<unsigned int>>("UnsignedIntVecValue");
   SIMPLE_ASSERT( unsignedIntVector2 == std::vector<unsigned int>( { uintTestConst, 2, 3 } ) );

   try {
     obj->getDetail<int>("NonExistent");
     SIMPLE_ASSERT(false);
   } catch (...) {}

   try {
     obj->getDetail<int>("FloatValue"); // confuse type
     SIMPLE_ASSERT(false);
   } catch (...) {}

   try {
     obj->getDetail<int>("IntVecValue"); // confuse cardinality
     SIMPLE_ASSERT(false);
   } catch (...) {}
   std::cout << "Missing details handled ok." << std::endl;

   SIMPLE_ASSERT( obj->hasObjectLink( "MuonRoI" ) );
   SIMPLE_ASSERT( obj->linkColNames().size() == 3 );
   SIMPLE_ASSERT( obj->linkColKeys().size() == 3 );
   SIMPLE_ASSERT( obj->linkColIndices().size() == 3 );
   SIMPLE_ASSERT( obj->linkColClids().size() == 3 );
   SIMPLE_ASSERT( obj->linkColKeys()[ 0 ] == 123 );
   SIMPLE_ASSERT( obj->linkColIndices()[ 0 ] == 456 );
   SIMPLE_ASSERT( obj->linkColClids()[ 0 ] ==
                  ClassID_traits< xAOD::MuonRoIContainer >::ID() );

   std::cout << "Basic link functionality OK" << std::endl;

   ElementLink< xAOD::MuonRoIContainer > getMuonRoILink = obj->objectLink<xAOD::MuonRoIContainer>("MuonRoI");
   SIMPLE_ASSERT(getMuonRoILink == ElementLink<xAOD::MuonRoIContainer>( 123, 456 ));

   ElementLinkVector<xAOD::MuonRoIContainer> getMuonRoILinks = obj->objectCollectionLinks<xAOD::MuonRoIContainer>("ManyMuonRoIs");
   ElementLinkVector<xAOD::MuonRoIContainer> elementLinks;
   elementLinks.push_back( ElementLink<xAOD::MuonRoIContainer>( 789, 012 ) );
   elementLinks.push_back( ElementLink<xAOD::MuonRoIContainer>( 345, 678 ) );
   SIMPLE_ASSERT(getMuonRoILinks == elementLinks);

   std::cout << "Link recovery OK" << std::endl;

   return 0;
}

int main() {

   // Create the container that we want to test:
   xAOD::TrigCompositeAuxContainer aux;
   xAOD::TrigCompositeContainer c;
   c.setStore( &aux );

   // Create a test object:
   xAOD::TrigComposite* obj = new xAOD::TrigComposite();
   c.push_back( obj );

   std::cout << "Populating initial TC object" << std::endl;
   populateObject(obj);

   // Standalone with copy constructor. Should get own store.
   xAOD::TrigComposite* copyConstructor = new xAOD::TrigComposite(*obj);

   // Standalone with assignment operator. Should get own store.
   xAOD::TrigComposite* assignmentOperator = new xAOD::TrigComposite();
   *assignmentOperator = *obj;

   // Creating a new object in a new store and copying to it
   xAOD::TrigCompositeAuxContainer aux2;
   xAOD::TrigCompositeContainer c2;
   c2.setStore( &aux2 );
   xAOD::TrigComposite* obj2 = new xAOD::TrigComposite();
   c2.push_back( obj2 );
   *obj2 = *obj;

   std::cout << "Testing initial TC object" << std::endl;
   SIMPLE_ASSERT( testObject(obj) == 0 );

   std::cout << "Testing copy constructor" << std::endl;
   SIMPLE_ASSERT( testObject(copyConstructor) == 0 );

   std::cout << "Testing assignment operator (standalone object)" << std::endl;
   SIMPLE_ASSERT( testObject(assignmentOperator) == 0 );

   std::cout << "Testing assignment operator (object with store)" << std::endl;
   SIMPLE_ASSERT( testObject(obj2) == 0 );

   // Apparently everything went well:
   std::cout << "All tests successful." << std::endl;

   // Return gracefully:
   return 0;
}
