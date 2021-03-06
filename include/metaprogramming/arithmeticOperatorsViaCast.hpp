#ifndef _ARITHMETICOPERATORSVIACAST_HPP
#define _ARITHMETICOPERATORSVIACAST_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file arithmeticOperatorsViaCast.hpp

#include <metaprogramming/crtp.hpp>
#include <metaprogramming/inline.hpp>

namespace grill
{
  /// Provides the arithmetic operators via cast
  template <typename CastToExec,
	    typename ReturnedType>
  struct ArithmeticOperators
  {
#define PROVIDE_POSTFIX_OPERATOR(OP)			\
    INLINE_FUNCTION constexpr HOST_DEVICE_ATTRIB	\
    ReturnedType& operator OP (int)			\
    {							\
      auto& This=DE_CRTPFY(ReturnedType,this);		\
      							\
      ((CastToExec&)This) OP;				\
      							\
      return This;					\
    }
    
    PROVIDE_POSTFIX_OPERATOR(++);
    PROVIDE_POSTFIX_OPERATOR(--);
    
#undef PROVIDE_POSTFIX_OPERATOR
    
#define PROVIDE_OPERATOR(OP,RETURNED_TYPE)				\
    									\
    INLINE_FUNCTION constexpr HOST_DEVICE_ATTRIB			\
    RETURNED_TYPE operator OP(const ArithmeticOperators& oth) const	\
    {									\
      const auto& This=DE_CRTPFY(const ReturnedType,this);		\
      									\
      const auto& Oth=DE_CRTPFY(const ReturnedType,&oth);		\
									\
      return ((const CastToExec&)This) OP ((const CastToExec&)Oth);	\
    }									\
    									\
    template <typename Oth>						\
    INLINE_FUNCTION constexpr HOST_DEVICE_ATTRIB			\
    RETURNED_TYPE operator OP(const Oth& oth) const			\
    {									\
      const auto& This=DE_CRTPFY(const ReturnedType,this);		\
      									\
      return ((const CastToExec&)This) OP ((const CastToExec&)oth);	\
    }
    
    PROVIDE_OPERATOR(+,ReturnedType);
    PROVIDE_OPERATOR(-,ReturnedType);
    PROVIDE_OPERATOR(*,ReturnedType);
    PROVIDE_OPERATOR(/,ReturnedType);
    PROVIDE_OPERATOR(%,ReturnedType);
    
    PROVIDE_OPERATOR(==,bool);
    PROVIDE_OPERATOR(!=,bool);
    PROVIDE_OPERATOR(<,bool);
    PROVIDE_OPERATOR(<=,bool);
    PROVIDE_OPERATOR(>,bool);
    PROVIDE_OPERATOR(>=,bool);
    
#undef PROVIDE_OPERATOR
    
#define PROVIDE_SELF_OPERATOR(OP)					\
    INLINE_FUNCTION constexpr HOST_DEVICE_ATTRIB			\
    ReturnedType& operator OP ##=(const ArithmeticOperators& oth)	\
    {									\
      const auto& Oth=DE_CRTPFY(const ReturnedType,&oth);		\
									\
      auto& This=DE_CRTPFY(ReturnedType,this);				\
									\
      (CastToExec&)This OP ## =(const CastToExec&)Oth;			\
									\
      return This;							\
    }
    
    PROVIDE_SELF_OPERATOR(+);
    PROVIDE_SELF_OPERATOR(-);
    PROVIDE_SELF_OPERATOR(*);
    PROVIDE_SELF_OPERATOR(/);
    PROVIDE_SELF_OPERATOR(%);
    
#undef PROVIDE_SELF_OPERATOR
  };
}

#endif
