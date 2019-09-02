/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AthenaMonitoring_MonitoredScalar_h
#define AthenaMonitoring_MonitoredScalar_h

#include <vector>

#include "AthenaMonitoring/IMonitoredVariable.h"

namespace Monitored {

  /**
   * Declare a monitored scalar
   *
   * Monitoring for any double-convertable scalar
   *
   * @param name            Name of monitored quantity
   * @param defaultValue    Default value assigned to the monitored scalar
   * @param valueTransform  Optional transformation applied to value
   *
   * \code
   *   auto phi = Monitored::Scalar("phi", 4.2);                                                        // deduced double
   *   auto eta = Monitored::Scalar<float>("eta", 0);                                                   // explicit float
   *   auto theta = Monitored::Scalar<double>("theta", 0.0, [](double value) { return value * 1000; }); // with transformation
   * \endcode
   */
  template <class T> class Scalar : public IMonitoredVariable {
  public:
    static_assert(std::is_convertible<T, double>::value, "Value must be convertable to double");

    Scalar(std::string name, const T& defaultValue = {}) :
        IMonitoredVariable(std::move(name)),
        m_value(defaultValue),
        m_valueTransform()
    {}
    Scalar(std::string name, const T& defaultValue, std::function<double(const T&)> valueTransform) :
        IMonitoredVariable(std::move(name)),
        m_value(defaultValue),
        m_valueTransform(valueTransform)
    {}
    Scalar(Scalar&&) = default;
    Scalar(Scalar const&) = delete;

    Scalar& operator=(Scalar const&) = delete;
    T operator=(T value)
    {
      m_value = value;
      return value;
    }

    operator T() const { return m_value; }
    operator T&() { return m_value; }

    // Needed to work around an apparent bug in clang 4.0.1.
    // Without these declarations, clang rejects `--SCALAR'
    // (but ++SCALAR, SCALAR++, and SCALAR-- are all accepted!).
    T operator--() { return --m_value; }
    T operator--(int) { return m_value--; }

    const std::vector<double> getVectorRepresentation() const override { 
      return { m_valueTransform ? m_valueTransform(m_value) : static_cast<double>(m_value) };
    }

  private:
    T m_value;
    std::function<double(const T&)> m_valueTransform;
  };

} // namespace Monitored

#endif /* AthenaMonitoring_MonitoredScalar_h */
