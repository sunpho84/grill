#ifndef _AS_CONSTEXPR_HPP
#define _AS_CONSTEXPR_HPP

#ifdef HAVE_CONFIG_H
# include "config.hpp"
#endif

/// \file metaprogramming/constexpr.hpp

#include <type_traits>

namespace esnort
{
  /// Returns I wrapped in a std::integral_constant
  template <auto I>
  auto asConstexpr=
    std::integral_constant<decltype(I),I>{};
}

#endif
