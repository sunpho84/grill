#ifndef _SIMD_HPP
#define _SIMD_HPP

#ifdef HAVE_CONFIG_H
# include "config.hpp"
#endif

/// \file SIMD.hpp

#if ENABLE_SIMD
# include <immintrin.h>
#endif

#include <type_traits>
#include <utility>

#include <tuples/tupleCat.hpp>

constexpr bool haveAvx512Instructions=
#if ENABLE_SIMD and defined(HAVE_AVX512F_INSTRUCTIONS)
	    true
#else
	    false
#endif
	    ;

constexpr bool haveAvxInstructions=
#if ENABLE_SIMD and defined(HAVE_AVX_INSTRUCTIONS)
	    true
#else
	    false
#endif
	    ;

constexpr bool haveMmxInstructions=
#if ENABLE_SIMD and defined(HAVE_MMX_INSTRUCTIONS)
	    true
#else
	    false
#endif
	    ;

namespace esnort
{
  namespace internal
  {
#define CASE(SIZE,NAME,ELSE)				\
    if constexpr(Size% SIZE ==0)			\
      return NAME{};					\
    else						\
      ELSE						\
    
#define CASE_ELSE(SIZE,NAME,ELSE)		\
    ELSE
    
#if ENABLE_SIMD and defined(HAVE_AVX512F_INSTRUCTIONS)
# define CASE_AVX512 CASE
#else
# define CASE_AVX512 CASE_ELSE
#endif
    
#if ENABLE_SIMD and defined(HAVE_AVX_INSTRUCTIONS)
# define CASE_AVX CASE
#else
# define CASE_AVX CASE_ELSE
#endif

#if ENABLE_SIMD and defined(HAVE_MMX_INSTRUCTIONS)
# define CASE_MMX CASE
#else
# define CASE_MMX CASE_ELSE
#endif
    
#define CASES(TYPE,SIZE_PER_EL,SUFF,ELSE)				\
    if constexpr(std::is_same_v<F,TYPE>)				\
      {									\
	CASE_AVX512((4*SIZE_PER_EL),__m512 ## SUFF,			\
		    CASE_AVX((2*SIZE_PER_EL),__m256 ## SUFF,		\
			     CASE_MMX(SIZE_PER_EL,__m128 ## SUFF,)));	\
      }									\
    else								\
      ELSE;
    
    /// Provides the largest SIMD type supporting a vector of type F and size Size
    ///
    /// Internal implementation
    template <typename F,
	      int Size>
    constexpr auto _simdTypeProvider()
    {
      CASES(float,8,,
	    CASES(double,4,d,));
    }
    
#undef CASES
#undef CASE_MMX
#undef CASE_AVX
#undef CASE_AVX512
#undef CASE
#undef CASE_ELSE
  }
  
  /// Provides the largest SIMD type supporting a vector of type F and size Size
  template <typename F,
	    int Size>
  struct SimdOfTypeTraits
  {
    using type=decltype(internal::_simdTypeProvider<F,Size>());
    
    /// Check if there is a SIMD type for the asked type and size
    static constexpr bool canSimdify()
    {
      return Size!=0 and not std::is_same_v<void,type>;
    }
    
    /// Returns the non-simdified size
    static constexpr int nonSimdifiedSize()
    {
      if constexpr(canSimdify())
	return Size*sizeof(F)/sizeof(type);
      else
	return Size;
    }
  };
}

#endif
