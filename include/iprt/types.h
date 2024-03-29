/** @file
 * IPRT - Types.
 */

/*
 * Copyright (C) 2006-2012 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 *
 * The contents of this file may alternatively be used under the terms
 * of the Common Development and Distribution License Version 1.0
 * (CDDL) only, as it comes in the "COPYING.CDDL" file of the
 * VirtualBox OSE distribution, in which case the provisions of the
 * CDDL are applicable instead of those of the GPL.
 *
 * You may elect to license modified versions of this file under the
 * terms and conditions of either the GPL or the CDDL or both.
 */

#ifndef ___iprt_types_h
#define ___iprt_types_h

#include <iprt/cdefs.h>
#include <iprt/stdint.h>
#include <iprt/stdarg.h>

/*
 * Include standard C types.
 */
#ifndef IPRT_NO_CRT

# if defined(IN_XF86_MODULE) && !defined(NO_ANSIC)
    /*
     * Kludge for xfree86 modules: size_t and other types are redefined.
     */
RT_C_DECLS_BEGIN
#  include "xf86_ansic.h"
#  undef NULL
RT_C_DECLS_END

# elif defined(RT_OS_DARWIN) && defined(KERNEL)
    /*
     * Kludge for the darwin kernel:
     *  stddef.h is missing IIRC.
     */
#  ifndef _PTRDIFF_T
#  define _PTRDIFF_T
    typedef __darwin_ptrdiff_t ptrdiff_t;
#  endif
#  include <sys/types.h>

# elif defined(RT_OS_FREEBSD) && defined(_KERNEL)
    /*
     * Kludge for the FreeBSD kernel:
     *  stddef.h and sys/types.h have slightly different offsetof definitions
     *  when compiling in kernel mode. This is just to make GCC shut up.
     */
#  ifndef _STDDEF_H_
#   undef offsetof
#  endif
#  include <sys/stddef.h>
#  ifndef _SYS_TYPES_H_
#   undef offsetof
#  endif
#  include <sys/types.h>
#  ifndef offsetof
#   error "offsetof is not defined..."
#  endif

# elif defined(RT_OS_FREEBSD) && HC_ARCH_BITS == 64 && defined(RT_ARCH_X86)
    /*
     * Kludge for compiling 32-bit code on a 64-bit FreeBSD:
     *  FreeBSD declares uint64_t and int64_t wrong (long unsigned and long int
     *  though they need to be long long unsigned and long long int). These
     *  defines conflict with our decleration in stdint.h. Adding the defines
     *  below omits the definitions in the system header.
     */
#  include <stddef.h>
#  define _UINT64_T_DECLARED
#  define _INT64_T_DECLARED
#  define _UINTPTR_T_DECLARED
#  define _INTPTR_T_DECLARED
#  include <sys/types.h>

# elif defined(RT_OS_LINUX) && defined(__KERNEL__)
    /*
     * Kludge for the linux kernel:
     *   1. sys/types.h doesn't mix with the kernel.
     *   2. Starting with 2.6.19, linux/types.h typedefs bool and linux/stddef.h
     *      declares false and true as enum values.
     *   3. Starting with 2.6.24, linux/types.h typedefs uintptr_t.
     * We work around these issues here and nowhere else.
     */
#  include <stddef.h>
#  if defined(__cplusplus)
    typedef bool _Bool;
#  endif
#  define bool linux_bool
#  define true linux_true
#  define false linux_false
#  define uintptr_t linux_uintptr_t
#  include <linux/version.h>
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
#   include <generated/autoconf.h>
#  else
#   ifndef AUTOCONF_INCLUDED
#    include <linux/autoconf.h>
#   endif
#  endif
#  include <linux/compiler.h>
#  if defined(__cplusplus)
    /*
     * Starting with 3.3, <linux/compiler-gcc.h> appends 'notrace' (which
     * expands to __attribute__((no_instrument_function))) to inline,
     * __inline and __inline__. Revert that.
     */
#   undef inline
#   define inline inline
#   undef __inline__
#   define __inline__ __inline__
#   undef __inline
#   define __inline __inline
#  endif
#  include <linux/types.h>
#  include <linux/stddef.h>
    /*
     * Starting with 3.4, <linux/stddef.h> defines NULL as '((void*)0)' which
     * does not work for C++ code.
     */
#  undef NULL
#  undef uintptr_t
#  ifdef __GNUC__
#   if (__GNUC__ * 100 + __GNUC_MINOR__) <= 400
     /*
      * <linux/compiler-gcc{3,4}.h> does
      *   #define __inline__  __inline__ __attribute__((always_inline))
      * in some older Linux kernels. Forcing inlining will fail for some RTStrA*
      * functions with gcc <= 4.0 due to passing variable argument lists.
      */
#    undef __inline__
#    define __inline__ __inline__
#   endif
#  endif
#  undef false
#  undef true
#  undef bool
# else
#  include <stddef.h>
#  include <sys/types.h>
# endif


/* Define any types missing from sys/types.h on windows. */
# ifdef _MSC_VER
#  undef ssize_t
   typedef intptr_t ssize_t;
# endif

#else  /* no crt */
# include <iprt/nocrt/compiler/compiler.h>
#endif /* no crt */



/** @def NULL
 * NULL pointer.
 */
#ifndef NULL
# ifdef __cplusplus
#  define NULL 0
# else
#  define NULL ((void*)0)
# endif
#endif



/** @defgroup grp_rt_types  IPRT Base Types
 * @{
 */

/* define wchar_t, we don't wanna include all the wcsstuff to get this. */
#ifdef _MSC_VER
# ifndef _WCHAR_T_DEFINED
  typedef unsigned short wchar_t;
# define _WCHAR_T_DEFINED
# endif
#endif
#ifdef __GNUC__
/** @todo wchar_t on GNUC */
#endif

/*
 * C doesn't have bool, nor does VisualAge for C++ v3.08.
 */
#if !defined(__cplusplus) || (defined(__IBMCPP__) && defined(RT_OS_OS2))
# if defined(__GNUC__)
#  if defined(RT_OS_LINUX) && __GNUC__ < 3
typedef uint8_t bool;
#  elif defined(RT_OS_FREEBSD)
#   ifndef __bool_true_false_are_defined
typedef _Bool bool;
#   endif
#  else
#   if (defined(RT_OS_DARWIN) || defined(RT_OS_HAIKU)) && (defined(_STDBOOL_H) || defined(__STDBOOL_H))
#    undef bool
#   endif
typedef _Bool bool;
#  endif
# else
typedef unsigned char bool;
# endif
# ifndef true
#  define true  (1)
# endif
# ifndef false
#  define false (0)
# endif
#endif

/**
 * 128-bit unsigned integer.
 */
#if defined(__GNUC__) && defined(RT_ARCH_AMD64)
typedef __uint128_t uint128_t;
#else
typedef struct uint128_s
{
# ifdef RT_BIG_ENDIAN
    uint64_t    Hi;
    uint64_t    Lo;
# else
    uint64_t    Lo;
    uint64_t    Hi;
# endif
} uint128_t;
#endif


/**
 * 128-bit signed integer.
 */
#if defined(__GNUC__) && defined(RT_ARCH_AMD64)
typedef __int128_t int128_t;
#else
typedef struct int128_s
{
# ifdef RT_BIG_ENDIAN
    int64_t     Hi;
    uint64_t    Lo;
# else
    uint64_t    Lo;
    int64_t     Hi;
# endif
} int128_t;
#endif


/**
 * 16-bit unsigned integer union.
 */
typedef union RTUINT16U
{
    /** natural view. */
    uint16_t    u;

    /** 16-bit view. */
    uint16_t    au16[1];
    /** 8-bit view. */
    uint8_t     au8[2];
    /** 16-bit hi/lo view. */
    struct
    {
#ifdef RT_BIG_ENDIAN
        uint8_t    Hi;
        uint8_t    Lo;
#else
        uint8_t    Lo;
        uint8_t    Hi;
#endif
    } s;
} RTUINT16U;
/** Pointer to a 16-bit unsigned integer union. */
typedef RTUINT16U *PRTUINT16U;
/** Pointer to a const 32-bit unsigned integer union. */
typedef const RTUINT16U *PCRTUINT16U;


/**
 * 32-bit unsigned integer union.
 */
typedef union RTUINT32U
{
    /** natural view. */
    uint32_t    u;
    /** Hi/Low view. */
    struct
    {
#ifdef RT_BIG_ENDIAN
        uint16_t    Hi;
        uint16_t    Lo;
#else
        uint16_t    Lo;
        uint16_t    Hi;
#endif
    } s;
    /** Word view. */
    struct
    {
#ifdef RT_BIG_ENDIAN
        uint16_t    w1;
        uint16_t    w0;
#else
        uint16_t    w0;
        uint16_t    w1;
#endif
    } Words;

    /** 32-bit view. */
    uint32_t    au32[1];
    /** 16-bit view. */
    uint16_t    au16[2];
    /** 8-bit view. */
    uint8_t     au8[4];
} RTUINT32U;
/** Pointer to a 32-bit unsigned integer union. */
typedef RTUINT32U *PRTUINT32U;
/** Pointer to a const 32-bit unsigned integer union. */
typedef const RTUINT32U *PCRTUINT32U;


/**
 * 64-bit unsigned integer union.
 */
typedef union RTUINT64U
{
    /** Natural view. */
    uint64_t    u;
    /** Hi/Low view. */
    struct
    {
#ifdef RT_BIG_ENDIAN
        uint32_t    Hi;
        uint32_t    Lo;
#else
        uint32_t    Lo;
        uint32_t    Hi;
#endif
    } s;
    /** Double-Word view. */
    struct
    {
#ifdef RT_BIG_ENDIAN
        uint32_t    dw1;
        uint32_t    dw0;
#else
        uint32_t    dw0;
        uint32_t    dw1;
#endif
    } DWords;
    /** Word view. */
    struct
    {
#ifdef RT_BIG_ENDIAN
        uint16_t    w3;
        uint16_t    w2;
        uint16_t    w1;
        uint16_t    w0;
#else
        uint16_t    w0;
        uint16_t    w1;
        uint16_t    w2;
        uint16_t    w3;
#endif
    } Words;

    /** 64-bit view. */
    uint64_t    au64[1];
    /** 32-bit view. */
    uint32_t    au32[2];
    /** 16-bit view. */
    uint16_t    au16[4];
    /** 8-bit view. */
    uint8_t     au8[8];
} RTUINT64U;
/** Pointer to a 64-bit unsigned integer union. */
typedef RTUINT64U *PRTUINT64U;
/** Pointer to a const 64-bit unsigned integer union. */
typedef const RTUINT64U *PCRTUINT64U;


/**
 * 128-bit unsigned integer union.
 */
#pragma pack(1)
typedef union RTUINT128U
{
    /** Hi/Low view.
     * @remarks We put this first so we can have portable initializers
     *          (RTUINT128_INIT) */
    struct
    {
#ifdef RT_BIG_ENDIAN
        uint64_t    Hi;
        uint64_t    Lo;
#else
        uint64_t    Lo;
        uint64_t    Hi;
#endif
    } s;

    /** Natural view.
     * WARNING! This member depends on the compiler supporting 128-bit stuff. */
    uint128_t   u;

    /** Quad-Word view. */
    struct
    {
#ifdef RT_BIG_ENDIAN
        uint64_t    qw1;
        uint64_t    qw0;
#else
        uint64_t    qw0;
        uint64_t    qw1;
#endif
    } QWords;
    /** Double-Word view. */
    struct
    {
#ifdef RT_BIG_ENDIAN
        uint32_t    dw3;
        uint32_t    dw2;
        uint32_t    dw1;
        uint32_t    dw0;
#else
        uint32_t    dw0;
        uint32_t    dw1;
        uint32_t    dw2;
        uint32_t    dw3;
#endif
    } DWords;
    /** Word view. */
    struct
    {
#ifdef RT_BIG_ENDIAN
        uint16_t    w7;
        uint16_t    w6;
        uint16_t    w5;
        uint16_t    w4;
        uint16_t    w3;
        uint16_t    w2;
        uint16_t    w1;
        uint16_t    w0;
#else
        uint16_t    w0;
        uint16_t    w1;
        uint16_t    w2;
        uint16_t    w3;
        uint16_t    w4;
        uint16_t    w5;
        uint16_t    w6;
        uint16_t    w7;
#endif
    } Words;

    /** 64-bit view. */
    uint64_t    au64[2];
    /** 32-bit view. */
    uint32_t    au32[4];
    /** 16-bit view. */
    uint16_t    au16[8];
    /** 8-bit view. */
    uint8_t     au8[16];
} RTUINT128U;
#pragma pack()
/** Pointer to a 64-bit unsigned integer union. */
typedef RTUINT128U *PRTUINT128U;
/** Pointer to a const 64-bit unsigned integer union. */
typedef const RTUINT128U *PCRTUINT128U;

/** @def RTUINT128_INIT
 * Portable RTUINT128U initializer. */
#ifdef RT_BIG_ENDIAN
# define RTUINT128_INIT(a_Hi, a_Lo) { a_Hi, a_Lo }
#else
# define RTUINT128_INIT(a_Hi, a_Lo) { a_Lo, a_Hi }
#endif

/** @def RTUINT128_INIT_C
 * Portable RTUINT128U initializer for 64-bit constants. */
#ifdef RT_BIG_ENDIAN
# define RTUINT128_INIT_C(a_Hi, a_Lo) { UINT64_C(a_Hi), UINT64_C(a_Lo) }
#else
# define RTUINT128_INIT_C(a_Hi, a_Lo) { UINT64_C(a_Lo), UINT64_C(a_Hi) }
#endif


/**
 * Double precision floating point format (64-bit).
 */
typedef union RTFLOAT64U
{
#if defined(RT_ARCH_AMD64) || defined(RT_ARCH_X86)
    /** Double view. */
    double      rd;
#endif
    /** Format using regular bitfields.  */
    struct
    {
# ifdef RT_BIG_ENDIAN
        /** The sign indicator. */
        uint32_t    fSign : 1;
        /** The exponent (offseted by 1023). */
        uint32_t    uExponent : 11;
        /** The fraction, bits 32 thru 51. */
        uint32_t    u20FractionHigh : 20;
        /** The fraction, bits 0 thru 31. */
        uint32_t    u32FractionLow;
# else
        /** The fraction, bits 0 thru 31. */
        uint32_t    u32FractionLow;
        /** The fraction, bits 32 thru 51. */
        uint32_t    u20FractionHigh : 20;
        /** The exponent (offseted by 1023). */
        uint32_t    uExponent : 11;
        /** The sign indicator. */
        uint32_t    fSign : 1;
# endif
    } s;

#ifdef RT_COMPILER_GROKS_64BIT_BITFIELDS
    /** Format using 64-bit bitfields.  */
    RT_GCC_EXTENSION struct
    {
# ifdef RT_BIG_ENDIAN
        /** The sign indicator. */
        RT_GCC_EXTENSION uint64_t    fSign : 1;
        /** The exponent (offseted by 1023). */
        RT_GCC_EXTENSION uint64_t    uExponent : 11;
        /** The fraction. */
        RT_GCC_EXTENSION uint64_t    uFraction : 52;
# else
        /** The fraction. */
        RT_GCC_EXTENSION uint64_t    uFraction : 52;
        /** The exponent (offseted by 1023). */
        RT_GCC_EXTENSION uint64_t    uExponent : 11;
        /** The sign indicator. */
        RT_GCC_EXTENSION uint64_t    fSign : 1;
# endif
    } s64;
#endif

    /** 64-bit view. */
    uint64_t    au64[1];
    /** 32-bit view. */
    uint32_t    au32[2];
    /** 16-bit view. */
    uint16_t    au16[4];
    /** 8-bit view. */
    uint8_t     au8[8];
} RTFLOAT64U;
/** Pointer to a double precision floating point format union. */
typedef RTFLOAT64U *PRTFLOAT64U;
/** Pointer to a const double precision floating point format union. */
typedef const RTFLOAT64U *PCRTFLOAT64U;


#if !defined(__IBMCPP__) && !defined(__IBMC__)

/**
 * Extended Double precision floating point format (80-bit).
 */
#pragma pack(1)
typedef union RTFLOAT80U
{
    /** Format using bitfields.  */
    RT_GCC_EXTENSION struct
    {
# ifdef RT_BIG_ENDIAN
        /** The sign indicator. */
        RT_GCC_EXTENSION uint16_t   fSign : 1;
        /** The exponent (offseted by 16383). */
        RT_GCC_EXTENSION uint16_t   uExponent : 15;
        /** The mantissa. */
        uint64_t                    u64Mantissa;
# else
        /** The mantissa. */
        uint64_t                    u64Mantissa;
        /** The exponent (offseted by 16383). */
        RT_GCC_EXTENSION uint16_t   uExponent : 15;
        /** The sign indicator. */
        RT_GCC_EXTENSION uint16_t   fSign : 1;
# endif
    } s;

    /** 64-bit view. */
    uint64_t    au64[1];
    /** 32-bit view. */
    uint32_t    au32[2];
    /** 16-bit view. */
    uint16_t    au16[5];
    /** 8-bit view. */
    uint8_t     au8[10];
} RTFLOAT80U;
#pragma pack()
/** Pointer to a extended precision floating point format union. */
typedef RTFLOAT80U *PRTFLOAT80U;
/** Pointer to a const extended precision floating point format union. */
typedef const RTFLOAT80U *PCRTFLOAT80U;


/**
 * A variant of RTFLOAT80U that may be larger than 80-bits depending on how the
 * compiler implements long double.
 */
#pragma pack(1)
typedef union RTFLOAT80U2
{
#ifdef RT_COMPILER_WITH_80BIT_LONG_DOUBLE
    /** Long double view. */
    long double     lrd;
#endif
    /** Format using bitfields.  */
    RT_GCC_EXTENSION struct
    {
#ifdef RT_BIG_ENDIAN
        /** The sign indicator. */
        RT_GCC_EXTENSION uint16_t   fSign : 1;
        /** The exponent (offseted by 16383). */
        RT_GCC_EXTENSION uint16_t   uExponent : 15;
        /** The mantissa. */
        uint64_t                    u64Mantissa;
#else
        /** The mantissa. */
        uint64_t                    u64Mantissa;
        /** The exponent (offseted by 16383). */
        RT_GCC_EXTENSION uint16_t   uExponent : 15;
        /** The sign indicator. */
        RT_GCC_EXTENSION uint16_t   fSign : 1;
#endif
    } s;

    /** Bitfield exposing the J bit and the fraction.  */
    RT_GCC_EXTENSION struct
    {
#ifdef RT_BIG_ENDIAN
        /** The sign indicator. */
        RT_GCC_EXTENSION uint16_t   fSign : 1;
        /** The exponent (offseted by 16383). */
        RT_GCC_EXTENSION uint16_t   uExponent : 15;
        /** The J bit, aka the integer bit. */
        uint32_t                    fInteger;
        /** The fraction, bits 32 thru 62. */
        uint32_t                    u31FractionHigh : 31;
        /** The fraction, bits 0 thru 31. */
        uint32_t                    u32FractionLow : 32;
#else
        /** The fraction, bits 0 thru 31. */
        uint32_t                    u32FractionLow : 32;
        /** The fraction, bits 32 thru 62. */
        uint32_t                    u31FractionHigh : 31;
        /** The J bit, aka the integer bit. */
        uint32_t                    fInteger;
        /** The exponent (offseted by 16383). */
        RT_GCC_EXTENSION uint16_t   uExponent : 15;
        /** The sign indicator. */
        RT_GCC_EXTENSION uint16_t   fSign : 1;
#endif
    } sj;

#ifdef RT_COMPILER_GROKS_64BIT_BITFIELDS
    /** 64-bit bitfields exposing the J bit and the fraction.  */
    RT_GCC_EXTENSION struct
    {
# ifdef RT_BIG_ENDIAN
        /** The sign indicator. */
        RT_GCC_EXTENSION uint16_t   fSign : 1;
        /** The exponent (offseted by 16383). */
        RT_GCC_EXTENSION uint16_t   uExponent : 15;
        /** The J bit, aka the integer bit. */
        RT_GCC_EXTENSION uint64_t   fInteger : 1;
        /** The fraction. */
        RT_GCC_EXTENSION uint64_t   u63Fraction : 63;
# else
        /** The fraction. */
        RT_GCC_EXTENSION uint64_t   u63Fraction : 63;
        /** The J bit, aka the integer bit. */
        RT_GCC_EXTENSION uint64_t   fInteger : 1;
        /** The exponent (offseted by 16383). */
        RT_GCC_EXTENSION uint16_t   uExponent : 15;
        /** The sign indicator. */
        RT_GCC_EXTENSION uint16_t   fSign : 1;
# endif
    } sj64;
#endif

    /** 64-bit view. */
    uint64_t    au64[1];
    /** 32-bit view. */
    uint32_t    au32[2];
    /** 16-bit view. */
    uint16_t    au16[5];
    /** 8-bit view. */
    uint8_t     au8[10];
} RTFLOAT80U2;
#pragma pack()
/** Pointer to a extended precision floating point format union, 2nd
 * variant. */
typedef RTFLOAT80U2 *PRTFLOAT80U2;
/** Pointer to a const extended precision floating point format union, 2nd
 * variant. */
typedef const RTFLOAT80U2 *PCRTFLOAT80U2;

#endif /* uint16_t bitfields doesn't work */


/** Generic function type.
 * @see PFNRT
 */
typedef DECLCALLBACK(void) FNRT(void);

/** Generic function pointer.
 * With -pedantic, gcc-4 complains when casting a function to a data object, for
 * example:
 *
 * @code
 *    void foo(void)
 *    {
 *    }
 *
 *    void *bar = (void *)foo;
 * @endcode
 *
 * The compiler would warn with "ISO C++ forbids casting between
 * pointer-to-function and pointer-to-object".  The purpose of this warning is
 * not to bother the programmer but to point out that he is probably doing
 * something dangerous, assigning a pointer to executable code to a data object.
 */
typedef FNRT *PFNRT;

/** Millisecond interval. */
typedef uint32_t            RTMSINTERVAL;
/** Pointer to a millisecond interval. */
typedef RTMSINTERVAL       *PRTMSINTERVAL;
/** Pointer to a const millisecond interval. */
typedef const RTMSINTERVAL *PCRTMSINTERVAL;

/** Pointer to a time spec structure. */
typedef struct RTTIMESPEC  *PRTTIMESPEC;
/** Pointer to a const time spec structure. */
typedef const struct RTTIMESPEC *PCRTTIMESPEC;



/** @defgroup grp_rt_types_both  Common Guest and Host Context Basic Types
 * @ingroup grp_rt_types
 * @{
 */

/** Signed integer which can contain both GC and HC pointers. */
#if (HC_ARCH_BITS == 32 && GC_ARCH_BITS == 32)
typedef int32_t         RTINTPTR;
#elif (HC_ARCH_BITS == 64 || GC_ARCH_BITS == 64)
typedef int64_t         RTINTPTR;
#else
#  error Unsupported HC_ARCH_BITS and/or GC_ARCH_BITS values.
#endif
/** Pointer to signed integer which can contain both GC and HC pointers. */
typedef RTINTPTR       *PRTINTPTR;
/** Pointer const to signed integer which can contain both GC and HC pointers. */
typedef const RTINTPTR *PCRTINTPTR;
/** The maximum value the RTINTPTR type can hold. */
#if (HC_ARCH_BITS == 32 && GC_ARCH_BITS == 32)
# define RTINTPTR_MAX   INT32_MAX
#elif (HC_ARCH_BITS == 64 || GC_ARCH_BITS == 64)
# define RTINTPTR_MAX   INT64_MAX
#else
#  error Unsupported HC_ARCH_BITS and/or GC_ARCH_BITS values.
#endif
/** The minimum value the RTINTPTR type can hold. */
#if (HC_ARCH_BITS == 32 && GC_ARCH_BITS == 32)
# define RTINTPTR_MIN   INT32_MIN
#elif (HC_ARCH_BITS == 64 || GC_ARCH_BITS == 64)
# define RTINTPTR_MIN   INT64_MIN
#else
#  error Unsupported HC_ARCH_BITS and/or GC_ARCH_BITS values.
#endif

/** Unsigned integer which can contain both GC and HC pointers. */
#if (HC_ARCH_BITS == 32 && GC_ARCH_BITS == 32)
typedef uint32_t        RTUINTPTR;
#elif (HC_ARCH_BITS == 64 || GC_ARCH_BITS == 64)
typedef uint64_t        RTUINTPTR;
#else
#  error Unsupported HC_ARCH_BITS and/or GC_ARCH_BITS values.
#endif
/** Pointer to unsigned integer which can contain both GC and HC pointers. */
typedef RTUINTPTR      *PRTUINTPTR;
/** Pointer const to unsigned integer which can contain both GC and HC pointers. */
typedef const RTUINTPTR *PCRTUINTPTR;
/** The maximum value the RTUINTPTR type can hold. */
#if (HC_ARCH_BITS == 32 && GC_ARCH_BITS == 32)
# define RTUINTPTR_MAX  UINT32_MAX
#elif (HC_ARCH_BITS == 64 || GC_ARCH_BITS == 64)
# define RTUINTPTR_MAX  UINT64_MAX
#else
#  error Unsupported HC_ARCH_BITS and/or GC_ARCH_BITS values.
#endif

/** Signed integer. */
typedef int32_t         RTINT;
/** Pointer to signed integer. */
typedef RTINT          *PRTINT;
/** Pointer to const signed integer. */
typedef const RTINT    *PCRTINT;

/** Unsigned integer. */
typedef uint32_t        RTUINT;
/** Pointer to unsigned integer. */
typedef RTUINT         *PRTUINT;
/** Pointer to const unsigned integer. */
typedef const RTUINT   *PCRTUINT;

/** A file offset / size (off_t). */
typedef int64_t         RTFOFF;
/** Pointer to a file offset / size. */
typedef RTFOFF         *PRTFOFF;
/** The max value for RTFOFF. */
#define RTFOFF_MAX      INT64_MAX
/** The min value for RTFOFF. */
#define RTFOFF_MIN      INT64_MIN

/** File mode (see iprt/fs.h). */
typedef uint32_t        RTFMODE;
/** Pointer to file mode. */
typedef RTFMODE        *PRTFMODE;

/** Device unix number. */
typedef uint32_t        RTDEV;
/** Pointer to a device unix number. */
typedef RTDEV          *PRTDEV;

/** @name RTDEV Macros
 * @{  */
/**
 * Our makedev macro.
 * @returns RTDEV
 * @param   uMajor          The major device number.
 * @param   uMinor          The minor device number.
 */
#define RTDEV_MAKE(uMajor, uMinor)      ((RTDEV)( ((RTDEV)(uMajor) << 24) | (uMinor & UINT32_C(0x00ffffff)) ))
/**
 * Get the major device node number from an RTDEV type.
 * @returns The major device number of @a uDev
 * @param   uDev            The device number.
 */
#define RTDEV_MAJOR(uDev)               ((uDev) >> 24)
/**
 * Get the minor device node number from an RTDEV type.
 * @returns The minor device number of @a uDev
 * @param   uDev            The device number.
 */
#define RTDEV_MINOR(uDev)               ((uDev) & UINT32_C(0x00ffffff))
/** @}  */

/** i-node number. */
typedef uint64_t        RTINODE;
/** Pointer to a i-node number. */
typedef RTINODE        *PRTINODE;

/** User id. */
typedef uint32_t        RTUID;
/** Pointer to a user id. */
typedef RTUID          *PRTUID;
/** NIL user id.
 * @todo check this for portability! */
#define NIL_RTUID       (~(RTUID)0)

/** Group id. */
typedef uint32_t        RTGID;
/** Pointer to a group id. */
typedef RTGID          *PRTGID;
/** NIL group id.
 * @todo check this for portability! */
#define NIL_RTGID       (~(RTGID)0)

/** I/O Port. */
typedef uint16_t        RTIOPORT;
/** Pointer to I/O Port. */
typedef RTIOPORT       *PRTIOPORT;
/** Pointer to const I/O Port. */
typedef const RTIOPORT *PCRTIOPORT;

/** Selector. */
typedef uint16_t        RTSEL;
/** Pointer to selector. */
typedef RTSEL          *PRTSEL;
/** Pointer to const selector. */
typedef const RTSEL    *PCRTSEL;
/** Max selector value. */
#define RTSEL_MAX       UINT16_MAX

/** Far 16-bit pointer. */
#pragma pack(1)
typedef struct RTFAR16
{
    uint16_t        off;
    RTSEL           sel;
} RTFAR16;
#pragma pack()
/** Pointer to Far 16-bit pointer. */
typedef RTFAR16 *PRTFAR16;
/** Pointer to const Far 16-bit pointer. */
typedef const RTFAR16 *PCRTFAR16;

/** Far 32-bit pointer. */
#pragma pack(1)
typedef struct RTFAR32
{
    uint32_t        off;
    RTSEL           sel;
} RTFAR32;
#pragma pack()
/** Pointer to Far 32-bit pointer. */
typedef RTFAR32 *PRTFAR32;
/** Pointer to const Far 32-bit pointer. */
typedef const RTFAR32 *PCRTFAR32;

/** Far 64-bit pointer. */
#pragma pack(1)
typedef struct RTFAR64
{
    uint64_t        off;
    RTSEL           sel;
} RTFAR64;
#pragma pack()
/** Pointer to Far 64-bit pointer. */
typedef RTFAR64 *PRTFAR64;
/** Pointer to const Far 64-bit pointer. */
typedef const RTFAR64 *PCRTFAR64;

/** @} */


/** @defgroup grp_rt_types_hc  Host Context Basic Types
 * @ingroup grp_rt_types
 * @{
 */

/** HC Natural signed integer.
 * @deprecated silly type. */
typedef int32_t             RTHCINT;
/** Pointer to HC Natural signed integer.
 * @deprecated silly type. */
typedef RTHCINT            *PRTHCINT;
/** Pointer to const HC Natural signed integer.
 * @deprecated silly type. */
typedef const RTHCINT      *PCRTHCINT;

/** HC Natural unsigned integer.
 * @deprecated silly type. */
typedef uint32_t            RTHCUINT;
/** Pointer to HC Natural unsigned integer.
 * @deprecated silly type. */
typedef RTHCUINT           *PRTHCUINT;
/** Pointer to const HC Natural unsigned integer.
 * @deprecated silly type. */
typedef const RTHCUINT     *PCRTHCUINT;


/** Signed integer which can contain a HC pointer. */
#if HC_ARCH_BITS == 32
typedef int32_t             RTHCINTPTR;
#elif HC_ARCH_BITS == 64
typedef int64_t             RTHCINTPTR;
#else
# error Unsupported HC_ARCH_BITS value.
#endif
/** Pointer to signed integer which can contain a HC pointer. */
typedef RTHCINTPTR         *PRTHCINTPTR;
/** Pointer to const signed integer which can contain a HC pointer. */
typedef const RTHCINTPTR   *PCRTHCINTPTR;
/** Max RTHCINTPTR value. */
#if HC_ARCH_BITS == 32
# define RTHCINTPTR_MAX     INT32_MAX
#else
# define RTHCINTPTR_MAX     INT64_MAX
#endif
/** Min RTHCINTPTR value. */
#if HC_ARCH_BITS == 32
# define RTHCINTPTR_MIN     INT32_MIN
#else
# define RTHCINTPTR_MIN     INT64_MIN
#endif

/** Signed integer which can contain a HC ring-3 pointer. */
#if R3_ARCH_BITS == 32
typedef int32_t             RTR3INTPTR;
#elif R3_ARCH_BITS == 64
typedef int64_t             RTR3INTPTR;
#else
#  error Unsupported R3_ARCH_BITS value.
#endif
/** Pointer to signed integer which can contain a HC ring-3 pointer. */
typedef RTR3INTPTR         *PRTR3INTPTR;
/** Pointer to const signed integer which can contain a HC ring-3 pointer. */
typedef const RTR3INTPTR   *PCRTR3INTPTR;
/** Max RTR3INTPTR value. */
#if R3_ARCH_BITS == 32
# define RTR3INTPTR_MAX     INT32_MAX
#else
# define RTR3INTPTR_MAX     INT64_MAX
#endif
/** Min RTR3INTPTR value. */
#if R3_ARCH_BITS == 32
# define RTR3INTPTR_MIN     INT32_MIN
#else
# define RTR3INTPTR_MIN     INT64_MIN
#endif

/** Signed integer which can contain a HC ring-0 pointer. */
#if R0_ARCH_BITS == 32
typedef int32_t             RTR0INTPTR;
#elif R0_ARCH_BITS == 64
typedef int64_t             RTR0INTPTR;
#else
# error Unsupported R0_ARCH_BITS value.
#endif
/** Pointer to signed integer which can contain a HC ring-0 pointer. */
typedef RTR0INTPTR         *PRTR0INTPTR;
/** Pointer to const signed integer which can contain a HC ring-0 pointer. */
typedef const RTR0INTPTR   *PCRTR0INTPTR;
/** Max RTR0INTPTR value. */
#if R0_ARCH_BITS == 32
# define RTR0INTPTR_MAX     INT32_MAX
#else
# define RTR0INTPTR_MAX     INT64_MAX
#endif
/** Min RTHCINTPTR value. */
#if R0_ARCH_BITS == 32
# define RTR0INTPTR_MIN     INT32_MIN
#else
# define RTR0INTPTR_MIN     INT64_MIN
#endif


/** Unsigned integer which can contain a HC pointer. */
#if HC_ARCH_BITS == 32
typedef uint32_t            RTHCUINTPTR;
#elif HC_ARCH_BITS == 64
typedef uint64_t            RTHCUINTPTR;
#else
# error Unsupported HC_ARCH_BITS value.
#endif
/** Pointer to unsigned integer which can contain a HC pointer. */
typedef RTHCUINTPTR        *PRTHCUINTPTR;
/** Pointer to unsigned integer which can contain a HC pointer. */
typedef const RTHCUINTPTR  *PCRTHCUINTPTR;
/** Max RTHCUINTTPR value. */
#if HC_ARCH_BITS == 32
# define RTHCUINTPTR_MAX    UINT32_MAX
#else
# define RTHCUINTPTR_MAX    UINT64_MAX
#endif

/** Unsigned integer which can contain a HC ring-3 pointer. */
#if R3_ARCH_BITS == 32
typedef uint32_t            RTR3UINTPTR;
#elif R3_ARCH_BITS == 64
typedef uint64_t            RTR3UINTPTR;
#else
# error Unsupported R3_ARCH_BITS value.
#endif
/** Pointer to unsigned integer which can contain a HC ring-3 pointer. */
typedef RTR3UINTPTR        *PRTR3UINTPTR;
/** Pointer to unsigned integer which can contain a HC ring-3 pointer. */
typedef const RTR3UINTPTR  *PCRTR3UINTPTR;
/** Max RTHCUINTTPR value. */
#if R3_ARCH_BITS == 32
# define RTR3UINTPTR_MAX    UINT32_MAX
#else
# define RTR3UINTPTR_MAX    UINT64_MAX
#endif

/** Unsigned integer which can contain a HC ring-0 pointer. */
#if R0_ARCH_BITS == 32
typedef uint32_t            RTR0UINTPTR;
#elif R0_ARCH_BITS == 64
typedef uint64_t            RTR0UINTPTR;
#else
# error Unsupported R0_ARCH_BITS value.
#endif
/** Pointer to unsigned integer which can contain a HC ring-0 pointer. */
typedef RTR0UINTPTR        *PRTR0UINTPTR;
/** Pointer to unsigned integer which can contain a HC ring-0 pointer. */
typedef const RTR0UINTPTR  *PCRTR0UINTPTR;
/** Max RTR0UINTTPR value. */
#if HC_ARCH_BITS == 32
# define RTR0UINTPTR_MAX    UINT32_MAX
#else
# define RTR0UINTPTR_MAX    UINT64_MAX
#endif


/** Host Physical Memory Address. */
typedef uint64_t            RTHCPHYS;
/** Pointer to Host Physical Memory Address. */
typedef RTHCPHYS           *PRTHCPHYS;
/** Pointer to const Host Physical Memory Address. */
typedef const RTHCPHYS     *PCRTHCPHYS;
/** @def NIL_RTHCPHYS
 * NIL HC Physical Address.
 * NIL_RTHCPHYS is used to signal an invalid physical address, similar
 * to the NULL pointer.
 */
#define NIL_RTHCPHYS        (~(RTHCPHYS)0)
/** Max RTHCPHYS value. */
#define RTHCPHYS_MAX        UINT64_MAX


/** HC pointer. */
#ifndef IN_RC
typedef void *              RTHCPTR;
#else
typedef RTHCUINTPTR         RTHCPTR;
#endif
/** Pointer to HC pointer. */
typedef RTHCPTR            *PRTHCPTR;
/** Pointer to const HC pointer. */
typedef const RTHCPTR      *PCRTHCPTR;
/** @def NIL_RTHCPTR
 * NIL HC pointer.
 */
#define NIL_RTHCPTR         ((RTHCPTR)0)
/** Max RTHCPTR value. */
#define RTHCPTR_MAX         ((RTHCPTR)RTHCUINTPTR_MAX)


/** HC ring-3 pointer. */
#ifdef IN_RING3
typedef void *              RTR3PTR;
#else
typedef RTR3UINTPTR         RTR3PTR;
#endif
/** Pointer to HC ring-3 pointer. */
typedef RTR3PTR            *PRTR3PTR;
/** Pointer to const HC ring-3 pointer. */
typedef const RTR3PTR      *PCRTR3PTR;
/** @def NIL_RTR3PTR
 * NIL HC ring-3 pointer.
 */
#ifndef IN_RING3
# define NIL_RTR3PTR        ((RTR3PTR)0)
#else
# define NIL_RTR3PTR        (NULL)
#endif
/** Max RTR3PTR value. */
#define RTR3PTR_MAX         ((RTR3PTR)RTR3UINTPTR_MAX)

/** HC ring-0 pointer. */
#ifdef  IN_RING0
typedef void *              RTR0PTR;
#else
typedef RTR0UINTPTR         RTR0PTR;
#endif
/** Pointer to HC ring-0 pointer. */
typedef RTR0PTR            *PRTR0PTR;
/** Pointer to const HC ring-0 pointer. */
typedef const RTR0PTR      *PCRTR0PTR;
/** @def NIL_RTR0PTR
 * NIL HC ring-0 pointer.
 */
#ifndef IN_RING0
# define NIL_RTR0PTR        ((RTR0PTR)0)
#else
# define NIL_RTR0PTR        (NULL)
#endif
/** Max RTR3PTR value. */
#define RTR0PTR_MAX         ((RTR0PTR)RTR0UINTPTR_MAX)


/** Unsigned integer register in the host context. */
#if HC_ARCH_BITS == 32
typedef uint32_t            RTHCUINTREG;
#elif HC_ARCH_BITS == 64
typedef uint64_t            RTHCUINTREG;
#else
# error "Unsupported HC_ARCH_BITS!"
#endif
/** Pointer to an unsigned integer register in the host context. */
typedef RTHCUINTREG        *PRTHCUINTREG;
/** Pointer to a const unsigned integer register in the host context. */
typedef const RTHCUINTREG  *PCRTHCUINTREG;

/** Unsigned integer register in the host ring-3 context. */
#if R3_ARCH_BITS == 32
typedef uint32_t            RTR3UINTREG;
#elif R3_ARCH_BITS == 64
typedef uint64_t            RTR3UINTREG;
#else
# error "Unsupported R3_ARCH_BITS!"
#endif
/** Pointer to an unsigned integer register in the host ring-3 context. */
typedef RTR3UINTREG        *PRTR3UINTREG;
/** Pointer to a const unsigned integer register in the host ring-3 context. */
typedef const RTR3UINTREG  *PCRTR3UINTREG;

/** Unsigned integer register in the host ring-3 context. */
#if R0_ARCH_BITS == 32
typedef uint32_t            RTR0UINTREG;
#elif R0_ARCH_BITS == 64
typedef uint64_t            RTR0UINTREG;
#else
# error "Unsupported R3_ARCH_BITS!"
#endif
/** Pointer to an unsigned integer register in the host ring-3 context. */
typedef RTR0UINTREG        *PRTR0UINTREG;
/** Pointer to a const unsigned integer register in the host ring-3 context. */
typedef const RTR0UINTREG  *PCRTR0UINTREG;

/** @} */


/** @defgroup grp_rt_types_gc  Guest Context Basic Types
 * @ingroup grp_rt_types
 * @{
 */

/** Natural signed integer in the GC.
 * @deprecated silly type. */
#if GC_ARCH_BITS == 32
typedef int32_t         RTGCINT;
#elif GC_ARCH_BITS == 64 /** @todo this isn't right, natural int is 32-bit, see RTHCINT. */
typedef int64_t         RTGCINT;
#endif
/** Pointer to natural signed integer in GC.
 * @deprecated silly type. */
typedef RTGCINT        *PRTGCINT;
/** Pointer to const natural signed integer in GC.
 * @deprecated silly type. */
typedef const RTGCINT  *PCRTGCINT;

/** Natural unsigned integer in the GC.
 * @deprecated silly type. */
#if GC_ARCH_BITS == 32
typedef uint32_t        RTGCUINT;
#elif GC_ARCH_BITS == 64 /** @todo this isn't right, natural int is 32-bit, see RTHCUINT. */
typedef uint64_t        RTGCUINT;
#endif
/** Pointer to natural unsigned integer in GC.
 * @deprecated silly type. */
typedef RTGCUINT       *PRTGCUINT;
/** Pointer to const natural unsigned integer in GC.
 * @deprecated silly type. */
typedef const RTGCUINT *PCRTGCUINT;

/** Signed integer which can contain a GC pointer. */
#if GC_ARCH_BITS == 32
typedef int32_t         RTGCINTPTR;
#elif GC_ARCH_BITS == 64
typedef int64_t         RTGCINTPTR;
#endif
/** Pointer to signed integer which can contain a GC pointer. */
typedef RTGCINTPTR     *PRTGCINTPTR;
/** Pointer to const signed integer which can contain a GC pointer. */
typedef const RTGCINTPTR *PCRTGCINTPTR;

/** Unsigned integer which can contain a GC pointer. */
#if GC_ARCH_BITS == 32
typedef uint32_t        RTGCUINTPTR;
#elif GC_ARCH_BITS == 64
typedef uint64_t        RTGCUINTPTR;
#else
#  error Unsupported GC_ARCH_BITS value.
#endif
/** Pointer to unsigned integer which can contain a GC pointer. */
typedef RTGCUINTPTR     *PRTGCUINTPTR;
/** Pointer to unsigned integer which can contain a GC pointer. */
typedef const RTGCUINTPTR *PCRTGCUINTPTR;

/** Unsigned integer which can contain a 32 bits GC pointer. */
typedef uint32_t        RTGCUINTPTR32;
/** Pointer to unsigned integer which can contain a 32 bits GC pointer. */
typedef RTGCUINTPTR32   *PRTGCUINTPTR32;
/** Pointer to unsigned integer which can contain a 32 bits GC pointer. */
typedef const RTGCUINTPTR32 *PCRTGCUINTPTR32;

/** Unsigned integer which can contain a 64 bits GC pointer. */
typedef uint64_t        RTGCUINTPTR64;
/** Pointer to unsigned integer which can contain a 32 bits GC pointer. */
typedef RTGCUINTPTR64   *PRTGCUINTPTR64;
/** Pointer to unsigned integer which can contain a 32 bits GC pointer. */
typedef const RTGCUINTPTR64 *PCRTGCUINTPTR64;

/** Guest Physical Memory Address.*/
typedef uint64_t            RTGCPHYS;
/** Pointer to Guest Physical Memory Address. */
typedef RTGCPHYS           *PRTGCPHYS;
/** Pointer to const Guest Physical Memory Address. */
typedef const RTGCPHYS     *PCRTGCPHYS;
/** @def NIL_RTGCPHYS
 * NIL GC Physical Address.
 * NIL_RTGCPHYS is used to signal an invalid physical address, similar
 * to the NULL pointer. Note that this value may actually be valid in
 * some contexts.
 */
#define NIL_RTGCPHYS        (~(RTGCPHYS)0U)
/** Max guest physical memory address value. */
#define RTGCPHYS_MAX        UINT64_MAX


/** Guest Physical Memory Address; limited to 32 bits.*/
typedef uint32_t            RTGCPHYS32;
/** Pointer to Guest Physical Memory Address. */
typedef RTGCPHYS32         *PRTGCPHYS32;
/** Pointer to const Guest Physical Memory Address. */
typedef const RTGCPHYS32    *PCRTGCPHYS32;
/** @def NIL_RTGCPHYS32
 * NIL GC Physical Address.
 * NIL_RTGCPHYS32 is used to signal an invalid physical address, similar
 * to the NULL pointer. Note that this value may actually be valid in
 * some contexts.
 */
#define NIL_RTGCPHYS32     (~(RTGCPHYS32)0)


/** Guest Physical Memory Address; limited to 64 bits.*/
typedef uint64_t        RTGCPHYS64;
/** Pointer to Guest Physical Memory Address. */
typedef RTGCPHYS64     *PRTGCPHYS64;
/** Pointer to const Guest Physical Memory Address. */
typedef const RTGCPHYS64 *PCRTGCPHYS64;
/** @def NIL_RTGCPHYS64
 * NIL GC Physical Address.
 * NIL_RTGCPHYS64 is used to signal an invalid physical address, similar
 * to the NULL pointer. Note that this value may actually be valid in
 * some contexts.
 */
#define NIL_RTGCPHYS64     (~(RTGCPHYS64)0)

/** Guest context pointer, 32 bits.
 * Keep in mind that this type is an unsigned integer in
 * HC and void pointer in GC.
 */
typedef RTGCUINTPTR32   RTGCPTR32;
/** Pointer to a guest context pointer. */
typedef RTGCPTR32      *PRTGCPTR32;
/** Pointer to a const guest context pointer. */
typedef const RTGCPTR32 *PCRTGCPTR32;
/** @def NIL_RTGCPTR32
 * NIL GC pointer.
 */
#define NIL_RTGCPTR32   ((RTGCPTR32)0)

/** Guest context pointer, 64 bits.
 */
typedef RTGCUINTPTR64   RTGCPTR64;
/** Pointer to a guest context pointer. */
typedef RTGCPTR64      *PRTGCPTR64;
/** Pointer to a const guest context pointer. */
typedef const RTGCPTR64 *PCRTGCPTR64;
/** @def NIL_RTGCPTR64
 * NIL GC pointer.
 */
#define NIL_RTGCPTR64   ((RTGCPTR64)0)

/** Guest context pointer.
 * Keep in mind that this type is an unsigned integer in
 * HC and void pointer in GC.
 */
#if GC_ARCH_BITS == 64
typedef RTGCPTR64       RTGCPTR;
/** Pointer to a guest context pointer. */
typedef PRTGCPTR64      PRTGCPTR;
/** Pointer to a const guest context pointer. */
typedef PCRTGCPTR64     PCRTGCPTR;
/** @def NIL_RTGCPTR
 * NIL GC pointer.
 */
# define NIL_RTGCPTR    NIL_RTGCPTR64
/** Max RTGCPTR value. */
# define RTGCPTR_MAX    UINT64_MAX
#elif GC_ARCH_BITS == 32
typedef RTGCPTR32       RTGCPTR;
/** Pointer to a guest context pointer. */
typedef PRTGCPTR32      PRTGCPTR;
/** Pointer to a const guest context pointer. */
typedef PCRTGCPTR32     PCRTGCPTR;
/** @def NIL_RTGCPTR
 * NIL GC pointer.
 */
# define NIL_RTGCPTR     NIL_RTGCPTR32
/** Max RTGCPTR value. */
# define RTGCPTR_MAX    UINT32_MAX
#else
# error "Unsupported GC_ARCH_BITS!"
#endif

/** Unsigned integer register in the guest context. */
typedef uint32_t              RTGCUINTREG32;
/** Pointer to an unsigned integer register in the guest context. */
typedef RTGCUINTREG32        *PRTGCUINTREG32;
/** Pointer to a const unsigned integer register in the guest context. */
typedef const RTGCUINTREG32  *PCRTGCUINTREG32;

typedef uint64_t              RTGCUINTREG64;
/** Pointer to an unsigned integer register in the guest context. */
typedef RTGCUINTREG64        *PRTGCUINTREG64;
/** Pointer to a const unsigned integer register in the guest context. */
typedef const RTGCUINTREG64  *PCRTGCUINTREG64;

#if GC_ARCH_BITS == 64
typedef RTGCUINTREG64           RTGCUINTREG;
#elif GC_ARCH_BITS == 32
typedef RTGCUINTREG32           RTGCUINTREG;
#else
# error "Unsupported GC_ARCH_BITS!"
#endif
/** Pointer to an unsigned integer register in the guest context. */
typedef RTGCUINTREG            *PRTGCUINTREG;
/** Pointer to a const unsigned integer register in the guest context. */
typedef const RTGCUINTREG      *PCRTGCUINTREG;

/** @} */

/** @defgroup grp_rt_types_rc  Raw mode Context Basic Types
 * @ingroup grp_rt_types
 * @{
 */

/** Raw mode context pointer; a 32 bits guest context pointer.
 * Keep in mind that this type is an unsigned integer in
 * HC and void pointer in RC.
 */
#ifdef IN_RC
typedef void *          RTRCPTR;
#else
typedef uint32_t        RTRCPTR;
#endif
/** Pointer to a raw mode context pointer. */
typedef RTRCPTR        *PRTRCPTR;
/** Pointer to a const raw mode context pointer. */
typedef const RTRCPTR  *PCRTRCPTR;
/** @def NIL_RTGCPTR
 * NIL RC pointer.
 */
#ifndef IN_RC
# define NIL_RTRCPTR   ((RTRCPTR)0)
#else
# define NIL_RTRCPTR   (NULL)
#endif
/** @def RTRCPTR_MAX
 * The maximum value a RTRCPTR can have. Mostly used as INVALID value.
 */
#define RTRCPTR_MAX    ((RTRCPTR)UINT32_MAX)

/** Raw mode context pointer, unsigned integer variant. */
typedef int32_t         RTRCINTPTR;
/** @def RTRCUINTPTR_MAX
 * The maximum value a RTRCUINPTR can have.
 */
#define RTRCUINTPTR_MAX ((RTRCUINTPTR)UINT32_MAX)

/** Raw mode context pointer, signed integer variant. */
typedef uint32_t        RTRCUINTPTR;
/** @def RTRCINTPTR_MIN
 * The minimum value a RTRCINPTR can have.
 */
#define RTRCINTPTR_MIN ((RTRCINTPTR)INT32_MIN)
/** @def RTRCINTPTR_MAX
 * The maximum value a RTRCINPTR can have.
 */
#define RTRCINTPTR_MAX ((RTRCINTPTR)INT32_MAX)

/** @} */


/** @defgroup grp_rt_types_cc  Current Context Basic Types
 * @ingroup grp_rt_types
 * @{
 */

/** Current Context Physical Memory Address.*/
#ifdef IN_RC
typedef RTGCPHYS RTCCPHYS;
#else
typedef RTHCPHYS RTCCPHYS;
#endif
/** Pointer to Current Context Physical Memory Address. */
typedef RTCCPHYS       *PRTCCPHYS;
/** Pointer to const Current Context Physical Memory Address. */
typedef const RTCCPHYS *PCRTCCPHYS;
/** @def NIL_RTCCPHYS
 * NIL CC Physical Address.
 * NIL_RTCCPHYS is used to signal an invalid physical address, similar
 * to the NULL pointer.
 */
#ifdef IN_RC
# define NIL_RTCCPHYS   NIL_RTGCPHYS
#else
# define NIL_RTCCPHYS   NIL_RTHCPHYS
#endif

/** Unsigned integer register in the current context. */
#if ARCH_BITS == 32
typedef uint32_t                RTCCUINTREG;
#elif ARCH_BITS == 64
typedef uint64_t                RTCCUINTREG;
#else
# error "Unsupported ARCH_BITS!"
#endif
/** Pointer to an unsigned integer register in the current context. */
typedef RTCCUINTREG            *PRTCCUINTREG;
/** Pointer to a const unsigned integer register in the current context. */
typedef RTCCUINTREG const      *PCRTCCUINTREG;

/** Signed integer register in the current context. */
#if ARCH_BITS == 32
typedef int32_t                 RTCCINTREG;
#elif ARCH_BITS == 64
typedef int64_t                 RTCCINTREG;
#endif
/** Pointer to a signed integer register in the current context. */
typedef RTCCINTREG             *PRTCCINTREG;
/** Pointer to a const signed integer register in the current context. */
typedef RTCCINTREG const       *PCRTCCINTREG;

/** @} */



/** Pointer to a big integer number. */
typedef struct RTBIGNUM                            *PRTBIGNUM;
/** Pointer to a const big integer number. */
typedef struct RTBIGNUM const                      *PCRTBIGNUM;


/** Pointer to a critical section. */
typedef struct RTCRITSECT                          *PRTCRITSECT;
/** Pointer to a const critical section. */
typedef const struct RTCRITSECT                    *PCRTCRITSECT;

/** Pointer to a read/write critical section. */
typedef struct RTCRITSECTRW                        *PRTCRITSECTRW;
/** Pointer to a const read/write critical section. */
typedef const struct RTCRITSECTRW                  *PCRTCRITSECTRW;


/** Condition variable handle. */
typedef R3PTRTYPE(struct RTCONDVARINTERNAL *)       RTCONDVAR;
/** Pointer to a condition variable handle. */
typedef RTCONDVAR                                  *PRTCONDVAR;
/** Nil condition variable handle. */
#define NIL_RTCONDVAR                               0

/** Cryptographic (certificate) store handle. */
typedef R3R0PTRTYPE(struct RTCRSTOREINT *)          RTCRSTORE;
/** Pointer to a Cryptographic (certificate) store handle. */
typedef RTCRSTORE                                  *PRTCRSTORE;
/** Nil Cryptographic (certificate) store handle. */
#define NIL_RTCRSTORE                               0

/** Pointer to a const (store) certificate context. */
typedef struct RTCRCERTCTX const                   *PCRTCRCERTCTX;

/** Cryptographic message digest handle. */
typedef R3R0PTRTYPE(struct RTCRDIGESTINT *)         RTCRDIGEST;
/** Pointer to a cryptographic message digest handle. */
typedef RTCRDIGEST                                 *PRTCRDIGEST;
/** NIL cryptographic message digest handle. */
#define NIL_RTCRDIGEST                              (0)

/** Public key encryption schema handle. */
typedef R3R0PTRTYPE(struct RTCRPKIXENCRYPTIONINT *) RTCRPKIXENCRYPTION;
/** Pointer to a public key encryption schema handle. */
typedef RTCRPKIXENCRYPTION                         *PRTCRPKIXENCRYPTION;
/** NIL publick key encryption schema handle */
#define NIL_RTCRPKIXENCRYPTION                      (0)

/** Public key signature schema handle. */
typedef R3R0PTRTYPE(struct RTCRPKIXSIGNATUREINT *)  RTCRPKIXSIGNATURE;
/** Pointer to a public key signature schema handle. */
typedef RTCRPKIXSIGNATURE                          *PRTCRPKIXSIGNATURE;
/** NIL publick key signature schema handle */
#define NIL_RTCRPKIXSIGNATURE                       (0)

/** X.509 certificate paths builder & validator handle. */
typedef R3R0PTRTYPE(struct RTCRX509CERTPATHSINT *)  RTCRX509CERTPATHS;
/** Pointer to a certificate paths builder & validator handle. */
typedef RTCRX509CERTPATHS                          *PRTCRX509CERTPATHS;
/** Nil certificate paths builder & validator handle. */
#define NIL_RTCRX509CERTPATHS                       0

/** File handle. */
typedef R3R0PTRTYPE(struct RTFILEINT *)             RTFILE;
/** Pointer to file handle. */
typedef RTFILE                                     *PRTFILE;
/** Nil file handle. */
#define NIL_RTFILE                                  ((RTFILE)~(RTHCINTPTR)0)

/** Async I/O request handle. */
typedef R3PTRTYPE(struct RTFILEAIOREQINTERNAL *)    RTFILEAIOREQ;
/** Pointer to an async I/O request handle. */
typedef RTFILEAIOREQ                               *PRTFILEAIOREQ;
/** Nil request handle. */
#define NIL_RTFILEAIOREQ                            0

/** Async I/O completion context handle. */
typedef R3PTRTYPE(struct RTFILEAIOCTXINTERNAL *)    RTFILEAIOCTX;
/** Pointer to an async I/O completion context handle. */
typedef RTFILEAIOCTX                               *PRTFILEAIOCTX;
/** Nil context handle. */
#define NIL_RTFILEAIOCTX                            0

/** Loader module handle. */
typedef R3R0PTRTYPE(struct RTLDRMODINTERNAL *)      RTLDRMOD;
/** Pointer to a loader module handle. */
typedef RTLDRMOD                                   *PRTLDRMOD;
/** Nil loader module handle. */
#define NIL_RTLDRMOD                                0

/** Lock validator class handle. */
typedef R3R0PTRTYPE(struct RTLOCKVALCLASSINT *)     RTLOCKVALCLASS;
/** Pointer to a lock validator class handle. */
typedef RTLOCKVALCLASS                             *PRTLOCKVALCLASS;
/** Nil lock validator class handle. */
#define NIL_RTLOCKVALCLASS                         ((RTLOCKVALCLASS)0)

/** Ring-0 memory object handle. */
typedef R0PTRTYPE(struct RTR0MEMOBJINTERNAL *)      RTR0MEMOBJ;
/** Pointer to a Ring-0 memory object handle. */
typedef RTR0MEMOBJ                                 *PRTR0MEMOBJ;
/** Nil ring-0 memory object handle. */
#define NIL_RTR0MEMOBJ                              0

/** Native thread handle. */
typedef RTHCUINTPTR                                 RTNATIVETHREAD;
/** Pointer to an native thread handle. */
typedef RTNATIVETHREAD                             *PRTNATIVETHREAD;
/** Nil native thread handle. */
#define NIL_RTNATIVETHREAD                          (~(RTNATIVETHREAD)0)

/** Pipe handle. */
typedef R3R0PTRTYPE(struct RTPIPEINTERNAL *)        RTPIPE;
/** Pointer to a pipe handle. */
typedef RTPIPE                                     *PRTPIPE;
/** Nil pipe handle.
 * @remarks This is not 0 because of UNIX and OS/2 handle values. Take care! */
#define NIL_RTPIPE                                 ((RTPIPE)RTHCUINTPTR_MAX)

/** @typedef RTPOLLSET
 * Poll set handle. */
typedef R3R0PTRTYPE(struct RTPOLLSETINTERNAL *)     RTPOLLSET;
/** Pointer to a poll set handle. */
typedef RTPOLLSET                                  *PRTPOLLSET;
/** Nil poll set handle handle. */
#define NIL_RTPOLLSET                               ((RTPOLLSET)0)

/** Process identifier. */
typedef uint32_t                                    RTPROCESS;
/** Pointer to a process identifier. */
typedef RTPROCESS                                  *PRTPROCESS;
/** Nil process identifier. */
#define NIL_RTPROCESS                               (~(RTPROCESS)0)

/** Process ring-0 handle. */
typedef RTR0UINTPTR                                 RTR0PROCESS;
/** Pointer to a ring-0 process handle. */
typedef RTR0PROCESS                                *PRTR0PROCESS;
/** Nil ring-0 process handle. */
#define NIL_RTR0PROCESS                             (~(RTR0PROCESS)0)

/** @typedef RTSEMEVENT
 * Event Semaphore handle. */
typedef R3R0PTRTYPE(struct RTSEMEVENTINTERNAL *)    RTSEMEVENT;
/** Pointer to an event semaphore handle. */
typedef RTSEMEVENT                                 *PRTSEMEVENT;
/** Nil event semaphore handle. */
#define NIL_RTSEMEVENT                              0

/** @typedef RTSEMEVENTMULTI
 * Event Multiple Release Semaphore handle. */
typedef R3R0PTRTYPE(struct RTSEMEVENTMULTIINTERNAL *) RTSEMEVENTMULTI;
/** Pointer to an event multiple release semaphore handle. */
typedef RTSEMEVENTMULTI                            *PRTSEMEVENTMULTI;
/** Nil multiple release event semaphore handle. */
#define NIL_RTSEMEVENTMULTI                         0

/** @typedef RTSEMFASTMUTEX
 * Fast mutex Semaphore handle. */
typedef R3R0PTRTYPE(struct RTSEMFASTMUTEXINTERNAL *) RTSEMFASTMUTEX;
/** Pointer to a fast mutex semaphore handle. */
typedef RTSEMFASTMUTEX                             *PRTSEMFASTMUTEX;
/** Nil fast mutex semaphore handle. */
#define NIL_RTSEMFASTMUTEX                          0

/** @typedef RTSEMMUTEX
 * Mutex Semaphore handle. */
typedef R3R0PTRTYPE(struct RTSEMMUTEXINTERNAL *)    RTSEMMUTEX;
/** Pointer to a mutex semaphore handle. */
typedef RTSEMMUTEX                                 *PRTSEMMUTEX;
/** Nil mutex semaphore handle. */
#define NIL_RTSEMMUTEX                              0

/** @typedef RTSEMSPINMUTEX
 * Spinning mutex Semaphore handle. */
typedef R3R0PTRTYPE(struct RTSEMSPINMUTEXINTERNAL *) RTSEMSPINMUTEX;
/** Pointer to a spinning mutex semaphore handle. */
typedef RTSEMSPINMUTEX                             *PRTSEMSPINMUTEX;
/** Nil spinning mutex semaphore handle. */
#define NIL_RTSEMSPINMUTEX                          0

/** @typedef RTSEMRW
 * Read/Write Semaphore handle. */
typedef R3R0PTRTYPE(struct RTSEMRWINTERNAL *)       RTSEMRW;
/** Pointer to a read/write semaphore handle. */
typedef RTSEMRW                                    *PRTSEMRW;
/** Nil read/write semaphore handle. */
#define NIL_RTSEMRW                                 0

/** @typedef RTSEMXROADS
 * Crossroads semaphore handle. */
typedef R3R0PTRTYPE(struct RTSEMXROADSINTERNAL *)   RTSEMXROADS;
/** Pointer to a crossroads semaphore handle. */
typedef RTSEMXROADS                                *PRTSEMXROADS;
/** Nil crossroads semaphore handle. */
#define NIL_RTSEMXROADS                             ((RTSEMXROADS)0)

/** Spinlock handle. */
typedef R3R0PTRTYPE(struct RTSPINLOCKINTERNAL *)    RTSPINLOCK;
/** Pointer to a spinlock handle. */
typedef RTSPINLOCK                                 *PRTSPINLOCK;
/** Nil spinlock handle. */
#define NIL_RTSPINLOCK                              0

/** Socket handle. */
typedef R3R0PTRTYPE(struct RTSOCKETINT *)           RTSOCKET;
/** Pointer to socket handle. */
typedef RTSOCKET                                   *PRTSOCKET;
/** Nil socket handle. */
#define NIL_RTSOCKET                                ((RTSOCKET)0)

/** Pointer to a RTTCPSERVER handle. */
typedef struct RTTCPSERVER                         *PRTTCPSERVER;
/** Pointer to a RTTCPSERVER handle. */
typedef PRTTCPSERVER                               *PPRTTCPSERVER;
/** Nil RTTCPSERVER handle. */
#define NIL_RTTCPSERVER                            ((PRTTCPSERVER)0)

/** Pointer to a RTUDPSERVER handle. */
typedef struct RTUDPSERVER                         *PRTUDPSERVER;
/** Pointer to a RTUDPSERVER handle. */
typedef PRTUDPSERVER                               *PPRTUDPSERVER;
/** Nil RTUDPSERVER handle. */
#define NIL_RTUDPSERVER                            ((PRTUDPSERVER)0)

/** Thread handle.*/
typedef R3R0PTRTYPE(struct RTTHREADINT *)           RTTHREAD;
/** Pointer to thread handle. */
typedef RTTHREAD                                   *PRTTHREAD;
/** Nil thread handle. */
#define NIL_RTTHREAD                                0

/** Thread-context handle.*/
typedef R0PTRTYPE(struct RTTHREADCTXINT *)          RTTHREADCTX;
/** Pointer to thread handle. */
typedef RTTHREADCTX                                *PRTTHREADCTX;
/** Nil thread-context handle. */
#define NIL_RTTHREADCTX                             0

/** A TLS index. */
typedef RTHCINTPTR                                  RTTLS;
/** Pointer to a TLS index. */
typedef RTTLS                                      *PRTTLS;
/** Pointer to a const TLS index. */
typedef RTTLS const                                *PCRTTLS;
/** NIL TLS index value. */
#define NIL_RTTLS                                   ((RTTLS)-1)

/** Trace buffer handle.
 * @remarks This is not a R3/R0 type like most other handles!
 */
typedef struct RTTRACEBUFINT                        *RTTRACEBUF;
/** Poiner to a trace buffer handle. */
typedef RTTRACEBUF                                  *PRTTRACEBUF;
/** Nil trace buffer handle. */
#define NIL_RTTRACEBUF                              ((RTTRACEBUF)0)
/** The handle of the default trace buffer.
 * This can be used with any of the RTTraceBufAdd APIs. */
#define RTTRACEBUF_DEFAULT                          ((RTTRACEBUF)-2)

/** Handle to a simple heap. */
typedef R3R0PTRTYPE(struct RTHEAPSIMPLEINTERNAL *)  RTHEAPSIMPLE;
/** Pointer to a handle to a simple heap. */
typedef RTHEAPSIMPLE                               *PRTHEAPSIMPLE;
/** NIL simple heap handle. */
#define NIL_RTHEAPSIMPLE                            ((RTHEAPSIMPLE)0)

/** Handle to an offset based heap. */
typedef R3R0PTRTYPE(struct RTHEAPOFFSETINTERNAL *)  RTHEAPOFFSET;
/** Pointer to a handle to an offset based heap. */
typedef RTHEAPOFFSET                               *PRTHEAPOFFSET;
/** NIL offset based heap handle. */
#define NIL_RTHEAPOFFSET                            ((RTHEAPOFFSET)0)

/** Handle to an environment block. */
typedef R3PTRTYPE(struct RTENVINTERNAL *)           RTENV;
/** Pointer to a handle to an environment block. */
typedef RTENV                                      *PRTENV;
/** NIL simple heap handle. */
#define NIL_RTENV                                   ((RTENV)0)

/** A CPU identifier.
 * @remarks This doesn't have to correspond to the APIC ID (intel/amd). Nor
 *          does it have to correspond to the bits in the affinity mask, at
 *          least not until we've sorted out Windows NT. */
typedef uint32_t                                    RTCPUID;
/** Pointer to a CPU identifier. */
typedef RTCPUID                                    *PRTCPUID;
/** Pointer to a const CPU identifier. */
typedef RTCPUID const                              *PCRTCPUID;
/** Nil CPU Id. */
#define NIL_RTCPUID                                 ((RTCPUID)~0)

/** The maximum number of CPUs a set can contain and IPRT is able
 * to reference. (Should be max of support arch/platforms.)
 * @remarks Must be a multiple of 64 (see RTCPUSET).  */
#if defined(RT_ARCH_X86) || defined(RT_ARCH_AMD64)
# define RTCPUSET_MAX_CPUS      256
#elif defined(RT_ARCH_SPARC) || defined(RT_ARCH_SPARC64)
# define RTCPUSET_MAX_CPUS      1024
#else
# define RTCPUSET_MAX_CPUS      64
#endif
/** A CPU set.
 * @note    Treat this as an opaque type and always use RTCpuSet* for
 *          manupulating it. */
typedef struct RTCPUSET
{
    /** The bitmap.  */
    uint64_t bmSet[RTCPUSET_MAX_CPUS / 64];
} RTCPUSET;
/** Pointer to a CPU set. */
typedef RTCPUSET                                   *PRTCPUSET;
/** Pointer to a const CPU set. */
typedef RTCPUSET const                             *PCRTCPUSET;

/** A handle table handle. */
typedef R3R0PTRTYPE(struct RTHANDLETABLEINT *)      RTHANDLETABLE;
/** A pointer to a handle table handle. */
typedef RTHANDLETABLE                              *PRTHANDLETABLE;
/** @def NIL_RTHANDLETABLE
 * NIL handle table handle. */
#define NIL_RTHANDLETABLE                           ((RTHANDLETABLE)0)

/** A handle to a low resolution timer. */
typedef R3R0PTRTYPE(struct RTTIMERLRINT *)          RTTIMERLR;
/** A pointer to a low resolution timer handle. */
typedef RTTIMERLR                                  *PRTTIMERLR;
/** @def NIL_RTTIMERLR
 * NIL low resolution timer handle value. */
#define NIL_RTTIMERLR                               ((RTTIMERLR)0)

/** Handle to a random number generator. */
typedef R3R0PTRTYPE(struct RTRANDINT *)             RTRAND;
/** Pointer to a random number generator handle. */
typedef RTRAND                                     *PRTRAND;
/** NIL random number genrator handle value. */
#define NIL_RTRAND                                  ((RTRAND)0)

/** Debug address space handle. */
typedef R3R0PTRTYPE(struct RTDBGASINT *)            RTDBGAS;
/** Pointer to a debug address space handle. */
typedef RTDBGAS                                    *PRTDBGAS;
/** NIL debug address space handle. */
#define NIL_RTDBGAS                                 ((RTDBGAS)0)

/** Debug module handle. */
typedef R3R0PTRTYPE(struct RTDBGMODINT *)           RTDBGMOD;
/** Pointer to a debug module handle. */
typedef RTDBGMOD                                   *PRTDBGMOD;
/** NIL debug module handle. */
#define NIL_RTDBGMOD                                ((RTDBGMOD)0)

/** Manifest handle. */
typedef struct RTMANIFESTINT                       *RTMANIFEST;
/** Pointer to a manifest handle. */
typedef RTMANIFEST                                 *PRTMANIFEST;
/** NIL manifest handle. */
#define NIL_RTMANIFEST                              ((RTMANIFEST)~(uintptr_t)0)

/** Memory pool handle. */
typedef R3R0PTRTYPE(struct RTMEMPOOLINT *)          RTMEMPOOL;
/** Pointer to a memory pool handle. */
typedef RTMEMPOOL                                  *PRTMEMPOOL;
/** NIL memory pool handle. */
#define NIL_RTMEMPOOL                               ((RTMEMPOOL)0)
/** The default memory pool handle. */
#define RTMEMPOOL_DEFAULT                           ((RTMEMPOOL)-2)

/** String cache handle. */
typedef R3R0PTRTYPE(struct RTSTRCACHEINT *)         RTSTRCACHE;
/** Pointer to a string cache handle. */
typedef RTSTRCACHE                                 *PRTSTRCACHE;
/** NIL string cache handle. */
#define NIL_RTSTRCACHE                              ((RTSTRCACHE)0)
/** The default string cache handle. */
#define RTSTRCACHE_DEFAULT                          ((RTSTRCACHE)-2)


/** Virtual Filesystem handle. */
typedef struct RTVFSINTERNAL                       *RTVFS;
/** Pointer to a VFS handle. */
typedef RTVFS                                      *PRTVFS;
/** A NIL VFS handle. */
#define NIL_RTVFS                                   ((RTVFS)~(uintptr_t)0)

/** Virtual Filesystem base object handle. */
typedef struct RTVFSOBJINTERNAL                    *RTVFSOBJ;
/** Pointer to a VFS base object handle. */
typedef RTVFSOBJ                                   *PRTVFSOBJ;
/** A NIL VFS base object handle. */
#define NIL_RTVFSOBJ                                ((RTVFSOBJ)~(uintptr_t)0)

/** Virtual Filesystem directory handle. */
typedef struct RTVFSDIRINTERNAL                    *RTVFSDIR;
/** Pointer to a VFS directory handle. */
typedef RTVFSDIR                                   *PRTVFSDIR;
/** A NIL VFS directory handle. */
#define NIL_RTVFSDIR                                ((RTVFSDIR)~(uintptr_t)0)

/** Virtual Filesystem filesystem stream handle. */
typedef struct RTVFSFSSTREAMINTERNAL               *RTVFSFSSTREAM;
/** Pointer to a VFS filesystem stream handle. */
typedef RTVFSFSSTREAM                              *PRTVFSFSSTREAM;
/** A NIL VFS filesystem stream handle. */
#define NIL_RTVFSFSSTREAM                           ((RTVFSFSSTREAM)~(uintptr_t)0)

/** Virtual Filesystem I/O stream handle. */
typedef struct RTVFSIOSTREAMINTERNAL               *RTVFSIOSTREAM;
/** Pointer to a VFS I/O stream handle. */
typedef RTVFSIOSTREAM                              *PRTVFSIOSTREAM;
/** A NIL VFS I/O stream handle. */
#define NIL_RTVFSIOSTREAM                           ((RTVFSIOSTREAM)~(uintptr_t)0)

/** Virtual Filesystem file handle. */
typedef struct RTVFSFILEINTERNAL                   *RTVFSFILE;
/** Pointer to a VFS file handle. */
typedef RTVFSFILE                                  *PRTVFSFILE;
/** A NIL VFS file handle. */
#define NIL_RTVFSFILE                               ((RTVFSFILE)~(uintptr_t)0)

/** Virtual Filesystem symbolic link handle. */
typedef struct RTVFSSYMLINKINTERNAL                *RTVFSSYMLINK;
/** Pointer to a VFS symbolic link handle. */
typedef RTVFSSYMLINK                               *PRTVFSSYMLINK;
/** A NIL VFS symbolic link handle. */
#define NIL_RTVFSSYMLINK                            ((RTVFSSYMLINK)~(uintptr_t)0)

/** Async I/O manager handle. */
typedef struct RTAIOMGRINT                         *RTAIOMGR;
/** Pointer to a async I/O manager handle. */
typedef RTAIOMGR                                   *PRTAIOMGR;
/** A NIL async I/O manager handle. */
#define NIL_RTAIOMGR                                ((RTAIOMGR)~(uintptr_t)0)

/** Async I/O manager file handle. */
typedef struct RTAIOMGRFILEINT                     *RTAIOMGRFILE;
/** Pointer to a async I/O manager file handle. */
typedef RTAIOMGRFILE                               *PRTAIOMGRFILE;
/** A NIL async I/O manager file handle. */
#define NIL_RTAIOMGRFILE                            ((RTAIOMGRFILE)~(uintptr_t)0)

/**
 * Handle type.
 *
 * This is usually used together with RTHANDLEUNION.
 */
typedef enum RTHANDLETYPE
{
    /** The invalid zero value. */
    RTHANDLETYPE_INVALID = 0,
    /** File handle. */
    RTHANDLETYPE_FILE,
    /** Pipe handle */
    RTHANDLETYPE_PIPE,
    /** Socket handle. */
    RTHANDLETYPE_SOCKET,
    /** Thread handle. */
    RTHANDLETYPE_THREAD,
    /** The end of the valid values. */
    RTHANDLETYPE_END,
    /** The 32-bit type blow up. */
    RTHANDLETYPE_32BIT_HACK = 0x7fffffff
} RTHANDLETYPE;
/** Pointer to a handle type. */
typedef RTHANDLETYPE *PRTHANDLETYPE;

/**
 * Handle union.
 *
 * This is usually used together with RTHANDLETYPE or as RTHANDLE.
 */
typedef union RTHANDLEUNION
{
    RTFILE          hFile;              /**< File handle. */
    RTPIPE          hPipe;              /**< Pipe handle. */
    RTSOCKET        hSocket;            /**< Socket handle. */
    RTTHREAD        hThread;            /**< Thread handle. */
    /** Generic integer handle value.
     * Note that RTFILE is not yet pointer sized, so accessing it via this member
     * isn't necessarily safe or fully portable. */
    RTHCUINTPTR     uInt;
} RTHANDLEUNION;
/** Pointer to a handle union. */
typedef RTHANDLEUNION *PRTHANDLEUNION;
/** Pointer to a const handle union. */
typedef RTHANDLEUNION const *PCRTHANDLEUNION;

/**
 * Generic handle.
 */
typedef struct RTHANDLE
{
    /** The handle type. */
    RTHANDLETYPE    enmType;
    /** The handle value. */
    RTHANDLEUNION   u;
} RTHANDLE;
/** Pointer to a generic handle. */
typedef RTHANDLE *PRTHANDLE;
/** Pointer to a const generic handle. */
typedef RTHANDLE const *PCRTHANDLE;


/**
 * Standard handles.
 *
 * @remarks These have the correct file descriptor values for unixy systems and
 *          can be used directly in code specific to those platforms.
 */
typedef enum RTHANDLESTD
{
    /** Invalid standard handle. */
    RTHANDLESTD_INVALID = -1,
    /** The standard input handle. */
    RTHANDLESTD_INPUT = 0,
    /** The standard output handle. */
    RTHANDLESTD_OUTPUT,
    /** The standard error handle. */
    RTHANDLESTD_ERROR,
    /** The typical 32-bit type hack. */
    RTHANDLESTD_32BIT_HACK = 0x7fffffff
} RTHANDLESTD;


/**
 * Error info.
 *
 * See RTErrInfo*.
 */
typedef struct RTERRINFO
{
    /** Flags, see RTERRINFO_FLAGS_XXX. */
    uint32_t    fFlags;
    /** The status code. */
    int32_t     rc;
    /** The size of the message  */
    size_t      cbMsg;
    /** The error buffer. */
    char       *pszMsg;
    /** Reserved for future use. */
    void       *apvReserved[2];
} RTERRINFO;
/** Pointer to an error info structure. */
typedef RTERRINFO *PRTERRINFO;
/** Pointer to a const error info structure. */
typedef RTERRINFO const *PCRTERRINFO;

/**
 * Static error info structure, see RTErrInfoInitStatic.
 */
typedef struct RTERRINFOSTATIC
{
    /** The core error info. */
    RTERRINFO   Core;
    /** The static message buffer. */
    char        szMsg[3072];
} RTERRINFOSTATIC;
/** Pointer to a error info buffer. */
typedef RTERRINFOSTATIC *PRTERRINFOSTATIC;
/** Pointer to a const static error info buffer. */
typedef RTERRINFOSTATIC const *PCRTERRINFOSTATIC;


/**
 * UUID data type.
 *
 * See RTUuid*.
 *
 * @remarks IPRT defines that the first three integers in the @c Gen struct
 *          interpretation are in little endian representation.  This is
 *          different to many other UUID implementation, and requires
 *          conversion if you need to achieve consistent results.
 */
typedef union RTUUID
{
    /** 8-bit view. */
    uint8_t     au8[16];
    /** 16-bit view. */
    uint16_t    au16[8];
    /** 32-bit view. */
    uint32_t    au32[4];
    /** 64-bit view. */
    uint64_t    au64[2];
    /** The way the UUID is declared by the DCE specification. */
    struct
    {
        uint32_t    u32TimeLow;
        uint16_t    u16TimeMid;
        uint16_t    u16TimeHiAndVersion;
        uint8_t     u8ClockSeqHiAndReserved;
        uint8_t     u8ClockSeqLow;
        uint8_t     au8Node[6];
    } Gen;
} RTUUID;
/** Pointer to UUID data. */
typedef RTUUID *PRTUUID;
/** Pointer to readonly UUID data. */
typedef const RTUUID *PCRTUUID;

/** Initializes a RTUUID structure with all zeros (RTUuidIsNull() true). */
#define RTUUID_INITIALIZE_NULL  { { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 } }

/** UUID string maximum length. */
#define RTUUID_STR_LENGTH       37


/** Compression handle. */
typedef struct RTZIPCOMP   *PRTZIPCOMP;
/** Decompressor handle. */
typedef struct RTZIPDECOMP *PRTZIPDECOMP;


/**
 * Unicode Code Point.
 */
typedef uint32_t            RTUNICP;
/** Pointer to an Unicode Code Point. */
typedef RTUNICP            *PRTUNICP;
/** Pointer to an Unicode Code Point. */
typedef const RTUNICP      *PCRTUNICP;
/** Max value a RTUNICP type can hold. */
#define RTUNICP_MAX         ( ~(RTUNICP)0 )
/** Invalid code point.
 * This is returned when encountered invalid encodings or invalid
 * unicode code points. */
#define RTUNICP_INVALID     ( UINT32_C(0xfffffffe) )


/**
 * UTF-16 character.
 * @remark  wchar_t is not usable since it's compiler defined.
 * @remark  When we use the term character we're not talking about unicode code point, but
 *          the basic unit of the string encoding. Thus cwc - count of wide chars - means
 *          count of RTUTF16; cuc - count of unicode chars - means count of RTUNICP;
 *          and cch means count of the typedef 'char', which is assumed to be an octet.
 */
typedef uint16_t        RTUTF16;
/** Pointer to a UTF-16 character. */
typedef RTUTF16        *PRTUTF16;
/** Pointer to a const UTF-16 character. */
typedef const RTUTF16  *PCRTUTF16;


/**
 * Wait for ever if we have to.
 */
#define RT_INDEFINITE_WAIT      (~0U)


/**
 * Generic process callback.
 *
 * @returns VBox status code. Failure will cancel the operation.
 * @param   uPercentage     The percentage of the operation which has been completed.
 * @param   pvUser          The user specified argument.
 */
typedef DECLCALLBACK(int) FNRTPROGRESS(unsigned uPrecentage, void *pvUser);
/** Pointer to a generic progress callback function, FNRTPROCESS(). */
typedef FNRTPROGRESS *PFNRTPROGRESS;

/**
 * Generic vprintf-like callback function for dumpers.
 *
 * @param   pvUser          User argument.
 * @param   pszFormat       The format string.
 * @param   va              Arguments for the format string.
 */
typedef DECLCALLBACK(void) FNRTDUMPPRINTFV(void *pvUser, const char *pszFormat, va_list va);
/** Pointer to a generic printf-like function for dumping. */
typedef FNRTDUMPPRINTFV *PFNRTDUMPPRINTFV;


/**
 * A point in a two dimentional coordinate system.
 */
typedef struct RTPOINT
{
    /** X coordinate. */
    int32_t     x;
    /** Y coordinate. */
    int32_t     y;
} RTPOINT;
/** Pointer to a point. */
typedef RTPOINT *PRTPOINT;
/** Pointer to a const point. */
typedef const RTPOINT *PCRTPOINT;


/**
 * Rectangle data type, double point.
 */
typedef struct RTRECT
{
    /** left X coordinate. */
    int32_t     xLeft;
    /** top Y coordinate. */
    int32_t     yTop;
    /** right X coordinate. (exclusive) */
    int32_t     xRight;
    /** bottom Y coordinate. (exclusive) */
    int32_t     yBottom;
} RTRECT;
/** Pointer to a double point rectangle. */
typedef RTRECT *PRTRECT;
/** Pointer to a const double point rectangle. */
typedef const RTRECT *PCRTRECT;


/**
 * Rectangle data type, point + size.
 */
typedef struct RTRECT2
{
    /** X coordinate.
     * Unless stated otherwise, this is the top left corner. */
    int32_t     x;
    /** Y coordinate.
     * Unless stated otherwise, this is the top left corner.  */
    int32_t     y;
    /** The width.
     * Unless stated otherwise, this is to the right of (x,y) and will not
     * be a negative number. */
    int32_t     cx;
    /** The height.
     * Unless stated otherwise, this is down from (x,y) and will not be a
     * negative number. */
    int32_t     cy;
} RTRECT2;
/** Pointer to a point + size rectangle. */
typedef RTRECT2 *PRTRECT2;
/** Pointer to a const point + size rectangle. */
typedef const RTRECT2 *PCRTRECT2;


/**
 * The size of a rectangle.
 */
typedef struct RTRECTSIZE
{
    /** The width (along the x-axis). */
    uint32_t    cx;
    /** The height (along the y-axis). */
    uint32_t    cy;
} RTRECTSIZE;
/** Pointer to a rectangle size. */
typedef RTRECTSIZE *PRTRECTSIZE;
/** Pointer to a const rectangle size. */
typedef const RTRECTSIZE *PCRTRECTSIZE;


/**
 * Ethernet MAC address.
 *
 * The first 24 bits make up the Organisationally Unique Identifier (OUI),
 * where the first bit (little endian) indicates multicast (set) / unicast,
 * and the second bit indicates locally (set) / global administered. If all
 * bits are set, it's a broadcast.
 */
typedef union RTMAC
{
    /** @todo add a bitfield view of this stuff. */
    /** 8-bit view. */
    uint8_t     au8[6];
    /** 16-bit view. */
    uint16_t    au16[3];
} RTMAC;
/** Pointer to a MAC address. */
typedef RTMAC *PRTMAC;
/** Pointer to a readonly MAC address. */
typedef const RTMAC *PCRTMAC;


/** Pointer to a lock validator record.
 * The structure definition is found in iprt/lockvalidator.h.  */
typedef struct RTLOCKVALRECEXCL        *PRTLOCKVALRECEXCL;
/** Pointer to a record of one ownership share.
 * The structure definition is found in iprt/lockvalidator.h.  */
typedef struct RTLOCKVALRECSHRD        *PRTLOCKVALRECSHRD;
/** Pointer to a lock validator source poisition.
 * The structure definition is found in iprt/lockvalidator.h.  */
typedef struct RTLOCKVALSRCPOS         *PRTLOCKVALSRCPOS;
/** Pointer to a const lock validator source poisition.
 * The structure definition is found in iprt/lockvalidator.h.  */
typedef struct RTLOCKVALSRCPOS const   *PCRTLOCKVALSRCPOS;

/** @name   Special sub-class values.
 * The range 16..UINT32_MAX is available to the user, the range 0..15 is
 * reserved for the lock validator.  In the user range the locks can only be
 * taking in ascending order.
 * @{ */
/** Invalid value.  */
#define RTLOCKVAL_SUB_CLASS_INVALID     UINT32_C(0)
/** Not allowed to be taken with any other locks in the same class.
  * This is the recommended value.  */
#define RTLOCKVAL_SUB_CLASS_NONE        UINT32_C(1)
/** Any order is allowed within the class. */
#define RTLOCKVAL_SUB_CLASS_ANY         UINT32_C(2)
/** The first user value. */
#define RTLOCKVAL_SUB_CLASS_USER        UINT32_C(16)
/** @} */


/**
 * Digest types.
 */
typedef enum RTDIGESTTYPE
{
    /** Invalid digest value.  */
    RTDIGESTTYPE_INVALID = 0,
    /** Unknown digest type. */
    RTDIGESTTYPE_UNKNOWN,
    /** CRC32 checksum. */
    RTDIGESTTYPE_CRC32,
    /** CRC64 checksum. */
    RTDIGESTTYPE_CRC64,
    /** MD2 checksum (unsafe!). */
    RTDIGESTTYPE_MD2,
    /** MD4 checksum (unsafe!!). */
    RTDIGESTTYPE_MD4,
    /** MD5 checksum (unsafe!). */
    RTDIGESTTYPE_MD5,
    /** SHA-1 checksum (unsafe!). */
    RTDIGESTTYPE_SHA1,
    /** SHA-224 checksum. */
    RTDIGESTTYPE_SHA224,
    /** SHA-256 checksum. */
    RTDIGESTTYPE_SHA256,
    /** SHA-384 checksum. */
    RTDIGESTTYPE_SHA384,
    /** SHA-512 checksum. */
    RTDIGESTTYPE_SHA512,
    /** SHA-512/224 checksum. */
    RTDIGESTTYPE_SHA512T224,
    /** SHA-512/256 checksum. */
    RTDIGESTTYPE_SHA512T256,
    /** End of valid types. */
    RTDIGESTTYPE_END,
    /** Usual 32-bit type blowup. */
    RTDIGESTTYPE_32BIT_HACK = 0x7fffffff
} RTDIGESTTYPE;

/**
 * Process exit codes.
 */
typedef enum RTEXITCODE
{
    /** Success. */
    RTEXITCODE_SUCCESS = 0,
    /** General failure. */
    RTEXITCODE_FAILURE = 1,
    /** Invalid arguments.  */
    RTEXITCODE_SYNTAX = 2,
    /** Initialization failure (usually IPRT, but could be used for other
     *  components as well). */
    RTEXITCODE_INIT = 3,
    /** Test skipped. */
    RTEXITCODE_SKIPPED = 4,
    /** The end of valid exit codes. */
    RTEXITCODE_END,
    /** The usual 32-bit type hack. */
    RTEXITCODE_32BIT_HACK = 0x7fffffff
} RTEXITCODE;

/**
 * Range descriptor.
 */
typedef struct RTRANGE
{
    /** Start offset. */
    uint64_t    offStart;
    /** Range size. */
    size_t      cbRange;
} RTRANGE;
/** Pointer to a range descriptor. */
typedef RTRANGE *PRTRANGE;
/** Pointer to a readonly range descriptor. */
typedef const RTRANGE *PCRTRANGE;


/**
 * Generic pointer union.
 */
typedef union RTPTRUNION
{
    /** Pointer into the void... */
    void                   *pv;
    /** As a signed integer. */
    intptr_t                i;
    /** As an unsigned integer. */
    intptr_t                u;
    /** Pointer to char value. */
    char                   *pch;
    /** Pointer to char value. */
    unsigned char          *puch;
    /** Pointer to a int value. */
    int                    *pi;
    /** Pointer to a unsigned int value. */
    unsigned int           *pu;
    /** Pointer to a long value. */
    long                   *pl;
    /** Pointer to a long value. */
    unsigned long          *pul;
    /** Pointer to a 8-bit unsigned value. */
    uint8_t                *pu8;
    /** Pointer to a 16-bit unsigned value. */
    uint16_t               *pu16;
    /** Pointer to a 32-bit unsigned value. */
    uint32_t               *pu32;
    /** Pointer to a 64-bit unsigned value. */
    uint64_t               *pu64;
    /** Pointer to a UTF-16 character. */
    PRTUTF16                pwc;
    /** Pointer to a UUID character. */
    PRTUUID                 pUuid;
} RTPTRUNION;
/** Pointer to a pointer union. */
typedef RTPTRUNION *PRTPTRUNION;

/**
 * Generic const pointer union.
 */
typedef union RTCPTRUNION
{
    /** Pointer into the void... */
    void const             *pv;
    /** As a signed integer. */
    intptr_t                i;
    /** As an unsigned integer. */
    intptr_t                u;
    /** Pointer to char value. */
    char const             *pch;
    /** Pointer to char value. */
    unsigned char const    *puch;
    /** Pointer to a int value. */
    int const              *pi;
    /** Pointer to a unsigned int value. */
    unsigned int const     *pu;
    /** Pointer to a long value. */
    long const             *pl;
    /** Pointer to a long value. */
    unsigned long const    *pul;
    /** Pointer to a 8-bit unsigned value. */
    uint8_t const          *pu8;
    /** Pointer to a 16-bit unsigned value. */
    uint16_t const         *pu16;
    /** Pointer to a 32-bit unsigned value. */
    uint32_t const         *pu32;
    /** Pointer to a 64-bit unsigned value. */
    uint64_t const         *pu64;
    /** Pointer to a UTF-16 character. */
    PCRTUTF16               pwc;
    /** Pointer to a UUID character. */
    PCRTUUID                pUuid;
} RTCPTRUNION;
/** Pointer to a const pointer union. */
typedef RTCPTRUNION *PRTCPTRUNION;

/**
 * Generic volatile pointer union.
 */
typedef union RTVPTRUNION
{
    /** Pointer into the void... */
    void volatile          *pv;
    /** As a signed integer. */
    intptr_t                i;
    /** As an unsigned integer. */
    intptr_t                u;
    /** Pointer to char value. */
    char volatile          *pch;
    /** Pointer to char value. */
    unsigned char volatile *puch;
    /** Pointer to a int value. */
    int volatile           *pi;
    /** Pointer to a unsigned int value. */
    unsigned int volatile  *pu;
    /** Pointer to a long value. */
    long volatile          *pl;
    /** Pointer to a long value. */
    unsigned long volatile *pul;
    /** Pointer to a 8-bit unsigned value. */
    uint8_t volatile       *pu8;
    /** Pointer to a 16-bit unsigned value. */
    uint16_t volatile      *pu16;
    /** Pointer to a 32-bit unsigned value. */
    uint32_t volatile      *pu32;
    /** Pointer to a 64-bit unsigned value. */
    uint64_t volatile      *pu64;
    /** Pointer to a UTF-16 character. */
    RTUTF16 volatile       *pwc;
    /** Pointer to a UUID character. */
    RTUUID volatile        *pUuid;
} RTVPTRUNION;
/** Pointer to a const pointer union. */
typedef RTVPTRUNION *PRTVPTRUNION;

/**
 * Generic const volatile pointer union.
 */
typedef union RTCVPTRUNION
{
    /** Pointer into the void... */
    void const volatile            *pv;
    /** As a signed integer. */
    intptr_t                        i;
    /** As an unsigned integer. */
    intptr_t                        u;
    /** Pointer to char value. */
    char const volatile            *pch;
    /** Pointer to char value. */
    unsigned char const volatile   *puch;
    /** Pointer to a int value. */
    int const volatile             *pi;
    /** Pointer to a unsigned int value. */
    unsigned int const volatile    *pu;
    /** Pointer to a long value. */
    long const volatile            *pl;
    /** Pointer to a long value. */
    unsigned long const volatile   *pul;
    /** Pointer to a 8-bit unsigned value. */
    uint8_t const volatile         *pu8;
    /** Pointer to a 16-bit unsigned value. */
    uint16_t const volatile        *pu16;
    /** Pointer to a 32-bit unsigned value. */
    uint32_t const volatile        *pu32;
    /** Pointer to a 64-bit unsigned value. */
    uint64_t const volatile        *pu64;
    /** Pointer to a UTF-16 character. */
    RTUTF16 const volatile         *pwc;
    /** Pointer to a UUID character. */
    RTUUID const volatile          *pUuid;
} RTCVPTRUNION;
/** Pointer to a const pointer union. */
typedef RTCVPTRUNION *PRTCVPTRUNION;



#ifdef __cplusplus
/**
 * Strict type validation helper class.
 *
 * See RTErrStrictType and RT_SUCCESS_NP.
 */
class RTErrStrictType2
{
protected:
    /** The status code.  */
    int32_t m_rc;

public:
    /**
     * Constructor.
     * @param   rc      IPRT style status code.
     */
    RTErrStrictType2(int32_t rc) : m_rc(rc)
    {
    }

    /**
     * Get the status code.
     * @returns IPRT style status code.
     */
    int32_t getValue() const
    {
        return m_rc;
    }
};
#endif /* __cplusplus */
/** @} */

#endif

