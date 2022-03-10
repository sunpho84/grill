#ifndef _EXPR_HPP
#define _EXPR_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file expr/expr.hpp
///
/// \brief Declare base expression, to issue the assignments

#include <type_traits>

#include <expr/executionSpace.hpp>
#include <ios/logger.hpp>
#include <metaprogramming/crtp.hpp>
#include <tuples/tupleHasType.hpp>
#include <tuples/tupleFilter.hpp>

namespace esnort
{
  DEFINE_CRTP_INHERITANCE_DISCRIMINER_FOR_TYPE(Expr)
  
  /// Base type representing an expression
  template <typename T>
  struct Expr :
    Crtp<T,crtp::ExprDiscriminer>
  {
    /// Define the assignment operator with the same expression type,
    /// in terms of the templated version
    Expr& operator=(const Expr& oth)
    {
      return this->operator=<T>(oth);
    }
    
    /// Assert assignability
    template <typename U>
    constexpr void assertCanAssign(const Expr<U>& rhs)
    {
      static_assert(tuplesContainsSameTypes<typename T::Comps,typename U::Comps>,"Cannot assign two expressions which differ for the components");
      
      if constexpr(U::hasDynamicComps)
	if(this->crtp().getDynamicSizes()!=rhs.crtp().getDynamicSizes())
	  CRASH<<"Dynamic comps not agreeing";
    }
    
    /// Assign from another expression
    template <typename Lhs>
    T& operator=(const Expr<Lhs>& u)
    {
      assertCanAssign(u);
      
      /// Gets the lhs by casting to actual type
      decltype(auto) lhs=this->crtp();
      
      /// Gets the rhs by casting to actual type
      decltype(auto) rhs=u.crtp();
      
      if constexpr(T::execSpace==Lhs::execSpace)
	{
	  #warning lhs()=rhs();
	}
      else
	{
	  /// Decide which side of the assignment will change the
	  /// execution space. This is done in terms of an euristic cost
	  /// of the change, but we might refine it in the future. The
	  /// assumption is that ultimately, the results will be stored in
	  /// any case on the lhs execution space, so we should avoid
	  /// moving the lhs unless it is much lest costly.
	  if constexpr(T::execSpaceChangeCost<Lhs::execSpaceChangeCost)
	    {
	      LOGGER<<"Needs to change the execution space of Lhs before assigning";
	    }
	  else
	    {
#warning messagelhs()=rhs.template changeExecSpaceTo<lhsExecSpace>()();
	      LOGGER<<"Needs to change the execution space of Rhs before assigning";
	    }
	}
      
      return this->crtp();
    }
    
    /// Returns the expression as a dynamic tensor
    auto fillDynamicTens() const;
    
#define PROVIDE_SUBSCRIBE(ATTRIB)					\
    template <typename...C>						\
    HOST_DEVICE_ATTRIB constexpr INLINE_FUNCTION			\
    decltype(auto) operator()(const C&...cs) ATTRIB			\
    {									\
      /*! Leftover components */					\
      using ResidualComps=						\
	TupleFilterAllTypes<typename T::Comps,std::tuple<C...>>;	\
									\
      if constexpr(std::tuple_size_v<ResidualComps> ==0)		\
	return this->crtp().eval(cs...);				\
      else								\
	return compBind(this->crtp(),std::make_tuple(cs...));		\
    }
    
    PROVIDE_SUBSCRIBE(const);
    
    PROVIDE_SUBSCRIBE(/* non const */);
    
#undef PROVIDE_SUBSCRIBE
  };
}

#endif
