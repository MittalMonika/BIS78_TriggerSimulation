/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: DbInstanceCount.h 458019 2011-09-09 10:11:57Z mnowak $
//====================================================================
//  DbInstanceCount definition file
//====================================================================
#ifndef POOL_DBINSTANCECOUNT_H
#define POOL_DBINSTANCECOUNT_H 1

// C++ include files
#include <string>
#include <typeinfo>

/*
 *  POOL namespace
 */
namespace pool   {

  /** @class DbInstanceCount DbInstanceCount.h StorageSvc/DbInstanceCount.h
    *
    * Small class to enable object construction/destruction tracing
    *
    * @author  Markus Frank
    * @version 1.0
    */
  class DbInstanceCount  {
  public:
    /// Enumeration to steer the output
    enum {  NONE = 1<<0,
            STRING = 1<<1,
            TYPEINFO = 1<<2, 
            ALL=STRING|TYPEINFO
    };
    /** @class Counter
      *
      * Small class to enable object construction/destruction tracing.
      *
      * @author  Markus Frank
      * @version 1.0
      */
    class Counter  {
    private:
      /// Reference counter value
      long long int m_count;
      /// Increment counter value
      long long int m_tot;
    public:
      /// Default constructor
      Counter() : m_count(0), m_tot(0)    {                       }
      /// Copy constructor
      Counter(const Counter& c) 
      : m_count(c.m_count), m_tot(c.m_tot){                       }
      /// Destructor
      ~Counter()                          {                       }
      /// Increment counter
      void increment()                    {  ++m_count; ++m_tot;  }
      /// Decrement counter
      void decrement()                    {  --m_count;           }
      /// Access counter value
      long long int value() const      {  return m_count;      }
      /// Access counter value
      long long int total() const      {  return m_tot;        }
    };
  public:
    /// Standard Constructor - No need to call explicitly
    DbInstanceCount();
    /// Standard Destructor - No need to call explicitly
    virtual ~DbInstanceCount();
    /// Access counter object for local caching on optimizations
    static Counter* getCounter(const std::type_info& typ);
    /// Access counter object for local caching on optimizations
    static Counter* getCounter(const std::string& typ);
    /// Increment count according to type information
    template<class T> static void increment(T*) 
    { getCounter(typeid(T))->increment();                 }
    /// Decrement count according to type information
    template<class T> static void decrement(T*) 
    { getCounter(typeid(T))->decrement();                 }
    /// Access current counter
    template<class T> static long long int get(T*)
    { return getCounter(typeid(T))->value();              }
    /// Increment count according to type information
    static void increment(const std::type_info& typ)
    { getCounter(typ)->increment();                       }
    /// Decrement count according to type information
    static void decrement(const std::type_info& typ)
    { getCounter(typ)->decrement();                       }
    /// Access current counter
    static long long int get(const std::type_info& typ)
    { return getCounter(typ)->value();                    }
    /// Increment count according to string information
    static void increment(const std::string& typ)
    { getCounter(typ)->increment();                       }
    /// Decrement count according to string information
    static void decrement(const std::string& typ)
    { getCounter(typ)->decrement();                       }
    /// Access current counter
    static long long int get(const std::string& typ)
    { return getCounter(typ)->value();                    }
    /// Dump list of instance counters
    static void dump(int which=ALL);
    /// Clear list of instance counters
    static void clear(int which=ALL);
    /// Check if tracing is enabled.
    static bool doTrace();
    /// Enable/Disable tracing
    static void doTracing(bool value);
  };
}       // End namespace pool

#endif  // POOL_DBINSTANCECOUNT_H
