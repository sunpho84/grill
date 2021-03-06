#ifndef _TUPLEDISCRIMINATE_HPP
#define _TUPLEDISCRIMINATE_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file tupleDiscriminate.hpp

#include <tuple>

namespace grill
{
  namespace internal
  {
    /// Result of discriminating a single type
    ///
    /// Forward declaration
    template <template <typename> typename F,
	      bool IsValid,
	      typename T,
	      typename V,
	      typename I>
    struct _TupleDiscriminateRes;
    
    /// Discriminates a single type
    ///
    /// Forward declaration
    template <template <typename> typename F,
	      typename T,
	      typename V,
	      typename I>
    struct _TupleDiscriminate;
    
    /// Discriminates a single type
    ///
    /// Endcase
    template <template <typename> typename F,
	      typename V,
	      typename I>
    struct _TupleDiscriminate<F,std::tuple<>,V,I>
    {
      using Valid=V;
      
      using Invalid=I;
    };
    
    /// Iteratively discriminate all types
    template <template <typename> typename F,
	      typename Head,
	      typename...Tail,
	      typename V,
	      typename I>
    struct _TupleDiscriminate<F,std::tuple<Head,Tail...>,V,I> :
      _TupleDiscriminateRes<F,F<Head>::value,std::tuple<Head,Tail...>,V,I>
    {
    };
    
    /// Result of discriminating a single type
    ///
    /// False case
    template <template <typename> typename F,
	      typename Head,
	      typename...Tail,
	      typename...V,
	      typename...I>
    struct _TupleDiscriminateRes<F,
			    false,
			    std::tuple<Head,Tail...>,
			    std::tuple<V...>,
			    std::tuple<I...>> :
      _TupleDiscriminate<F,std::tuple<Tail...>,std::tuple<V...>,std::tuple<I...,Head>>
    {
    };
    
    /// Result of discriminating a single type
    ///
    /// True case
    template <template <typename> typename F,
	      typename Head,
	      typename...Tail,
	      typename...V,
	      typename...I>
    struct _TupleDiscriminateRes<F,
			    true,
			    std::tuple<Head,Tail...>,
			    std::tuple<V...>,
			    std::tuple<I...>> :
      _TupleDiscriminate<F,std::tuple<Tail...>,std::tuple<V...,Head>,std::tuple<I...>>
    {
    };
  }
  
  /// Discriminates elements of a tuple on the basis of a predicate
  template <template <typename> typename F,
	    typename T>
  using TupleDiscriminate=
    internal::_TupleDiscriminate<F,T,std::tuple<>,std::tuple<>>;
}

#endif
