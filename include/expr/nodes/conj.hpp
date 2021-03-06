#ifndef _CONJ_HPP
#define _CONJ_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file expr/nodes/conj.hpp

#include <expr/comps/comp.hpp>
#include <expr/comps/comps.hpp>
#include <expr/nodes/node.hpp>
#include <expr/nodes/subNodes.hpp>

namespace grill
{
  DECLARE_UNTRANSPOSABLE_COMP(ComplId,int,2,reIm);
  
  PROVIDE_DETECTABLE_AS(Conjugator);
  
  /// Conjugator
  ///
  /// Forward declaration to capture the components
  template <typename _E,
	    typename _Comps,
	    typename _Fund>
  struct Conjugator;
  
#define THIS					\
  Conjugator<std::tuple<_E...>,CompsList<C...>,_Fund>

#define BASE					\
    Node<THIS>
  
  /// Conjugator
  ///
  template <typename..._E,
	    typename...C,
	    typename _Fund>
  struct THIS :
    DynamicCompsProvider<CompsList<C...>>,
    DetectableAsConjugator,
    SubNodes<_E...>,
    BASE
  {
    /// Import the base expression
    using Base=BASE;
    
    using This=THIS;
    
#undef BASE
    
#undef THIS
    
    static_assert(sizeof...(_E)==1,"Expecting 1 argument");
    
    IMPORT_SUBNODE_TYPES;
    
    /// Components
    using Comps=
      CompsList<C...>;
    
    /// Fundamental tye
    using Fund=_Fund;
    
    /// Executes where allocated
    static constexpr ExecSpace execSpace=
      SubNode<0>::execSpace;
    
    /// Returns the dynamic sizes
    INLINE_FUNCTION HOST_DEVICE_ATTRIB
    decltype(auto) getDynamicSizes() const
    {
      return SUBNODE(0).getDynamicSizes();
    }
    
    /// Returns whether can assign
    INLINE_FUNCTION
    bool canAssign()
    {
      return false;
    }
    
    /// Return whether can be assigned at compile time
    static constexpr bool canAssignAtCompileTime=false;
    
    /// This is a lightweight object
    static constexpr bool storeByRef=false;
    
    /// Import assignment operator
    using Base::operator=;
    
    /// States whether the tensor can be simdified
    static constexpr bool canSimdify=
      SubNode<0>::canSimdify and
      not std::is_same_v<ComplId,typename SubNode<0>::SimdifyingComp>;
    
    /// Components on which simdifying
    using SimdifyingComp=
      std::conditional_t<canSimdify,typename SubNode<0>::SimdifyingComp,void>;
    
#define PROVIDE_SIMDIFY(ATTRIB)					\
    /*! Returns a ATTRIB simdified view */			\
    INLINE_FUNCTION						\
    auto simdify() ATTRIB					\
    {								\
      return conj(SUBNODE(0).simdify());			\
    }
    
    PROVIDE_SIMDIFY(const);
    
    PROVIDE_SIMDIFY(/* non const */);
    
#undef PROVIDE_SIMDIFY
    
    /////////////////////////////////////////////////////////////////
    
    //// Returns a conjugator on a different expression
    template <typename T>
    INLINE_FUNCTION
    decltype(auto) recreateFromExprs(T&& t) const
    {
      return conj(std::forward<T>(t));
    }
    
    /////////////////////////////////////////////////////////////////
    
#define PROVIDE_GET_REF(ATTRIB)					\
    /*! Returns a reference */					\
    INLINE_FUNCTION						\
    auto getRef() ATTRIB					\
    {								\
      return conj(SUBNODE(0).getRef());				\
    }
    
    PROVIDE_GET_REF(const);
    
    PROVIDE_GET_REF(/* non const */);
    
#undef PROVIDE_GET_REF
    
    /// Evaluate
    template <typename...TD>
    HOST_DEVICE_ATTRIB INLINE_FUNCTION constexpr
    Fund eval(const TD&...td) const
    {
      /// Compute the real or imaginary component
      const ComplId reIm= //don't take as ref, it messes up
	std::get<ComplId>(std::make_tuple(td...));
      
      /// Nested result
      decltype(auto) nestedRes=
	SUBNODE(0)(td...);
      
      if(reIm==0)
	return nestedRes;
      else
       	return -nestedRes;
    }
    
    /// Construct
    template <typename T>
    HOST_DEVICE_ATTRIB INLINE_FUNCTION constexpr
    Conjugator(T&& arg,
	       UNIVERSAL_CONSTRUCTOR_IDENTIFIER) :
      SubNodes<_E...>(std::forward<T>(arg))
    {
    }
  };
  
  /// Conjugate an expression
  template <typename _E,
	    ENABLE_THIS_TEMPLATE_IF(isNode<_E>)>
  HOST_DEVICE_ATTRIB INLINE_FUNCTION constexpr
  decltype(auto) conj(_E&& e)
  {
    /// Base passed type
    using E=
      std::decay_t<_E>;
    
    if constexpr(isConjugator<E>)
      return e.template subNode<0>;
    else
      {
	/// Components
	using Comps=
	  typename E::Comps;
	
	if constexpr(not tupleHasType<Comps,ComplId>)
	  return RemoveRValueReference<_E>(e);
	else
	  {
	    /// Type returned when evaluating the expression
	    using Fund=
	      typename E::Fund;
	    
	    return
	      Conjugator<std::tuple<_E>,Comps,Fund>(std::forward<_E>(e),UNIVERSAL_CONSTRUCTOR_CALL);
	  }
      }
  }
  
#define FOR_REIM_PARTS(NAME)		\
  FOR_ALL_COMPONENT_VALUES(ComplId,NAME)
  
  /// Real component index - we cannot rely on a constexpr inline as the compiler does not propagate it correctly
#define Re ComplId(0)
  
  /// Imaginary component index
#define Im ComplId(1)
  
  /// Returns the real part, subscribing the complex component to Re value
  template <typename _E>
  HOST_DEVICE_ATTRIB INLINE_FUNCTION constexpr
  decltype(auto) real(_E&& e)
  {
    return
      e(Re);
  }
  
  /// Returns the imaginary part, subscribing the complex component to Im value
  template <typename _E>
  HOST_DEVICE_ATTRIB INLINE_FUNCTION constexpr
  decltype(auto) imag(_E&& e)
  {
    return
      e(Im);
  }
}

#endif
