#ifndef _MINIMALLOGGER_HPP
#define _MINIMALLOGGER_HPP

#ifdef HAVE_CONFIG_H
# include "config.hpp"
#endif

/// \file minimalLogger.hpp
///
/// \brief Minimal version of the logger
///
/// This file can be included in place of Logger.hpp, in case the
/// former cannot be included due to missing requirements, e.g. in
/// presence of routine from which the actual logger depends, such as
/// threads or MPI. The actual logger is called in the cpp file

#include <ios/file.hpp>

namespace grill
{
  /// Wraps the actual logger through the old-style C variadic function
  ///
  /// We cannot use variadic template version here, as the logger cannot be called directly here, since "Logger.hpp" is expected
  void minimalLogger(const char* format,       ///< Formatting string
		     ...);
}

#endif
