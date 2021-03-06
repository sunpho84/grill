#ifndef _COMP_HPP
#define _COMP_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file expr/comps/comp.hpp

#include <expr/comps/baseComp.hpp>
#include <expr/comps/compRwCl.hpp>
#include <expr/comps/transposableComp.hpp>
#include <metaprogramming/detectableAs.hpp>

namespace grill
{
  PROVIDE_DETECTABLE_AS(Comp);
  
  /// Component
  template <compFeat::IsTransposable IsTransposable,
	    typename Index,
	    typename Derived>
  struct Comp  :
    compFeat::Transposable<IsTransposable,Derived>,
    DetectableAsComp,
    BaseComp<Derived,Index>
  {
    using Base=BaseComp<Derived,Index>;
    
    using Base::Base;
  };
  
  /// Predicate if a certain component has known size at compile time
  template <typename T>
  struct SizeIsKnownAtCompileTime
  {
    static constexpr bool value=T::sizeIsKnownAtCompileTime;
  };
  
  /// Promotes the argument i to a component of type TYPE
#define DECLARE_COMPONENT_FACTORY(FACTORY,TYPE)			\
  template <typename T>						\
  INLINE_FUNCTION constexpr HOST_DEVICE_ATTRIB			\
  TYPE FACTORY(T&& i)						\
  {								\
    return i;							\
  }
  
  PROVIDE_DETECTABLE_AS(TransposableComp);
  
#define DECLARE_TRANSPOSABLE_COMP(NAME,TYPE,SIZE,FACTORY)	\
  template <RwCl _RC=RwCl::ROW,					\
	    int _Which=0>					\
  struct NAME ## RwOrCl:					\
    DetectableAsTransposableComp,				\
    Comp<compFeat::IsTransposable::TRUE,			\
	 TYPE,							\
	 NAME ## RwOrCl<_RC,_Which>>				\
  {								\
    using Base=							\
      Comp<compFeat::IsTransposable::TRUE,			\
      TYPE,							\
      NAME ## RwOrCl<_RC,_Which>>;				\
    								\
    using Base::Base;						\
    								\
    static constexpr int sizeAtCompileTime=SIZE;		\
  };								\
  								\
  using NAME ## Row=NAME ## RwOrCl<RwCl::ROW,0>;		\
  								\
  using NAME=NAME ## Row;					\
  								\
  using NAME ## Cln=NAME ## RwOrCl<RwCl::CLN,0>;		\
  								\
  DECLARE_COMPONENT_FACTORY(FACTORY ## Row,NAME ## Row);	\
								\
  DECLARE_COMPONENT_FACTORY(FACTORY ## Cln,NAME ## Cln);	\
								\
  DECLARE_COMPONENT_FACTORY(FACTORY,NAME)
  
#define DECLARE_UNTRANSPOSABLE_COMP(NAME,TYPE,SIZE,FACTORY)	\
  struct NAME :							\
    Comp<compFeat::IsTransposable::FALSE,			\
	 TYPE,							\
	 NAME>							\
  {								\
    using Base=							\
      Comp<compFeat::IsTransposable::FALSE,			\
      TYPE,							\
      NAME>;							\
								\
    using Base::Base;						\
								\
    static constexpr int sizeAtCompileTime=SIZE;		\
  };								\
								\
  DECLARE_COMPONENT_FACTORY(FACTORY,NAME)
  
  /////////////////////////////////////////////////////////////////
  
  template <typename T,
	    ENABLE_THIS_TEMPLATE_IF(isTransposableComp<T>)>
  typename T::Transp transp(const T& t)
  {
    return ~t;
  }
  
  template <typename T,
	    ENABLE_THIS_TEMPLATE_IF(not isTransposableComp<T>)>
  const T& transp(const T& t)
  {
    return t;
  }
  
  template <typename T>
  using Transp=decltype(transp(std::declval<T>()));
}

#endif
