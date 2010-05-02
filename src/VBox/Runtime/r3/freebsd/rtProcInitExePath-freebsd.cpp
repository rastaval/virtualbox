/* $Id: rtProcInitExePath-freebsd.cpp 28800 2010-04-27 08:22:32Z vboxsync $ */
/** @file
 * IPRT - rtProcInitName, FreeBSD.
 */

/*
 * Copyright (C) 2006-2008 Oracle Corporation
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
#define LOG_GROUP RTLOGGROUP_PROCESS
#include <sys/param.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <link.h>

#include <iprt/string.h>
#include <iprt/assert.h>
#include <iprt/err.h>
#include <iprt/path.h>
#include "internal/process.h"
#include "internal/path.h"


DECLHIDDEN(int) rtProcInitExePath(char *pszPath, size_t cchPath)
{
#ifdef KERN_PROC_PATHNAME
    int aiName[4];
    aiName[0] = CTL_KERN;
    aiName[1] = KERN_PROC;
    aiName[2] = KERN_PROC_PATHNAME;     /* This was introduced in FreeBSD 6.0, thus the #ifdef above. */
    aiName[3] = -1;                     /* Shorthand for the current process. */

    size_t cchExePath = cchPath;
    if (sysctl(aiName, RT_ELEMENTS(aiName), pszPath, &cchExePath, NULL, 0) == 0)
    {

        char *pszTmp = NULL;
        int rc = rtPathFromNative(&pszTmp, pszPath);
        AssertMsgRCReturn(rc, ("rc=%Rrc pszPath=\"%s\"\nhex: %.*Rhsx\n", rc, pszPath, cchExePath, pszPath), rc);

        size_t cch = strlen(pszTmp);
        AssertReturn(cch <= cchPath, VERR_BUFFER_OVERFLOW);

        memcpy(pszPath, pszTmp, cch + 1);
        RTStrFree(pszTmp);

        return VINF_SUCCESS;
    }

    int rc = RTErrConvertFromErrno(errno);
    AssertMsgFailed(("rc=%Rrc errno=%d cchExePath=%d\n", rc, errno, cchExePath));
    return rc;

#else

    /*
     * Read the /proc/curproc/file link, convert to native and return it.
     */
    int cchLink = readlink("/proc/curproc/file", pszPath, cchPath - 1);
    if (cchLink > 0 && (size_t)cchLink <= cchPath - 1)
    {
        pszPath[cchLink] = '\0';

        char *pszTmp = NULL;
        int rc = rtPathFromNative(&pszTmp, pszPath);
        AssertMsgRCReturn(rc, ("rc=%Rrc pszLink=\"%s\"\nhex: %.*Rhsx\n", rc, pszPath, cchLink, pszPath), rc);

        size_t cch = strlen(pszTmp);
        AssertReturn(cch <= cchPath, VERR_BUFFER_OVERFLOW);

        memcpy(pszPath, pszTmp, cch + 1);
        RTStrFree(pszTmp);

        return VINF_SUCCESS;
    }

    int err = errno;

    /*
     * Fall back on the dynamic linker since /proc is optional.
     */
    void *hExe = dlopen(NULL, 0);
    if (hExe)
    {
        struct link_map const *pLinkMap = 0;
        if (dlinfo(hExe, RTLD_DI_LINKMAP, &pLinkMap) == 0)
        {
            const char *pszImageName = pLinkMap->l_name;
            if (*pszImageName == '/') /* this may not always be absolute, despite the docs. :-( */
            {
                char *pszTmp = NULL;
                int rc = rtPathFromNative(&pszTmp, pszImageName);
                AssertMsgRCReturn(rc, ("rc=%Rrc pszImageName=\"%s\"\n", rc, pszImageName), rc);

                size_t cch = strlen(pszTmp);
                AssertReturn(cch <= cchPath, VERR_BUFFER_OVERFLOW);

                memcpy(pszPath, pszTmp, cch + 1);
                RTStrFree(pszTmp);

                return VINF_SUCCESS;
            }
            /** @todo Try search the PATH for the file name or append the current
             *        directory, which ever makes sense... */
        }
    }

    int rc = RTErrConvertFromErrno(err);
    AssertMsgFailed(("rc=%Rrc err=%d cchLink=%d hExe=%p\n", rc, err, cchLink, hExe));
    return rc;
#endif
}

