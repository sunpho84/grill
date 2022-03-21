#ifndef _INLINER_HPP
#define _INLINER_HPP

#ifdef HAVE_CONFIG_H
# include "config.hpp"
#endif

/// \file inline.hpp
///
/// \brief Provides attribute to inline functions

/// Force the compiler to inline
///
/// \todo This is not very portable, let us investigate about other
/// compilers
#define INLINE_ATTRIBUTE			\
  __attribute__((always_inline))
  
/// Force the compiler to inline a function
#define INLINE_FUNCTION				\
  INLINE_ATTRIBUTE inline

/// Clang has its own way to express inline of a lambda
#ifdef __clang__
# define MUTABLE_INLINE_ATTRIBUTE INLINE_ATTRIBUTE mutable
#else
# define MUTABLE_INLINE_ATTRIBUTE mutable INLINE_ATTRIBUTE
#endif

#endif
