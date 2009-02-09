/* $Id: kHlpString-iprt.cpp 8155 2008-04-18 15:16:47Z vboxsync $ */
/** @file
 * kHlpString - String And Memory Routines, IPRT based implementation.
 */

/*
 * Copyright (C) 2007 Sun Microsystems, Inc.
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 USA or visit http://www.sun.com if you need
 * additional information or have any questions.
 */

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <k/kHlpString.h>
#include <iprt/string.h>


#ifndef kHlpMemChr
void   *kHlpMemChr(const void *pv, int ch, KSIZE cb)
{
    return (void *)memchr(pv, ch, cb);
}
#endif


#ifndef kHlpMemComp
int     kHlpMemComp(const void *pv1, const void *pv2, KSIZE cb)
{
    return memcmp(pv1, pv2, cb);
}
#endif


#ifndef kHlpMemCopy
void   *kHlpMemCopy(void *pv1, const void *pv2, KSIZE cb)
{
    return memcpy(pv1, pv2, cb);
}
#endif


#ifndef kHlpMemPCopy
void   *kHlpMemPCopy(void *pv1, const void *pv2, KSIZE cb)
{
    return (KU8 *)memcpy(pv1, pv2, cb) + cb;
}
#endif


#ifndef kHlpMemMove
void   *kHlpMemMove(void *pv1, const void *pv2, KSIZE cb)
{
    return memmove(pv1, pv2, cb);
}
#endif


#ifndef kHlpMemPMove
void   *kHlpMemPMove(void *pv1, const void *pv2, KSIZE cb)
{
    return (KU8 *)memmove(pv1, pv2, cb) + cb;
}
#endif


#ifndef kHlpMemSet
void   *kHlpMemSet(void *pv1, int ch, KSIZE cb)
{
    return memset(pv1, ch, cb);
}
#endif


#ifndef kHlpMemPSet
void   *kHlpMemPSet(void *pv1, int ch, KSIZE cb)
{
    return (KU8 *)memset(pv1, ch, cb) + cb;
}
#endif


#ifndef kHlpStrCat
char   *kHlpStrCat(char *psz1, const char *psz2)
{
    return strcat(psz1, psz2);
}
#endif


#ifndef kHlpStrNCat
char   *kHlpStrNCat(char *psz1, const char *psz2, KSIZE cb)
{
    return strncat(psz1, psz2, cb);
}
#endif


#ifndef kHlpStrChr
char   *kHlpStrChr(const char *psz, int ch)
{
    return (char *)strchr(psz, ch);
}
#endif


#ifndef kHlpStrRChr
char   *kHlpStrRChr(const char *psz, int ch)
{
    return (char *)strrchr(psz, ch);
}
#endif


#ifndef kHlpStrComp
int     kHlpStrComp(const char *psz1, const char *psz2)
{
    return strcmp(psz1, psz2);
}
#endif


#ifndef kHlpStrNComp
int     kHlpStrNComp(const char *psz1, const char *psz2, KSIZE cch)
{
    return strncmp(psz1, psz2, cch);
}
#endif


#ifndef kHlpStrCopy
char   *kHlpStrCopy(char *psz1, const char *psz2)
{
    return strcpy(psz1, psz2);
}
#endif


#ifndef kHlpStrLen
KSIZE   kHlpStrLen(const char *psz1)
{
    return strlen(psz1);
}
#endif

