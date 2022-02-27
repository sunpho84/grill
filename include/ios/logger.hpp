#ifndef _LOGGER_HPP
#define _LOGGER_HPP

#ifdef HAVE_CONFIG_H
# include "config.hpp"
#endif

/// \file logger.hpp
///
/// \brief Header file to define a logger wrapping FILE*
///
/// The internal class is used to really print, while the external one
/// to determine whether to print or not, and to lock in case the
/// threads are present. If printing is not needed, a fake internal
/// logger is passed, printing on /dev/null
///
/// If threads are running, and all threads are allowed to print, the
/// Logger is locked so that only one thread at the time can print,
/// and all lines are prepended with thread id indication.
///
/// If MPI is running, and all ranks are allowed, to print, each line
/// is prepended with the rank id. No cross-ranks lock is issued.

#include <cstdio>

#include <debug/backtracing.hpp>
#include <debug/crash.hpp>
#include <ios/file.hpp>
#include <ios/scopeFormatter.hpp>
#include <ios/textFormat.hpp>
#include <metaprogramming/operatorExists.hpp>
#include <system/Mpi.hpp>
#include <system/timer.hpp>
#include <resources/scopeDoer.hpp>

namespace esnort
{
  DEFINE_BINARY_OPERATOR_IMPLEMENTATION_CHECK(canPrint,CanPrint,<<);
  
  /// Write output to a file, using different level of indentation
  class Logger :
    private File
  {
    /// Access to the logger as if it was a file
    const File& file()
      const
    {
      return
	static_cast<const File&>(*this);
    }
    
    PROVIDE_ALSO_NON_CONST_METHOD(file);
    
    /// Single line in the logger
    class LoggerLine
    {
      /// Store wether the line has to be ended or not
      bool hasToEndLine;
      
      /// Store whether the line is crashing
      bool hasToCrash;
      
      /// Mark that the color has changed in this line
      bool colorChanged;
      
      /// Mark that the style has changed in this line
      bool styleChanged;
      
      /// Decide whether to prepend the rank id on the line
      const bool prependRank;
      
      /// Decide whether to prepend the thread id on the line
      const bool prependThread;
      
      /// Check whether we need to lock
      const bool hasToLock;
      
      /// Reference to the logger
      Logger& logger;
      
      /// Forbids copying a line
      LoggerLine(const LoggerLine&)=delete;
      
    public:
      
      /// Starts a new line
      void startNewLine()
      {
	/// Total number of the character written
	int rc=
	  0;
	
 	// Prepend with time
	if(logger.prependTime)
	  {
	    SCOPE_REAL_FORMAT_FIXED();
	    SCOPE_REAL_PRECISION(10);
	    SCOPE_ALWAYS_PRINT_ZERO();
	    SCOPE_NOT_ALWAYS_PUT_SIGN();
	    rc+=
	      (logger.file()<<durationInSec(timings.currentMeasure())<<" s").getRc();
	  }
	
	// Prepend with rank
	if(prependRank)
	  rc+=
	    (logger.file()<<" Rank "<<mpi.rank()).getRc();
	
	// Prepend with thread
#warning messagewarning
	// if(someOtherThreadCouldBePrinting)
	//   rc+=
	//     (logger.file()<<" Thread "<<threads.getThreadId()).getRc();
	
	// Mark the margin
	if(rc)
	  logger.file()<<":\t";
	
	// Writes the given number of spaces
	for(int i=0;i<logger.indentLev;i++)
	  logger.file()<<' ';
      }
      
      /// Construct
      LoggerLine(Logger& logger)
	: hasToEndLine(true),
	  hasToCrash(false),
	  colorChanged(false),
	  styleChanged(false),
	  prependRank(mpi.nRanks()!=1 and not onlyMasterRankPrint),
	  prependThread(1 and // threads.nActiveThreads()!=1 and 
			not onlyMasterThreadPrint),
	  hasToLock(prependThread),
	  logger(logger)
      {
	
	// if(hasToLock)
	//   logger.getExclusiveAccess();
	  
	startNewLine();
      }
      
      /// Move constructor
      LoggerLine(LoggerLine&& oth)
      	: hasToEndLine(true),
      	  hasToCrash(oth.hasToCrash),
	  colorChanged(oth.colorChanged),
	  styleChanged(oth.styleChanged),
	  prependRank(oth.prependRank),
	  prependThread(oth.prependThread),
	  hasToLock(oth.hasToLock),
      	  logger(oth.logger)
      {
      	oth.hasToEndLine=
      	  false;
      }
      
      /// Ends the line
      void endLine()
      {
	logger.file()<<'\n';
      }
      
      /// Destroy (end the line)
      ~LoggerLine()
      {
	// Wrap everything here
	if(hasToEndLine)
	  {
	    // Ends the quoted text
	    if(hasToCrash)
	      *this<<"\"";
	    
	    // Reset color
	    if(colorChanged)
	      *this<<TextColor::DEFAULT;
	    
	    // Reset style
	    if(styleChanged)
	      *this<<TextStyle::RESET;
	    
	    // Ends the line
	    endLine();
	    
	    // if(hasToLock)
	    //   logger.releaseExclusiveAccess();
	    
	    if(hasToCrash)
	      {
		printBacktraceList();
		exit(1);
	      }
	  }
      }
      
      /// Prints after putting a space
      template <typename T>               // Type of the obected to print
      LoggerLine& operator*(T&& t)      ///< Object to be printed
      {
	logger.file()*std::forward<T>(t);
	
	return
	  *this;
      }
      
      /// Catch-all print
      ///
      /// The SFINAE is needed to avoid that the method is used when
      /// File does not know how to print
      template <typename T,                                           // Type of the quantity to print
		ENABLE_THIS_TEMPLATE_IF(not canPrint<LoggerLine,T>    // SFINAE needed to avoid ambiguous overload
					and canPrint<File,const T&>)>
      LoggerLine& operator<<(const T& t)                             ///< Object to print
      {
	logger.file()<<t;
	
	return
	  *this;
      }
      
      /// Print a C-style variadic message
      LoggerLine& printVariadicMessage(const char* format, ///< Format to print
				       va_list ap)         ///< Variadic part
      {
	logger.file().printVariadicMessage(format,ap);
	
	return
	  *this;
      }
      
      /// Changes the color of the line
      LoggerLine& operator<<(const TextColor& c)
      {
	colorChanged=
	  true;
	
	return
	  *this<<
	    TEXT_CHANGE_COLOR_HEAD<<
	    static_cast<char>(c)<<
	    TEXT_CHANGE_COLOR_TAIL;
      }
      
      /// Changes the style of the line
      LoggerLine& operator<<(const TextStyle& c)
      {
	styleChanged=
	  true;
	
	return
	  *this<<
	    TEXT_CHANGE_STYLE_HEAD<<
	    static_cast<char>(c)<<
	    TEXT_CHANGE_STYLE_TAIL;
      }
      
      /// Prints crash information
      ///
      /// Then sets the flag \c hasToCrash to true, such that at
      /// destroy of the line, crash happens
      LoggerLine& operator<<(const Crasher& cr)
      {
	this->hasToCrash=
	  true;
	
	(*this)<<TextColor::RED<<" ERROR in function "<<cr.getFuncName()<<" at line "<<cr.getLine()<<" of file "<<cr.getPath()<<": \"";
	
	return
	  *this;
      }
      
      /// Prints a string, parsing newline
      LoggerLine& operator<<(const char* str)
      {
	if(str==nullptr)
	  logger.file()<<str;
	else
	  {
	    /// Pointer to the first char of the string
	    const char* p=
	      str;
	    
	    // Prints until finding end of string
	    while(*p!='\0')
	      {
		// starts a new line
		if(*p=='\n')
		  {
		    endLine();
		    startNewLine();
		  }
		else
		  // Prints the char
		  *this<<*p;
		
		// Increment the char
		p++;
	      }
	  }
	
	return
	  *this;
      }
      
      /// Prints a c++ string
      LoggerLine& operator<<(const std::string& str)
      {
	return
	  *this<<str.c_str();
      }
      
    };
    
    /// Indentation level
    int indentLev{0};
    
    /// Determine wheter the new line includes time
    bool prependTime;
    
    // /// Mutex used to lock the logger
    // mutable Mutex mutex;
    
    // /// Set the exclusive access right
    // void getExclusiveAccess()
    // {
    //   mutex.lock();
    // }
    
    /// Release the exclusive access right
    // void releaseExclusiveAccess()
    // {
    //   mutex.unlock();
    // }
    
  public:
    
    ///Verbosity level
    static int verbosityLv;
    
    /// Decide whether only master thread can write here
    static bool onlyMasterThreadPrint;
    
    /// Decide whether only master MPI can write here
    static bool onlyMasterRankPrint;
    
    using File::alwaysPrintSign;
    using File::alwaysPrintZero;
    using File::realFormat;
    using File::realPrecision;
    
    /////////////////////////////////////////////////////////////////
    
    /// Increase indentation
    void indentMore()
    {
      indentLev++;
    }
    
    /// Decrease indentation
    void indentLess()
    {
      indentLev--;
    }
    
    /// Create a new line
    LoggerLine getNewLine()
    {
      return
	*this;
    }
    
    /// Create a new line
    LoggerLine operator()()
    {
      return
	*this;
    }
    // /// Create a new line, and print on it
    // template <typename T,
    // 	      typename=EnableIf<canPrint<LoggerLine,T>>,            // SFINAE needed to avoid ambiguous overload
    // 	      typename=EnableIf<not canPrint<Logger,RemRef<T>>>>    // SFINAE to avoid ambiguous reimplementation
    // LoggerLine operator<<(T&& t)
    // {
    //   return
    // 	std::move(getNewLine()<<forw<T>(t));
    // }
    
    /// Print a C-style variadic message
    LoggerLine printVariadicMessage(const char* format, ///< Format to print
				    va_list ap)         ///< Variadic part
    {
      return
	std::move(getNewLine().printVariadicMessage(format,ap));
    }
    
    /// Create with a path
    Logger(const char* path,              ///< Path to open
	   const bool& prependTime=true)  ///< Prepend or not with time
      :
      prependTime(prependTime)
    {
      file().open(path,"w");
      
      // Cannot print, otherwise all rank would!
      //*this<<"Logger initialized";
    }
  };
  
  /// Increment the logger indentation level for the object scope
  class ScopeIndenter
  {
    /// Pointed logger
    Logger& logger;
    
  public:
    
    /// Create and increase indent level
    ScopeIndenter(Logger& logger) :
      logger(logger)
    {
      logger.indentMore();
    }
    
    /// Delete and decrease indent level
    ~ScopeIndenter()
    {
      logger.indentLess();
    }
  };
  
  namespace resources
  {
    extern Logger fakeLogger;
    
    extern Logger logger;
  }
  
  INLINE_FUNCTION auto logger(const int verbosityLv=0)
  {
    if((mpi.isMasterRank() or not Logger::onlyMasterRankPrint) and
       (1 and /* threads.nActiveThreads()!=1 */ not Logger::onlyMasterThreadPrint) and
       (verbosityLv<=Logger::verbosityLv))
      return resources::logger();
    else
      return resources::fakeLogger();
  }
}

/// Shortcut to avoid having to put ()
#define LOGGER \
  logger()
  
  /// Verbose logger or not, capital worded for homogeneity
#define VERB_LOGGER(LV) \
  verbLogger(LV)

#endif
