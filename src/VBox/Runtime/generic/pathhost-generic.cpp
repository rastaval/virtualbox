/* $Id: pathhost-generic.cpp 28800 2010-04-27 08:22:32Z vboxsync $ */
/** @file
 * IPRT - Path Convertions, generic.
 */

/*
 * Copyright (C) 2006-2007 Oracle Corporation
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


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#define LOG_GROUP RTLOGGROUP_PATH
#include <iprt/string.h>
#include "internal/iprt.h"

#include "internal/path.h"


int rtPathToNative(char **ppszNativePath, const char *pszPath)
{
    return RTStrUtf8ToCurrentCP(ppszNativePath, pszPath);
}

int rtPathToNativeEx(char **ppszNativePath, const char *pszPath, const char *pszBasePath)
{
    NOREF(pszBasePath);
    return RTStrUtf8ToCurrentCP(ppszNativePath, pszPath);
}

void rtPathFreeNative(char *pszNativePath)
{
    if (pszNativePath)
        RTStrFree(pszNativePath);
}


int rtPathFromNative(char **pszPath, const char *pszNativePath)
{
    return RTStrCurrentCPToUtf8(pszPath, pszNativePath);
}


int rtPathFromNativeEx(char **pszPath, const char *pszNativePath, const char *pszBasePath)
{
    NOREF(pszBasePath);
    return RTStrCurrentCPToUtf8(pszPath, pszNativePath);
}

