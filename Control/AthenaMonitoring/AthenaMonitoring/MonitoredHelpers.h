//
//  MonitoringHelpers.h
//  AthenaMonitoring
//
//  Created by Piotr Sarna on 03/04/2017.
//

#ifndef MonitoringHelpers_h
#define MonitoringHelpers_h

#include <type_traits>

namespace Monitored {
    namespace MonitoredHelpers {
        template <typename T>
        struct get_collection_values_type {
            typedef typename T::value_type value_type;
        };
        
        template <typename T, size_t N>
        struct get_collection_values_type<T[N]> {
            typedef T value_type;
        };
        
        template <typename, typename = void>
        struct has_double_operator {
            enum { value = false };
        };
        
        template <typename T>
        struct has_double_operator<T, decltype(void(double(std::declval<const T&>())))> {
            enum { value = true };
        };
        
        template <typename T>
        struct are_collection_values_have_double_operator {
            enum { value = has_double_operator<typename get_collection_values_type<T>::value_type>::value };
        };
    }
}

#endif /* MonitoringHelpers_h */
