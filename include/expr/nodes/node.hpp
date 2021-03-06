#ifndef _NODE_HPP
#define _NODE_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file expr/nodes/node.hpp
///
/// \brief Declare base node of the syntactic tree

#include <type_traits>

#include <expr/nodes/bindComps.hpp>
#include <expr/comps/compLoops.hpp>
#include <expr/assign/deviceAssign.hpp>
#include <expr/assign/directAssign.hpp>
#include <expr/assign/executionSpace.hpp>
#include <expr/nodes/scalar.hpp>
#include <expr/nodes/nodeDeclaration.hpp>
#include <expr/assign/simdAssign.hpp>
#include <expr/assign/threadAssign.hpp>
#include <ios/logger.hpp>
#include <metaprogramming/crtp.hpp>
#include <tuples/tupleHasType.hpp>
#include <tuples/tupleFilter.hpp>

namespace grill
{
  namespace constraints
  {
    PROVIDE_HAS_MEMBER(getDynamicSizes);
    PROVIDE_HAS_MEMBER(Comps);
    PROVIDE_HAS_MEMBER(canAssign);
    PROVIDE_HAS_MEMBER(execSpace);
    PROVIDE_HAS_MEMBER(getRef);
    PROVIDE_HAS_MEMBER(eval);
    PROVIDE_HAS_MEMBER(storeByRef);
    PROVIDE_HAS_MEMBER(canSimdify);
    PROVIDE_HAS_MEMBER(SimdifyingComp);
    PROVIDE_HAS_MEMBER(hasDynamicComps);
    PROVIDE_HAS_MEMBER(canAssignAtCompileTime);
    PROVIDE_HAS_MEMBER(recreateFromExprs);
  }
  
  template <typename T>
  struct Node :
    DetectableAsNode
  {
    // /// Define the move-assignment operator
    // INLINE_FUNCTION
    // Node& operator=(Node&& oth)
    // {
    //   return this->operator=<T>(std::forward<Node>(oth));
    // }
    
    /// Returns whether can assign: this is actually used when no other assignability is defined
    constexpr bool canAssign() const
    {
      return false;
    }
    
    static constexpr bool canAssignAtCompileTime=false;
    
    /// Used to check that the derived type satisfy the Node criterion
    constexpr Node()
    {
      using namespace constraints;
      
      static_assert(hasMember_getDynamicSizes<T> and
		    hasMember_canAssign<T> and
		    hasMember_Comps<T> and
		    hasMember_execSpace<T> and
		    hasMember_eval<T> and
		    hasMember_getRef<T> and
		    hasMember_canSimdify<T> and
		    hasMember_SimdifyingComp<T> and
		    hasMember_canAssignAtCompileTime<T> and
		    hasMember_hasDynamicComps<T> and
		    hasMember_recreateFromExprs<T> and
		    hasMember_storeByRef<T>,
		    "Incomplete node type");
   }
    
    /// Assert assignability
    template <typename U>
    INLINE_FUNCTION
    constexpr void assertCanAssign(const Node<U>& _rhs)
    {
      //static_assert(tuplesContainsSameTypes<typename T::Comps,typename U::Comps>,"Cannot assign two expressions which differ for the components");
      
      static_assert(T::canAssignAtCompileTime,"Trying to assign to a non-assignable expression");
      
      auto& lhs=DE_CRTPFY(T,this);
      const auto& rhs=DE_CRTPFY(const U,&_rhs);
      
      if constexpr(not T::canAssignAtCompileTime)
	CRASH<<"Trying to assign to a non-assignable expression";
      
      if constexpr(U::hasDynamicComps)
	if(lhs.getDynamicSizes()!=rhs.getDynamicSizes())
	  CRASH<<"Dynamic comps not agreeing";
      
      static_assert(T::execSpace==U::execSpace or
		    U::execSpace==ExecSpace::HOST_DEVICE,"Cannot assign among different execution space, first change one of them");
    }
    
    /// Assign from another expression
    template <typename Rhs>
    constexpr INLINE_FUNCTION
    T& assign(const Node<Rhs>& u)
    {
      assertCanAssign(u);
      
      auto& lhs=DE_CRTPFY(T,this);
      const auto& rhs=DE_CRTPFY(const Rhs,&u);
      
#if ENABLE_SIMD
      if constexpr(T::canSimdify and
		   ((Rhs::canSimdify and std::is_same_v<typename T::SimdifyingComp,typename Rhs::SimdifyingComp>)
		    or not tupleHasType<typename Rhs::Comps,typename T::SimdifyingComp>))
	simdAssign(lhs,rhs);
      else
#endif
#if ENABLE_DEVICE_CODE
	if constexpr(Rhs::execSpace==ExecSpace::DEVICE)
	  deviceAssign(lhs,rhs);
	else
#endif
#if ENABLE_THREADS
	  if constexpr(Rhs::nDynamicComps==1)
	    threadAssign(lhs,rhs);
	  else
#endif
	    directAssign(lhs,rhs);
      
      return lhs;
    }
    
    /// Assign from another expression
    template <typename Rhs,
	      ENABLE_THIS_TEMPLATE_IF(not std::is_same_v<T,Rhs>)>
    constexpr INLINE_FUNCTION
    T& operator=(const Node<Rhs>& u)
    {
      return this->assign(u);
    }
    
    /// Assign from a scalar
    template <typename Oth,
	      ENABLE_THIS_TEMPLATE_IF(std::is_arithmetic_v<Oth>)>
    constexpr INLINE_FUNCTION
    T& operator=(const Oth& value)
    {
      return (*this)=scalar(value);
    }
    
    /// Define the assignment operator with the same expression type
    constexpr INLINE_FUNCTION
    Node& operator=(const Node& oth)
    {
      return this->assign(oth);
    }
    
    /// Returns the expression as a dynamic tensor
    auto fillDynamicTens() const;
    
    /// Close the expression into an appropriate tensor or field
    template <typename Res,
	      typename...Args>
    INLINE_FUNCTION
    Res closeAs(Args&&...args) const
    {
      Res res(std::forward<Args>(args)...);
      
      res=*this;
      
      return res;
    }
    
    /// Returns the closed expression
    auto close() const;
    
#define PROVIDE_SUBSCRIBE(ATTRIB)					\
    template <typename...C>						\
    constexpr INLINE_FUNCTION HOST_DEVICE_ATTRIB			\
    decltype(auto) operator()(const C&...cs) ATTRIB			\
    {									\
      using Comps=typename T::Comps;					\
									\
      using SubsComps=std::tuple<C...>;					\
									\
      static_assert((isComp<C> and...),"Trying to subscribe with non-comps arguments"); \
      									\
      static_assert(tupleHaveTypes<Comps,SubsComps>);			\
									\
      /*! Leftover components */					\
      using ResidualComps=						\
	TupleFilterAllTypes<Comps,SubsComps>;				\
      									\
      ATTRIB auto& t=DE_CRTPFY(ATTRIB T,this);				\
      									\
      if constexpr(std::tuple_size_v<ResidualComps> ==0)		\
	return t.eval(cs...);						\
      else								\
	return bindComps(t,std::make_tuple(cs...));			\
    }
    
    PROVIDE_SUBSCRIBE(const);
    
    PROVIDE_SUBSCRIBE(/* non const */);
    
#undef PROVIDE_SUBSCRIBE
  };

#define PROVIDE_CATCH_OP_WITH_ARITHMETIC(OP)				\
									\
  /* Catch operator OP between a node and an arithmetic type */		\
  template <typename T,							\
	    typename Oth,						\
	    ENABLE_THIS_TEMPLATE_IF(std::is_arithmetic_v<Oth>)>		\
  constexpr INLINE_FUNCTION						\
  auto operator OP(const Node<T>& node,const Oth& value)		\
  {									\
    return DE_CRTPFY(const T,&node) OP scalar(value);			\
  }									\
  									\
  /* Catch operator OP between an arithmetic type and a node */		\
  template <typename T,							\
	    typename Oth,						\
	    ENABLE_THIS_TEMPLATE_IF(std::is_arithmetic_v<Oth>)>		\
  constexpr INLINE_FUNCTION						\
  auto operator OP(const Oth& value,const Node<T>& node)		\
  {									\
    return scalar(value) OP DE_CRTPFY(const T,&node);			\
  }
  
  PROVIDE_CATCH_OP_WITH_ARITHMETIC(+);
  
  PROVIDE_CATCH_OP_WITH_ARITHMETIC(-);
  
  PROVIDE_CATCH_OP_WITH_ARITHMETIC(*);
  
  PROVIDE_CATCH_OP_WITH_ARITHMETIC(/);
  
  PROVIDE_CATCH_OP_WITH_ARITHMETIC(%);
  
  PROVIDE_CATCH_OP_WITH_ARITHMETIC(==);
  
  PROVIDE_CATCH_OP_WITH_ARITHMETIC(!=);
  
  PROVIDE_CATCH_OP_WITH_ARITHMETIC(and);
  
  PROVIDE_CATCH_OP_WITH_ARITHMETIC(or);
  
  PROVIDE_CATCH_OP_WITH_ARITHMETIC(xor);
  
#undef PROVIDE_CATCH_OP_WITH_ARITHMETIC
}

#endif
