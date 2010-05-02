/* $Id: RTStrStr.cpp 28800 2010-04-27 08:22:32Z vboxsync $ */
/** @file
 * IPRT - RTStrStr.
 */

/*
 * Copyright (C) 2006-2009 Oracle Corporation
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
#include <iprt/string.h>
#include "internal/iprt.h"


RTDECL(char *) RTStrStr(const char *pszHaystack, const char *pszNeedle)
{
    /* Any NULL strings means NULL return. (In the RTStrCmp tradition.) */
    if (!pszHaystack)
        return NULL;
    if (!pszNeedle)
        return NULL;

    /* The rest is CRT. */
    return (char *)strstr(pszHaystack, pszNeedle);
}
RT_EXPORT_SYMBOL(RTStrStr);

