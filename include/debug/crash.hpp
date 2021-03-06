#ifndef _CRASH_HPP
#define _CRASH_HPP

#ifdef HAVE_CONFIG_H
# include <config.hpp>
#endif

/// \file crash.hpp
///
/// \brief Makes it possible to crash

#include <errno.h>

namespace grill
{
  /// Initialize the crasher
#define CRASH							\
  logger()<<grill::Crasher(__FILE__,__LINE__,__PRETTY_FUNCTION__)
  
  /// Keep all the information to crash
  class Crasher
  {
    /// File where the crash happened
    const char* path;
    
    /// Line number where the crash happened
    const int line;
    
    /// Function where the crash happened
    const char* funcName;
    
    /// Error from standard error library
    int errNo{-1};
    
  public:
    
    /// Creates the crasher
    Crasher(const char* path,
	    const int line,
	    const char* funcName)
      :
      path(path),
      line(line),
      funcName(funcName)
    {
    }
    
    /// Returns the path
    const char* getPath()
      const
    {
      return
	path;
    }
    
    /// Returns the line
    int getLine() const
    {
      return
	line;
    }
    
    /// Returns the function name
    const char* getFuncName() const
    {
      return
	funcName;
    }
    
    /// Returns the standard library error
    int getErrNo() const
    {
      return
	errNo;
    }
    
    /// Store the stdlib error
    Crasher& printStdLibErr()
    {
      errNo=
	errno;
      
      return
	*this;
    }
  };
}

#endif
