#ifndef _TRANSPOSABLECOMP_HPP
#define _TRANSPOSABLECOMP_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file expr/comps/transposableComp.hpp

#include <expr/comps/compRwCl.hpp>

namespace grill
{
  namespace compFeat
  {
    enum class IsTransposable{FALSE,TRUE};
    
    template <IsTransposable _IsT,
	      typename _C>
    struct Transposable;
    
    template <RwCl _RC,
	      int _Which,
	      template <RwCl,int> typename _C>
    struct Transposable<IsTransposable::TRUE,_C<_RC,_Which>>
    {
      static constexpr bool isTransposable=true;
      
      static constexpr IsTransposable Transposability=IsTransposable::TRUE;
      
      static constexpr RwCl RC=_RC;
      
      static constexpr RwCl transpRc=transpRwCl<RC>;
      
      static constexpr int Which=_Which;
      
      using Transp=_C<transpRc,_Which>;
      
      static constexpr bool isMatr=true;
    };
    
    template <typename _C>
    struct Transposable<IsTransposable::FALSE,_C>
    {
      static constexpr bool isTransposable=false;
      
      static constexpr IsTransposable Transposability=IsTransposable::FALSE;
      
      using Transp=_C;
      
      static constexpr bool isMatr=false;
    };
  }
  
  namespace internal
  {
    /// Determine if a component is of row or column type
    template <typename T,
	      RwCl RC>
    static constexpr
    bool _transposableCompIsOfRwCl()
    {
      if constexpr(T::isTransposable)
	return (T::RC==RC);
      else
	return false;
    }
  }
  
  /// Determine if a component is of row type
  template <typename T>
  static constexpr bool isRow=
    internal::_transposableCompIsOfRwCl<T,RwCl::ROW>();
  
  /// Determine if a component is of cln type
  template <typename T>
  static constexpr bool isCln=
    internal::_transposableCompIsOfRwCl<T,RwCl::CLN>();
}

#endif
