// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODROOTACCESS_TOOLS_TDESTRUCTORREGISTRY_H
#define XAODROOTACCESS_TOOLS_TDESTRUCTORREGISTRY_H

// System include(s):
#include <map>
#include <shared_mutex>
#include <memory>

// Forward declaration(s):
namespace std {
   class type_info;
}

namespace xAOD {

   // Forward declaration(s):
   class TVirtualDestructor;

   /// Application-wide registry of destructor objects
   ///
   /// This registry is used to keep track of how to destruct objects
   /// that are kept in the transient store, and don't have a ROOT dictionary
   /// available for them.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TDestructorRegistry {

   public:
      /// Function accessing the singleton instance of this type
      static TDestructorRegistry& instance();

      /// Get the destructor for a given type
      const TVirtualDestructor* get( const std::type_info& ti ) const;
      /// Add a new destructor object
      template< class T >
      void add();

   private:
      /// Hide the constructor of the type
      TDestructorRegistry();
      /// Hide the copy-constructor
      TDestructorRegistry( const TDestructorRegistry& ) = delete;

      /// Type used internally to clean up memory at the end of the process
      class TDestructorHolder {
      public:
         /// The managed object
         std::unique_ptr< TVirtualDestructor > m_destructor;
      };

      /// Type of the internal map
      typedef std::map< const std::type_info*, TDestructorHolder > Map_t;
      /// Internal map of known destructor objects
      Map_t m_types;
      /// Mutex for the destructor map
      mutable std::shared_timed_mutex m_mutex;

   }; // class TDestructorRegistry

} // namespace xAOD

// Include the template implementation:
#include "TDestructorRegistry.icc"

#endif // XAODROOTACCESS_TOOLS_TDESTRUCTORREGISTRY_H
