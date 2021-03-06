#ifndef _TENSREF_HPP
#define _TENSREF_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file expr/nodes/tensRef.hpp

#include <expr/assign/executionSpace.hpp>
#include <expr/comps/comps.hpp>
#include <expr/comps/indexComputer.hpp>
#include <expr/nodes/baseTens.hpp>
#include <expr/nodes/node.hpp>

namespace grill
{
  PROVIDE_DETECTABLE_AS(TensRef);
  
  /// Tensor reference
  ///
  /// Forward declaration
  template <typename C,
	    typename F,
	    ExecSpace ES>
  struct TensRef;
  
#define THIS					\
  TensRef<CompsList<C...>,_Fund,ES>
  
#define BASE					\
  BaseTens<THIS,CompsList<C...>,_Fund,ES>
  
  /// Tensor reference
  template <typename...C,
	    typename _Fund,
	    ExecSpace ES>
  struct THIS :
    BASE,
    DetectableAsTensRef
  {
    using This=THIS;
    using Base=BASE;
    
#undef BASE
#undef THIS
    
    /// Importing assignment operator from BaseTens
    using Base::operator=;
    
    /// Copy assign: we still assign the content
    INLINE_FUNCTION
    TensRef& operator=(const TensRef& oth)
    {
      Base::operator=(oth);
      
      return *this;
    }
    
    /// Move assign
    INLINE_FUNCTION
    TensRef& operator=(TensRef&& oth)
    {
      std::swap(this->storage,oth.storage);
      std::swap(this->dynamicSizes,oth.dynamicSizes);
      
      return *this;
    }
    
    /// List of dynamic comps
    using DynamicComps=
      typename Base::DynamicComps;
    
    /// Components
    using Comps=CompsList<C...>;
    
    /// Fundamental type
    using Fund=_Fund;
    
    /// Executes where originally allocated
    static constexpr ExecSpace execSpace=ES;
    
    /// Sizes of the dynamic components
    DynamicComps dynamicSizes;
    
    /// Returns the dynamic sizes
    INLINE_FUNCTION HOST_DEVICE_ATTRIB constexpr
    const DynamicComps& getDynamicSizes() const
    {
      return dynamicSizes;
    }
    
    /// Pointer to storage
    Fund* const storage;
    
    /// Number of elements
    const int64_t nElements;
    
    /// Returns whether can assign: we must assume that the reference is pointing to a valid pointer
    bool canAssign()
    {
      return true;
    }
    
    /// We can safely copy since it is a light object
    static constexpr bool storeByRef=false;
    
    /// Initialize the reference
    INLINE_FUNCTION HOST_DEVICE_ATTRIB constexpr
    TensRef(Fund* storage,
	    const int64_t& nElements,
	    const DynamicComps& dynamicSizes) :
      dynamicSizes(dynamicSizes),
      storage(storage),
      nElements(nElements)
    {
    }
    
    /// Copy constructor
    INLINE_FUNCTION HOST_DEVICE_ATTRIB
    TensRef(const TensRef& oth) :
      TensRef(oth.storage,oth.nElements,oth.dynamicSizes)
    {
    }
    
    // /Move constructor
    INLINE_FUNCTION HOST_DEVICE_ATTRIB
    TensRef(TensRef&& oth) :
      TensRef(oth.storage,oth.nElements,oth.dynamicSizes)
    {
    }
    
    /////////////////////////////////////////////////////////////////
    
#define PROVIDE_EVAL(ATTRIB)						\
    template <typename...U>						\
    HOST_DEVICE_ATTRIB constexpr INLINE_FUNCTION			\
    ATTRIB Fund& eval(const U&...cs) ATTRIB				\
    {									\
      assertCorrectEvaluationStorage<ES>();				\
      									\
      /* const int64_t d[]={cs()...};				     */	\
      /* const char* n[]={typeid(cs).name()...};		     */	\
      /* 							     */	\
      /* LOGGER<<"Evaluating acces:";				     */	\
      /* for(size_t i=0;i<sizeof...(cs);i++)			     */	\
      /* LOGGER<<" "<<demangle(n[i])<<" "<<d[i];		     */	\
      /* forEachInTuple(this->dynamicSizes,[](const auto c)	     */	\
      /* {							     */	\
      /* LOGGER<<"Dynamic size "<<typeid(c).name()<<" "<<c;	     */	\
      /* });							     */	\
      									\
      const auto id=orderedIndex<C...>(this->dynamicSizes,cs...);	\
      									\
      /* LOGGER<<" id: "<<id;					     */	\
      									\
      return storage[id];						\
    }
    
    PROVIDE_EVAL(const);
    
    PROVIDE_EVAL(/* non const */);
    
#undef PROVIDE_EVAL
  };
}

#endif
