#ifndef _INDEX_COMPUTER_HPP
#define _INDEX_COMPUTER_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file expr/comps/indexComputer.hpp
///
/// \brief Compute index given components

#include <expr/comps/comp.hpp>
#include <expr/comps/comps.hpp>
#include <metaprogramming/crtp.hpp>
#include <tuples/tupleDiscriminate.hpp>
#include <tuples/tupleSubset.hpp>
#include <tuples/uniqueTuple.hpp>

namespace grill
{
  /// Dispatch the internal index calculation
  ///
  /// This works when the passed components are already well ordered
  template <typename...D,
	    typename...C,
	    typename...Index>
  HOST_DEVICE_ATTRIB INLINE_FUNCTION
  auto index(const CompsList<D...>& dynamicSizes,
	     const BaseComp<C,Index>&...comps)
  {
    /// Returned type
    using GlbIndex=
      std::common_type_t<int,Index...>;
    
    /// Recursive computer
    auto index=
      [&dynamicSizes](const auto& index,
		      const GlbIndex& outer,
		      const auto& head,
		      const auto&...tail) INLINE_ATTRIBUTE
    {
      /// Type of the component
      using Head=
	std::decay_t<decltype(head)>;
      
      /// Maximal value
      GlbIndex size;
      
      if constexpr(Head::sizeIsKnownAtCompileTime)
	size=Head::sizeAtCompileTime;
      else
	size=~std::get<Head>(dynamicSizes);
      
      /// Value of the index when including this component
      const GlbIndex inner=
	outer*size+(~head);
      
      if constexpr(sizeof...(tail))
	return
	  index(index,inner,tail...);
      else
	return inner;
    };
    
    return index(index,0,DE_CRTPFY(const C,&comps)...);
  }
  
  template <typename T>
  struct StackIndTerm
  {
    const T value;
    
    constexpr INLINE_FUNCTION
    explicit StackIndTerm(const T& value) :
      value(value)
    {
    }
  };
  
  template <typename I,
	    typename T>
  constexpr INLINE_FUNCTION
  auto operator%(const I lhs,const StackIndTerm<T>& rhs)
  {
    return rhs.value+T::sizeAtCompileTime*lhs;
  }
  
  /// Dispatch the internal index calculation
  ///
  /// In this case we have no dynamic component
  template <typename...C,
	    typename...Index>
  constexpr HOST_DEVICE_ATTRIB INLINE_FUNCTION
  auto index(const std::tuple<>&,
	     const BaseComp<C,Index>&...comps)
  {
    /// Returned type
    using GlbIndex=
      std::common_type_t<int,Index...>;
    
    // /// Recursive computer
    // constexpr auto index=
    //   [](const auto& index,
    // 	 const GlbIndex& outer,
    // 	 const auto& head,
    // 	 const auto&...tail) INLINE_ATTRIBUTE
    // {
    //   /// Type of the component
    //   using Head=
    // 	std::decay_t<decltype(head)>;
      
    //   /// Maximal value
    //   constexpr GlbIndex size=Head::sizeAtCompileTime;
      
    //   /// Value of the index when including this component
    //   const GlbIndex inner=
    // 	outer*size+head();
      
    //   if constexpr(sizeof...(tail))
    // 	return
    // 	  index(index,inner,tail...);
    //   else
    // 	return inner;
    // };
    
    // return index(index,0,DE_CRTPFY(const C,&comps)...);
    return (GlbIndex(0)%...%StackIndTerm(DE_CRTPFY(const C,&comps)));
  }
  
  /// Returns the index after reordering elements
  template <typename...O,
	    typename DynamicComps,
	    typename...U>
  constexpr HOST_DEVICE_ATTRIB INLINE_FUNCTION
  auto orderedIndex(const DynamicComps& dynamicSizes,
		    const U&...cs)
  {
    const auto tmp=std::make_tuple(cs...);
    (void)&tmp;
    
    return index(dynamicSizes,std::get<O>(tmp)...);
  }
  
  namespace internal
  {
    /// Gets the maximal value for the given comp
    template <typename T,
	      typename DynamicComps>
    constexpr auto _getMaxCompValue(const DynamicComps& dynamicSizes)
    {
      if constexpr(T::sizeIsKnownAtCompileTime)
	return T::sizeAtCompileTime;
      else
	return ~std::get<T>(dynamicSizes);
    }
  }
  
  /// Computes the maximal value of an index
  template <typename...C,
	    typename DynamicComps>
  constexpr auto indexMaxValue(const DynamicComps& dynamicSizes)
  {
    return (internal::_getMaxCompValue<C>(dynamicSizes)*...*1);
  }
  
  /// Computes the maximal value of an index
  template <typename...C>
  constexpr auto indexMaxValue(const std::tuple<>& null={})
  {
    return (internal::_getMaxCompValue<C>(null)*...*1);
  }
}

#endif
