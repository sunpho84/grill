#ifndef _TIMERGLOBALVARIABLESDECLARATIONS_HPP
#define _TIMERGLOBALVARIABLESDECLARATIONS_HPP

/// \file timerGlobalVariablesDeclarations.hpp

#ifdef HAVE_CONFIG_H
# include "config.hpp"
#endif

#include <metaprogramming/globalVariableProvider.hpp>
#include <resources/timer.hpp>

namespace grill
{
  DEFINE_OR_DECLARE_GLOBAL_VARIABLE(Timer,timings,("Total time",Timer::NO_FATHER,Timer::UNSTOPPABLE));
}

#endif
