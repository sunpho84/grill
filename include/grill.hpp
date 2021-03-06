#ifndef _GRILL_HPP
#define _GRILL_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file grill.hpp

#include <debug/backtracing.hpp>
#include <debug/attachDebugger.hpp>
#include <debug/crash.hpp>
#include <debug/demangle.hpp>
#include <debug/minimalCrash.hpp>

#include <expr/assign/deviceAssign.hpp>
#include <expr/assign/directAssign.hpp>
#include <expr/assign/threadAssign.hpp>

#include <expr/comps/comp.hpp>
#include <expr/comps/compRwCl.hpp>
#include <expr/comps/indexComputer.hpp>

#include <expr/nodes/conj.hpp>
#include <expr/nodes/cWiseCombine.hpp>
#include <expr/nodes/dynamicTens.hpp>
#include <expr/nodes/node.hpp>
#include <expr/nodes/nodeCloser.hpp>
#include <expr/nodes/prod.hpp>
#include <expr/nodes/stackTens.hpp>
#include <expr/nodes/shift.hpp>
#include <expr/nodes/tens.hpp>
#include <expr/nodes/trace.hpp>
#include <expr/nodes/transp.hpp>

#include <expr/operations/dagger.hpp>
#include <expr/operations/reduce.hpp>

#include <lattice/fieldLayoutGetter.hpp>
#include <lattice/lattice.hpp>
#include <lattice/latticeCoverage.hpp>
#include <lattice/latticeGetter.hpp>
#include <lattice/latticeCoverageGetter.hpp>
#include <lattice/parityProvider.hpp>
#include <lattice/universe.hpp>

#include <ios/file.hpp>
#include <ios/logger.hpp>
#include <ios/minimalLogger.hpp>

#include <metaprogramming/arithmeticOperatorsViaCast.hpp>
#include <metaprogramming/inline.hpp>
#include <metaprogramming/asConstexpr.hpp>
#include <metaprogramming/unrolledFor.hpp>

#include <resources/aliver.hpp>
#include <resources/SIMD.hpp>

#include <tuples/tupleDiscriminate.hpp>
#include <tuples/tupleSubset.hpp>
#include <tuples/tupleCat.hpp>
#include <tuples/uniqueTuple.hpp>
#include <tuples/uniqueTupleFromTuple.hpp>

/// Global namespace
namespace grill
{
}

#endif
