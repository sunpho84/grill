#ifndef _TIMER_HPP
#define _TIMER_HPP

/// \file timer.hpp
///
/// \brief Provides a timer which can be split
///

#ifdef HAVE_CONFIG_H
 #include "config.hpp"
#endif

#include <chrono>
#include <map>

#include <debug/minimalCrash.hpp>
#include <metaprogramming/nonConstMethod.hpp>

namespace grill
{
  /// Machine clock type
  using Clock=
    std::chrono::steady_clock;
  
  /// Instant type, defining a moment in time
  using Instant=
    std::chrono::time_point<Clock>;
  
  /// Difference of time between two moments
  using Duration=
    decltype(Instant{}-Instant{});
  
  /// Get current time
  inline Instant takeTime()
  {
    return
      Clock::now();
  }
  
  /// Splittable stopwatch
  ///
  /// If the stopwatch is created with \c isStoppable set to true, it
  /// cannot be stopped and is automatically started
  class Timer
  {
    /// Object incapsulating the comparison operator between two c strings
    struct CStringComparer
    {
      /// Comparison operator
      bool operator()(const char* first,  ///< First element to compare
		      const char* second) ///< Second element to compare
	const
      {
	return
	  strcmp(first,second)<0;
      }
    };
    
    /// A map which can be safely used with a c-string
    template <typename T> // Mapped type
    using MapWithCStrKey=
      std::map<const char*,T,CStringComparer>;
    
    /////////////////////////////////////////////////////////////////
    
    /// Number of intervals measured
    int nMeasures;
    
    /// Mark if the stopwatch is started
    bool isStartedFlag;
    
    /// Last moment the stopwatch was started
    Instant lastMomentStarted;
    
    /// Last moment the stopwatch was stopped
    Instant lastMomentStopped{lastMomentStarted};
    
    /// Cumulative time for which the timer has been let running
    Duration _cumulativeMeasure;
    
    /// Father of current timer
    Timer* _father;
    
    /// Number of children measuring
    int nStartedChildren{0};
    
    /// Children
    MapWithCStrKey<Timer> children;
    
    /// Determine whether it was explicitly started (or from a children)
    bool isExplicitlyStarted;
    
    /// Started mode: was it started by a children or explicityly?
    enum Started{IMPLICITLY,EXPLICITLY};
    
    /// Common start
    void innerStart(const bool implExpl)
    {
      // Check that the timer was not started already
      if(isStarted())
	MINIMAL_CRASH("Trying to start the already started stopwatch: %s",name);
      
      // Mark the timer as started
      isStartedFlag=
	true;
      
      /// Store whether the stopwatch was explitly or implicitly started
      isExplicitlyStarted=
	(implExpl==EXPLICITLY);
      
      // Increments the number of intervals measured
      nMeasures++;
      
      // Take current time
      lastMomentStarted=
	takeTime();
    }
    
    /// Starts through a children
    void childrenStarted()
    {
      if(not isStarted())
	innerStart(IMPLICITLY);
      
      nStartedChildren++;
    }
    
  public:
    
    /// Gets reference to the mapped sub timer
    Timer& operator[](const char* subName)
    {
      /// Finds the sub timer
      auto ref=
	children.find(subName);
      
      // Insert if not found
      if(ref==children.end())
	  ref=
	    children.try_emplace(subName,subName,this).first;
      
      return
	ref->second;
    }
    
    /// Checks if has children
    bool hasChildren()
      const
    {
      return
	children.size();
    }
    
    /// Checks if has running children
    bool hasRunningChildren()
      const
    {
      return
	nStartedChildren!=0;
    }
    
    /// Gets a reference to the father
    const Timer& father()
      const
    {
      return
	*_father;
    }
    
    PROVIDE_ALSO_NON_CONST_METHOD(father);
    
    /// Used to mark that there is no father
    static constexpr Timer* NO_FATHER=
      nullptr;
    
    /// Defines whether the timer can be stopped or not
    enum{UNSTOPPABLE,STOPPABLE};
    
    /// Name of the timer
    const char* name;
    
    /// Mark whether the stopwatch can be stopped
    const bool isStoppable;
    
    /// Returns whether the timer is started
    const bool& isStarted()
      const
    {
      return
	isStartedFlag;
    }
    
    /// Explicitly starts the stopwatch
    void start()
    {
      innerStart(EXPLICITLY);
    }
    
    /// Returns the last measured or the current one
    Duration currentMeasure()
      const
    {
      if(isStarted())
	return
	  takeTime()-lastMomentStarted;
      else
	return
	  lastMomentStopped-lastMomentStarted;
    }
    
    /// Returns the total measured time, including current one if is running
    Duration cumulativeMeasure()
      const
    {
      if(isStarted())
	return
	  _cumulativeMeasure+currentMeasure();
      else
	return
	  _cumulativeMeasure;
    }
    
    /// Average measure
    Duration averageMeasure()
      const
    {
      return
	cumulativeMeasure()/nMeasures;
    }
    
    /// Stop the stopwatch
    void stop()
    {
      if(not isStarted())
	MINIMAL_CRASH("Trying to stop the stopped stopwatch %s",name);
      
      if(not isStoppable)
	MINIMAL_CRASH("Trying to stop the unstoppable stopwatch %s",name);
      
      if(hasRunningChildren())
	MINIMAL_CRASH("Trying to stop stopwatch %s with %d running children",name,nStartedChildren);
      
      // Store the stopping time
      lastMomentStopped=
	takeTime();
      
      // Mark as stopped
      isStartedFlag=
	false;
      
      // Increment the cumulative measure
      _cumulativeMeasure+=
	currentMeasure();
    }
    
    /// Builds the stopwatch
    Timer(const char* name,                       ///< Name of the timer
	  Timer* father=nullptr,                  ///< Father of current timer
	  const bool isStoppableFlag=STOPPABLE)   ///< Flag determining if the stopwatch can be stopped
      :
      nMeasures(0),
      isStartedFlag(false),
      _cumulativeMeasure(0),
      _father(NO_FATHER),
      name(name),
      isStoppable(isStoppableFlag)
      
    {
      // Auto starts the stopwatch if it cannot be stopped
      if(not isStoppable)
	start();
    }
  };
  
  /// Convert duration into a units of measure
  template <typename U,                        // Units
	    typename O=double>                 // Output type
  double durationIn(const Duration& duration) ///< Input duration
  {
    return
      std::chrono::duration<O,U>(duration).count();
  }
  
  /// Convert duration into seconds
  template <typename O=double>                    // Output type
  double durationInSec(const Duration& duration) ///< Input duration
  {
    return
      std::chrono::duration<O>(duration).count();
  }
  
  /// Returns the duration of executing a function
  template <typename F,                  // Function type
	    typename...Args>             // Arguments type
  auto durationOf(Duration& duration,  ///< Variable where to store the duration
		  F&& f,               ///< Function to execute
		  Args&&...args)       ///< Arguments to call
  {
    /// Beginning instant
    Instant start=
      takeTime();
    
    /// Store the result
    auto res=
      f(std::forward<Args>(args)...);
    
    duration=
      takeTime()-start;
    
    return
      res;
  }
  
  /// Returns the duration of executing a function
  ///
  /// Result of f() is discarded
  template <typename F,                  // Function type
	    typename...Args>             // Arguments type
  Duration durationOf(F&& f,            ///< Function to execute
		      Args&&...args)    ///< Arguments to call
  {
    /// Result to be returned
    Duration duration;
    
    duationOf(duration,std::forward<F>(f),std::forward<Args>(args)...);
    
    return
      duration;
  }
  
}

#endif
