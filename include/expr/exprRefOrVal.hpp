#ifndef _EXPR_REF_OR_VAL_HPP
#define _EXPR_REF_OR_VAL_HPP

#include "metaprogramming/nonConstMethod.hpp"
#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file exprReforVal.hpp

#include <metaprogramming/constnessChanger.hpp>

namespace esnort
{
  namespace internal
  {
    /// Helper to determine all parameters to implement reference inside an expression
    template <typename ACTUAL_TYPE> /// corresponding to decltype(e)
    struct _ExprRefOrVal
    {
      using E=
	std::remove_const_t<std::decay_t<ACTUAL_TYPE>>;
      
      static constexpr bool storeByRef=
	std::is_lvalue_reference_v<ACTUAL_TYPE>;
      
      static constexpr bool needsToBeMoveConstructed=
	std::is_rvalue_reference_v<ACTUAL_TYPE>;
      
      static constexpr bool isVal=
	not std::is_reference_v<ACTUAL_TYPE>;
      
      static constexpr bool canBeMoveConstructed=
	std::is_move_constructible_v<E>;
      
      static constexpr bool canBeCopyConstructed=
	std::is_copy_constructible_v<E>;
      
      static constexpr bool passAsConst=
	std::is_const_v<std::remove_reference_t<ACTUAL_TYPE>>;
      
      static_assert(canBeMoveConstructed or not needsToBeMoveConstructed,"Would need to move-construct, but the move constructor is not available");
      
      static_assert(canBeCopyConstructed or not isVal,
		    "Would need to copy-construct, but the copy constructor is not available or the inner object must be stored by ref");
      
      using type=
	RefIf<storeByRef
	      ,ConstIf<passAsConst,E>>;
    };
  }
  
  /// Type used to nest an expression
  ///
  /// The passed type _E must correspond to decltype(e)
  template <typename _E>
  using ExprRefOrVal=
    typename internal::_ExprRefOrVal<_E>::type;
}

#endif