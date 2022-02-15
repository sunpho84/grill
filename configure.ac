AC_INIT([e-snort],[1.0],[fr.sanfilippo@gmail.com])

#include all config flags
config_flags="$*"
AC_DEFINE_UNQUOTED([CONFIG_FLAGS],["$config_flags"],[Flags passed to configure])

#silent automake
AM_SILENT_RULES([yes])

#get configuration time
AC_MSG_CHECKING([configuration date and time])
CONFIG_TIME=`date`
AC_DEFINE_UNQUOTED([CONFIG_TIME], "$CONFIG_TIME", [time when configured])
AC_MSG_RESULT([$CONFIG_TIME])

#parameters definition
AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.14 no-define foreign subdir-objects])
AM_CONFIG_HEADER(src/config.hpp)
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST

#checks for programs
AC_PROG_CC
AC_PROG_FC
AC_PROG_CXX
AC_PROG_RANLIB
AC_LANG([C++])

AX_CXX_COMPILE_STDCXX(17,noext,mandatory)

LIBRARY_RESULT="---"
SUMMARY_RESULT="---"

AX_ASSEMBLY_REPORT


#check if we enable splitting MPI autosearch
AC_ARG_ENABLE(MPI-autosearch,
	AS_HELP_STRING([--enable-MPI-autosearch],[Enable MPI autosearch]),
	enable_MPI_autosearch="${enableval}",
	enable_MPI_autosearch="yes")
AC_MSG_RESULT([enabling MPI autosearch... $enable_MPI_autosearch])

#set MPI for c++
if test "$enable_MPI_autosearch" == "yes"
then
	AC_LANG_PUSH([C++])
	LX_FIND_MPI
	AC_LANG_POP([C++])
	AC_MSG_RESULT([with C++ MPI link options... $MPI_CXXLDFLAGS])
fi

#MPI
AC_ARG_ENABLE(MPI,
	AS_HELP_STRING([--disable-MPI],[Enable MPI]),
	enable_MPI="${enableval}",
	enable_MPI="yes")
AM_CONDITIONAL([USE_MPI],[test "$enable_MPI" == "yes" ],[true],[false])
AC_MSG_RESULT([enabling MPI... $enable_MPI])
SUMMARY_RESULT="$SUMMARY_RESULT
MPI enabled         : $enable_MPI"

if test "$enable_MPI" == "yes"
then
	# Check if we have MPI
	if test "$have_CXX_mpi" == "yes" -o "$enable_MPI_autosearch" != "yes"
	then
		CPPFLAGS="$MPI_CXXFLAGS $CPPFLAGS"
		LIBS="$MPI_CXXLIBS $LIBS"
		LDFLAGS="$MPI_CXXLDFLAGS $LDFLAGS"
		AC_DEFINE([USE_MPI],1,[Enable MPI])
	else
		AC_MSG_ERROR(["Unable to find MPI"])
	fi
fi

#check openmp
AX_OPENMP

#cuda
AX_CHECK_CUDA

AH_VERBATIM([COMPILING_FOR_DEVICE],
#ifdef __CUDA_ARCH__
 
 /// A convenient macro to detect that we are compiling on device
# define COMPILING_FOR_DEVICE
 
#endif
)

#cuda code
AC_ARG_ENABLE(cuda-code,
	AS_HELP_STRING([--enable-cuda-code],[Enable cuda code]),
	enable_cuda_code="${enableval}",
	enable_cuda_code="no")
AM_CONDITIONAL([CUDA_CODE],[test "$enable_cuda_code" == "yes" ],[true],[false])
AC_MSG_RESULT([enabling cuda code... $enable_cuda_code])
SUMMARY_RESULT="$SUMMARY_RESULT
Cuda code      : $enable_cuda_code"

if test "$enable_cuda_code" = "yes"
then
     case "$CXX" in
     	  nvcc) CPPFLAGS="-dc --expt-relaxed-constexpr --expt-extended-lambda -x cu $CPPFLAGS";;
     	  clang++) CPPFLAGS="-xcuda $CPPFLAGS";;
     	  *) AC_MSG_ERROR(["Unkwnown how to compile cuda code with $CXX"])
     esac
fi

#AX_CXXFLAGS_WARN_ALL
AC_MSG_CHECKING(whether compiler understands -Wall)
OLD_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS -Wall"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],
  AC_MSG_RESULT(yes),
  AC_MSG_RESULT(no)
  CXXFLAGS="$OLD_CXXFLAGS")

#macros to concatenate
AC_DEFINE([_CONCAT(X,Y)],[X##Y],[Internally concatenation])
AC_DEFINE([CONCAT(X,Y)],[_CONCAT(X,Y)],[Concatenate two pieces to produce a new token])
AC_DEFINE([CONCAT2(s1,s2)],[CONCAT(s1,s2)],[Wrapper to beat CPP])
AC_DEFINE([CONCAT3(s1,s2,s3)],[CONCAT(CONCAT2(s1,s2),s3)],[Concatenate three])
AC_DEFINE([NAME2(s1,s2)],[CONCAT3(s1,_,s2)],[Link with a _])
AC_DEFINE([NAME3(s1,s2,s3)],[NAME2(CONCAT3(s1,_,s2),s3)],[Name with two _])
AC_DEFINE([NAME4(s1,s2,s3,s4)],[NAME3(CONCAT3(s1,_,s2),s3,s4)],[Name with four _])
AC_DEFINE([REORDER_BARRIER()],[__asm volatile ("")],[Avoid spilling])
AC_DEFINE([MACRO_GUARD(...)],[do{__VA_ARGS__}while(0)],[Guard semicolon])
AC_DEFINE([TO_STRING_INTERNAL(x)],[#x],[Internally stringify])
AC_DEFINE([TO_STRING(x)],[TO_STRING_INTERNAL(x)],[Stringify])
AC_DEFINE([LINE_AS_STRING],[TO_STRING(__LINE__)],[Instantiate the line number in a string])
AC_DEFINE([ASM_BOOKMARK(COMMENT)],
	[asm(_ASM_BOOKMARK_SYMBOL "Bookmark file: \"" __FILE__ "\", line: " LINE_AS_STRING  ", " COMMENT)],
	[Include a comment in the assembler, recognizable in the compilation])
AC_DEFINE([ASM_BOOKMARK_BEGIN(COMMENT)],[ASM_BOOKMARK("BEGIN " COMMENT)],[Put a BEGIN for asm bookmark section])
AC_DEFINE([ASM_BOOKMARK_END(COMMENT)],[ASM_BOOKMARK("END " COMMENT)],[Put a END for asm bookmark section])

#result of conf to screen
AC_MSG_RESULT([
--------------------------------------------------
Configuration summary for $PACKAGE_NAME ($VERSION)
--------------------------------------------------
CC                  : $CC
CPPFLAGS            : $CPPFLAGS
CXXFLAGS            : $CXXFLAGS
CXX                 : $CXX
---
LDFLAGS             : $LDFLAGS
LIBS                : $LIBS
---
Installation prefix : $prefix
$LIBRARY_RESULT
$SUMMARY_RESULT
--------------------------------------------------
Configuration OK
--------------------------------------------------
])

#create config.hpp
AC_CONFIG_FILES(
	Makefile
	src/Makefile)

AC_OUTPUT