#ifndef _CONSTNESS_CHANGER_HPP
#define _CONSTNESS_CHANGER_HPP

#ifdef HAVE_CONFIG_H
# include "config.hpp"
#endif

/// \file constnessChanger.hpp
///
/// \brief Provides several changes in the constness of a quantity

#include <metaprogramming/templateEnabler.hpp>

namespace grill
{
  /// Returns the argument as a constant
  template <typename T>
  constexpr const T& asConst(T& t) noexcept
  {
    return t;
  }
  
  /// Remove \c const qualifier from any reference
  template <typename T,
	    ENABLE_THIS_TEMPLATE_IF(not std::is_pointer<T>::value)>
  constexpr T& asMutable(const T& v) noexcept
  {
    return const_cast<T&>(v);
  }
  
  /// Remove \c const qualifier from any pointer
  template <typename T>
  constexpr T* asMutable(const T* v) noexcept
  {
    return (T*)(v);
  }
  
  /// Return the type T or const T if B is true
  template <bool B,
	    typename T>
  using ConstIf=
    std::conditional_t<B,const T,T>;
  
  /// Return the type T or T& if B is true
  template <bool B,
	    typename T>
  using RefIf=
    std::conditional_t<B,T&,T>;
  
  /// Removes RValue reference
  template <typename E>
  using RemoveRValueReference=
    RefIf<std::is_lvalue_reference<E>::value,std::remove_reference_t<E>>;
}

#endif
