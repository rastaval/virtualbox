/* $Id: dir.cpp 28918 2010-04-29 18:30:09Z vboxsync $ */
/** @file
 * IPRT - Directory Manipulation.
 */

/*
 * Copyright (C) 2006-2010 Oracle Corporation
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
#define LOG_GROUP RTLOGGROUP_DIR
#ifdef RT_OS_WINDOWS /* PORTME: Assumes everyone else is using dir-posix.cpp */
# include <Windows.h>
#else
# include <dirent.h>
# include <unistd.h>
# include <limits.h>
#endif

#include <iprt/dir.h>
#include "internal/iprt.h"

#include <iprt/assert.h>
#include <iprt/file.h>
#include <iprt/err.h>
#include <iprt/log.h>
#include <iprt/mem.h>
#include <iprt/param.h>
#include <iprt/path.h>
#include <iprt/string.h>
#include <iprt/uni.h>
#include "internal/dir.h"
#include "internal/path.h"


static DECLCALLBACK(bool) rtDirFilterWinNtMatch(PRTDIR pDir, const char *pszName);
static DECLCALLBACK(bool) rtDirFilterWinNtMatchNoWildcards(PRTDIR pDir, const char *pszName);
DECLINLINE(bool) rtDirFilterWinNtMatchEon(PCRTUNICP puszFilter);
static bool rtDirFilterWinNtMatchDosStar(unsigned iDepth, RTUNICP uc, const char *pszNext, PCRTUNICP puszFilter);
static bool rtDirFilterWinNtMatchStar(unsigned iDepth, RTUNICP uc, const char *pszNext, PCRTUNICP puszFilter);
static bool rtDirFilterWinNtMatchBase(unsigned iDepth, const char *pszName, PCRTUNICP puszFilter);



RTDECL(int) RTDirCreateFullPath(const char *pszPath, RTFMODE fMode)
{
    /*
     * Resolve the path.
     */
    char szAbsPath[RTPATH_MAX];
    int rc = RTPathAbs(pszPath, szAbsPath, sizeof(szAbsPath));
    if (RT_FAILURE(rc))
        return rc;

    /*
     * Iterate the path components making sure each of them exists.
     */
    /* skip volume name */
    char *psz = &szAbsPath[rtPathVolumeSpecLen(szAbsPath)];

    /* skip the root slash if any */
    if (    psz[0] == '/'
#if defined(RT_OS_WINDOWS) || defined(RT_OS_OS2)
        ||  psz[0] == '\\'
#endif
        )
        psz++;

    /* iterate over path components. */
    do
    {
        /* the next component is NULL, stop iterating */
        if (!*psz)
            break;
#if defined(RT_OS_WINDOWS) || defined(RT_OS_OS2)
        psz = strpbrk(psz, "\\/");
#else
        psz = strchr(psz, '/');
#endif
        if (psz)
            *psz = '\0';
        /*
         * ASSUME that RTDirCreate will return VERR_ALREADY_EXISTS and not VERR_ACCESS_DENIED in those cases
         * where the directory exists but we don't have write access to the parent directory.
         */
        rc = RTDirCreate(szAbsPath, fMode);
        if (rc == VERR_ALREADY_EXISTS)
            rc = VINF_SUCCESS;
        if (!psz)
            break;
        *psz++ = RTPATH_DELIMITER;
    } while (RT_SUCCESS(rc));

    return rc;
}


/**
 * Filter a the filename in the against a filter.
 *
 * @returns true if the name matches the filter.
 * @returns false if the name doesn't match filter.
 * @param   pDir        The directory handle.
 * @param   pszName     The path to match to the filter.
 */
static DECLCALLBACK(bool) rtDirFilterWinNtMatchNoWildcards(PRTDIR pDir, const char *pszName)
{
    /*
     * Walk the string and compare.
     */
    PCRTUNICP   pucFilter = pDir->puszFilter;
    const char *psz = pszName;
    RTUNICP     uc;
    do
    {
        int rc = RTStrGetCpEx(&psz, &uc);
        AssertRCReturn(rc, false);
        RTUNICP ucFilter = *pucFilter++;
        if (    uc != ucFilter
            &&  RTUniCpToUpper(uc) != ucFilter)
            return false;
    } while (uc);
    return true;
}


/**
 * Matches end of name.
 */
DECLINLINE(bool) rtDirFilterWinNtMatchEon(PCRTUNICP puszFilter)
{
    RTUNICP ucFilter;
    while (     (ucFilter = *puszFilter) == '>'
           ||   ucFilter == '<'
           ||   ucFilter == '*'
           ||   ucFilter == '"')
        puszFilter++;
    return !ucFilter;
}


/**
 * Recursive star matching.
 * Practically the same as normal star, except that the dos star stops
 * when hitting the last dot.
 *
 * @returns true on match.
 * @returns false on miss.
 */
static bool rtDirFilterWinNtMatchDosStar(unsigned iDepth, RTUNICP uc, const char *pszNext, PCRTUNICP puszFilter)
{
    AssertReturn(iDepth++ < 256, false);

    /*
     * If there is no dos star, we should work just like the NT star.
     * Since that's generally faster algorithms, we jump down to there if we can.
     */
    const char *pszDosDot = strrchr(pszNext, '.');
    if (!pszDosDot && uc == '.')
        pszDosDot = pszNext - 1;
    if (!pszDosDot)
        return rtDirFilterWinNtMatchStar(iDepth, uc, pszNext, puszFilter);

    /*
     * Inspect the next filter char(s) until we find something to work on.
     */
    RTUNICP ucFilter = *puszFilter++;
    switch (ucFilter)
    {
        /*
         * The star expression is the last in the pattern.
         * We're fine if the name ends with a dot.
         */
        case '\0':
            return !pszDosDot[1];

        /*
         * Simplified by brute force.
         */
        case '>': /* dos question mark */
        case '?':
        case '*':
        case '<': /* dos star */
        case '"': /* dos dot */
        {
            puszFilter--;
            const char *pszStart = pszNext;
            do
            {
                if (rtDirFilterWinNtMatchBase(iDepth, pszNext, puszFilter))
                    return true;
                int rc = RTStrGetCpEx(&pszNext, &uc); AssertRCReturn(rc, false);
            } while ((intptr_t)pszDosDot - (intptr_t)pszNext >= -1);

            /* backtrack and do the current char. */
            pszNext = RTStrPrevCp(NULL, pszStart); AssertReturn(pszNext, false);
            return rtDirFilterWinNtMatchBase(iDepth, pszNext, puszFilter);
        }

        /*
         * Ok, we've got zero or more characters.
         * We'll try match starting at each occurence of this character.
         */
        default:
        {
            if (    RTUniCpToUpper(uc) == ucFilter
                &&  rtDirFilterWinNtMatchBase(iDepth, pszNext, puszFilter))
                return true;
            do
            {
                int rc = RTStrGetCpEx(&pszNext, &uc); AssertRCReturn(rc, false);
                if (    RTUniCpToUpper(uc) == ucFilter
                    &&  rtDirFilterWinNtMatchBase(iDepth, pszNext, puszFilter))
                    return true;
            } while ((intptr_t)pszDosDot - (intptr_t)pszNext >= -1);
            return false;
        }
    }
    /* won't ever get here! */
}


/**
 * Recursive star matching.
 *
 * @returns true on match.
 * @returns false on miss.
 */
static bool rtDirFilterWinNtMatchStar(unsigned iDepth, RTUNICP uc, const char *pszNext, PCRTUNICP puszFilter)
{
    AssertReturn(iDepth++ < 256, false);

    /*
     * Inspect the next filter char(s) until we find something to work on.
     */
    for (;;)
    {
        RTUNICP ucFilter = *puszFilter++;
        switch (ucFilter)
        {
            /*
             * The star expression is the last in the pattern.
             * Cool, that means we're done!
             */
            case '\0':
                return true;

            /*
             * Just in case (doubt we ever get here), just merge it with the current one.
             */
            case '*':
                break;

            /*
             * Skip a fixed number of chars.
             * Figure out how many by walking the filter ignoring '*'s.
             */
            case '?':
            {
                unsigned cQms = 1;
                while ((ucFilter = *puszFilter) == '*' || ucFilter == '?')
                {
                    cQms += ucFilter == '?';
                    puszFilter++;
                }
                do
                {
                    if (!uc)
                        return false;
                    int rc = RTStrGetCpEx(&pszNext, &uc); AssertRCReturn(rc, false);
                } while (--cQms > 0);
                /* done? */
                if (!ucFilter)
                    return true;
                break;
            }

            /*
             * The simple way is to try char by char and match the remaining
             * expression. If it's trailing we're done.
             */
            case '>': /* dos question mark */
            {
                if (rtDirFilterWinNtMatchEon(puszFilter))
                    return true;
                const char *pszStart = pszNext;
                do
                {
                    if (rtDirFilterWinNtMatchBase(iDepth, pszNext, puszFilter))
                        return true;
                    int rc = RTStrGetCpEx(&pszNext, &uc); AssertRCReturn(rc, false);
                } while (uc);

                /* backtrack and do the current char. */
                pszNext = RTStrPrevCp(NULL, pszStart); AssertReturn(pszNext, false);
                return rtDirFilterWinNtMatchBase(iDepth, pszNext, puszFilter);
            }

            /*
             * This bugger is interesting.
             * Time for brute force. Iterate the name char by char.
             */
            case '<':
            {
                do
                {
                    if (rtDirFilterWinNtMatchDosStar(iDepth, uc, pszNext, puszFilter))
                        return true;
                    int rc = RTStrGetCpEx(&pszNext, &uc); AssertRCReturn(rc, false);
                } while (uc);
                return false;
            }

            /*
             * This guy matches a '.' or the end of the name.
             * It's very simple if the rest of the filter expression also matches eon.
             */
            case '"':
                if (rtDirFilterWinNtMatchEon(puszFilter))
                    return true;
                ucFilter = '.';
                /* fall thru */

            /*
             * Ok, we've got zero or more characters.
             * We'll try match starting at each occurence of this character.
             */
            default:
            {
                do
                {
                    if (    RTUniCpToUpper(uc) == ucFilter
                        &&  rtDirFilterWinNtMatchBase(iDepth, pszNext, puszFilter))
                        return true;
                    int rc = RTStrGetCpEx(&pszNext, &uc); AssertRCReturn(rc, false);
                } while (uc);
                return false;
            }
        }
    } /* for (;;) */

    /* won't ever get here! */
}


/**
 * Filter a the filename in the against a filter.
 *
 * The rules are as follows:
 *      '?'     Matches exactly one char.
 *      '*'     Matches zero or more chars.
 *      '<'     The dos star, matches zero or more chars except the DOS dot.
 *      '>'     The dos question mark, matches one char, but dots and end-of-name eats them.
 *      '"'     The dos dot, matches a dot or end-of-name.
 *
 * @returns true if the name matches the filter.
 * @returns false if the name doesn't match filter.
 * @param   iDepth      The recursion depth.
 * @param   pszName     The path to match to the filter.
 * @param   puszFilter  The filter string.
 */
static bool rtDirFilterWinNtMatchBase(unsigned iDepth, const char *pszName, PCRTUNICP puszFilter)
{
    AssertReturn(iDepth++ < 256, false);

    /*
     * Walk the string and match it up char by char.
     */
    RTUNICP uc;
    do
    {
        RTUNICP ucFilter = *puszFilter++;
        int rc = RTStrGetCpEx(&pszName, &uc); AssertRCReturn(rc, false);
        switch (ucFilter)
        {
            /* Exactly one char. */
            case '?':
                if (!uc)
                    return false;
                break;

            /* One char, but the dos dot and end-of-name eats '>' and '<'. */
            case '>': /* dos ? */
                if (!uc)
                    return rtDirFilterWinNtMatchEon(puszFilter);
                if (uc == '.')
                {
                    while ((ucFilter = *puszFilter) == '>' || ucFilter == '<')
                        puszFilter++;
                    if (ucFilter == '"' || ucFilter == '.')  /* not 100% sure about the last dot */
                        ++puszFilter;
                    else /* the does question mark doesn't match '.'s, so backtrack. */
                        pszName = RTStrPrevCp(NULL, pszName);
                }
                break;

            /* Match a dot or the end-of-name. */
            case '"': /* dos '.' */
                if (uc != '.')
                {
                    if (uc)
                        return false;
                    return rtDirFilterWinNtMatchEon(puszFilter);
                }
                break;

            /* zero or more */
            case '*':
                return rtDirFilterWinNtMatchStar(iDepth, uc, pszName, puszFilter);
            case '<': /* dos '*' */
                return rtDirFilterWinNtMatchDosStar(iDepth, uc, pszName, puszFilter);


            /* uppercased match */
            default:
            {
                if (RTUniCpToUpper(uc) != ucFilter)
                    return false;
                break;
            }
        }
    } while (uc);

    return true;
}


/**
 * Filter a the filename in the against a filter.
 *
 * @returns true if the name matches the filter.
 * @returns false if the name doesn't match filter.
 * @param   pDir        The directory handle.
 * @param   pszName     The path to match to the filter.
 */
static DECLCALLBACK(bool) rtDirFilterWinNtMatch(PRTDIR pDir, const char *pszName)
{
    return rtDirFilterWinNtMatchBase(0, pszName, pDir->puszFilter);
}


/**
 * Initializes a WinNt like wildcard filter.
 *
 * @returns Pointer to the filter function.
 * @returns NULL if the filter doesn't filter out anything.
 * @param   pDir        The directory handle (not yet opened).
 */
static PFNRTDIRFILTER rtDirFilterWinNtInit(PRTDIR pDir)
{
    /*
     * Check for the usual * and <"< (*.* in DOS language) patterns.
     */
    if (    (pDir->cchFilter == 1 && pDir->pszFilter[0] == '*')
        ||  (pDir->cchFilter == 3 && !memcmp(pDir->pszFilter, "<\".>", 3))
        )
        return NULL;

    /*
     * Uppercase the expression, also do a little optimizations when possible.
     */
    bool fHaveWildcards = false;
    unsigned iRead = 0;
    unsigned iWrite = 0;
    while (iRead < pDir->cucFilter)
    {
        RTUNICP uc = pDir->puszFilter[iRead++];
        if (uc == '*')
        {
            fHaveWildcards = true;
            /* remove extra stars. */
            RTUNICP uc2;
            while ((uc2 = pDir->puszFilter[iRead + 1]) == '*')
                iRead++;
        }
        else if (uc == '?' || uc == '>' || uc == '<' || uc == '"')
            fHaveWildcards = true;
        else
            uc = RTUniCpToUpper(uc);
        pDir->puszFilter[iWrite++] = uc;
    }
    pDir->puszFilter[iWrite] = 0;
    pDir->cucFilter = iWrite;

    return fHaveWildcards
        ? rtDirFilterWinNtMatch
        : rtDirFilterWinNtMatchNoWildcards;
}


/**
 * Common worker for opening a directory.
 *
 * @returns IPRT status code.
 * @param   ppDir       Where to store the directory handle.
 * @param   pszPath     The specified path.
 * @param   pszFilter   Pointer to where the filter start in the path. NULL if no filter.
 * @param   enmFilter   The type of filter to apply.
 */
static int rtDirOpenCommon(PRTDIR *ppDir, const char *pszPath, const char *pszFilter, RTDIRFILTER enmFilter)
{
    /*
     * Expand the path.
     *
     * The purpose of this exercise to have the abs path around
     * for querying extra information about the objects we list.
     * As a sideeffect we also validate the path here.
     */
    char szRealPath[RTPATH_MAX + 1];
    int rc;
    size_t cbFilter;                    /* includes '\0' (thus cb and not cch). */
    size_t cucFilter0;                  /* includes U+0. */
    if (!pszFilter)
    {
        cbFilter = cucFilter0 = 0;
        rc = RTPathReal(pszPath, szRealPath, sizeof(szRealPath) - 1);
    }
    else
    {
        cbFilter = strlen(pszFilter) + 1;
        cucFilter0 = RTStrUniLen(pszFilter) + 1;

        if (pszFilter != pszPath)
        {
            /* yea, I'm lazy. sue me. */
            char *pszTmp = RTStrDup(pszPath);
            if (!pszTmp)
                return VERR_NO_MEMORY;
            pszTmp[pszFilter - pszPath] = '\0';
            rc = RTPathReal(pszTmp, szRealPath, sizeof(szRealPath) - 1);
            RTStrFree(pszTmp);
        }
        else
            rc = RTPathReal(".", szRealPath, sizeof(szRealPath) - 1);
    }
    if (RT_FAILURE(rc))
        return rc;

    /* add trailing '/' if missing. */
    size_t cchRealPath = strlen(szRealPath);
    if (!RTPATH_IS_SEP(szRealPath[cchRealPath - 1]))
    {
        szRealPath[cchRealPath++] = RTPATH_SLASH;
        szRealPath[cchRealPath] = '\0';
    }

    /*
     * Allocate and initialize the directory handle.
     *
     * The posix definition of Data.d_name allows it to be < NAME_MAX + 1,
     * thus the horrible uglyness here. Solaris uses d_name[1] for instance.
     */
#ifndef RT_OS_WINDOWS
    long cbNameMax = pathconf(szRealPath, _PC_NAME_MAX);
# ifdef NAME_MAX
    if (cbNameMax < NAME_MAX)           /* This is plain paranoia, but it doesn't hurt. */
        cbNameMax = NAME_MAX;
# endif
# ifdef _XOPEN_NAME_MAX
    if (cbNameMax < _XOPEN_NAME_MAX)    /* Ditto. */
        cbNameMax = _XOPEN_NAME_MAX;
# endif
    size_t cbDir = RT_OFFSETOF(RTDIR, Data.d_name[cbNameMax + 1]);
    if (cbDir < sizeof(RTDIR))          /* Ditto. */
        cbDir = sizeof(RTDIR);
    cbDir = RT_ALIGN_Z(cbDir, 8);
#else
    size_t cbDir = sizeof(RTDIR);
#endif
    size_t const cbAllocated = cbDir
                             + cucFilter0 * sizeof(RTUNICP)
                             + cbFilter
                             + cchRealPath + 1 + 4;
    PRTDIR pDir = (PRTDIR)RTMemAlloc(cbAllocated);
    if (!pDir)
        return VERR_NO_MEMORY;
    uint8_t *pb = (uint8_t *)pDir + cbDir;

    /* initialize it */
    pDir->u32Magic = RTDIR_MAGIC;
    if (cbFilter)
    {
        pDir->puszFilter = (PRTUNICP)pb;
        rc = RTStrToUniEx(pszFilter, RTSTR_MAX, &pDir->puszFilter, cucFilter0, &pDir->cucFilter);
        AssertRC(rc);
        pb += cucFilter0 * sizeof(RTUNICP);
        pDir->pszFilter = (char *)memcpy(pb, pszFilter, cbFilter);
        pDir->cchFilter = cbFilter - 1;
        pb += cbFilter;
    }
    else
    {
        pDir->puszFilter = NULL;
        pDir->cucFilter = 0;
        pDir->pszFilter = NULL;
        pDir->cchFilter = 0;
    }
    pDir->enmFilter = enmFilter;
    switch (enmFilter)
    {
        default:
        case RTDIRFILTER_NONE:
            pDir->pfnFilter = NULL;
            break;
        case RTDIRFILTER_WINNT:
            pDir->pfnFilter = rtDirFilterWinNtInit(pDir);
            break;
        case RTDIRFILTER_UNIX:
            pDir->pfnFilter = NULL;
            break;
        case RTDIRFILTER_UNIX_UPCASED:
            pDir->pfnFilter = NULL;
            break;
    }
    pDir->cchPath = cchRealPath;
    pDir->pszPath = (char *)memcpy(pb, szRealPath, cchRealPath + 1);
    Assert(pb - (uint8_t *)pDir + cchRealPath + 1 <= cbAllocated);
    pDir->fDataUnread = false;
    pDir->pszName = NULL;
    pDir->cchName = 0;
#ifndef RT_OS_WINDOWS
    pDir->cbMaxName = cbDir - RT_OFFSETOF(RTDIR, Data.d_name);
#endif

    /*
     * Hand it over to the native part.
     */
    rc = rtDirNativeOpen(pDir, szRealPath);
    if (RT_SUCCESS(rc))
        *ppDir = pDir;
    else
        RTMemFree(pDir);

    return rc;
}



RTDECL(int) RTDirOpen(PRTDIR *ppDir, const char *pszPath)
{
    /*
     * Validate input.
     */
    AssertMsgReturn(VALID_PTR(ppDir), ("%p\n", ppDir), VERR_INVALID_POINTER);
    AssertMsgReturn(VALID_PTR(pszPath), ("%p\n", pszPath), VERR_INVALID_POINTER);

    /*
     * Take common cause with RTDirOpenFiltered().
     */
    int rc = rtDirOpenCommon(ppDir, pszPath, NULL,  RTDIRFILTER_NONE);
    LogFlow(("RTDirOpen(%p:{%p}, %p:{%s}): return %Rrc\n", ppDir, *ppDir, pszPath, pszPath, rc));
    return rc;
}


RTDECL(int) RTDirOpenFiltered(PRTDIR *ppDir, const char *pszPath, RTDIRFILTER enmFilter)
{
    /*
     * Validate input.
     */
    AssertMsgReturn(VALID_PTR(ppDir), ("%p\n", ppDir), VERR_INVALID_POINTER);
    AssertMsgReturn(VALID_PTR(pszPath), ("%p\n", pszPath), VERR_INVALID_POINTER);
    switch (enmFilter)
    {
        case RTDIRFILTER_UNIX:
        case RTDIRFILTER_UNIX_UPCASED:
            AssertMsgFailed(("%d is not implemented!\n", enmFilter));
            return VERR_NOT_IMPLEMENTED;
        case RTDIRFILTER_NONE:
        case RTDIRFILTER_WINNT:
            break;
        default:
            AssertMsgFailedReturn(("%d\n", enmFilter), VERR_INVALID_PARAMETER);
    }

    /*
     * Find the last component, i.e. where the filter criteria starts and the dir name ends.
     */
    const char *pszFilter;
    if (enmFilter == RTDIRFILTER_NONE)
        pszFilter = NULL;
    else
    {
        pszFilter = RTPathFilename(pszPath);
        if (!pszFilter) /* trailing slash => directory to read => no filter. */
            enmFilter = RTDIRFILTER_NONE;
    }

    /*
     * Call worker common with RTDirOpen which will verify the path, allocate
     * and initialize the handle, and finally call the backend.
     */
    int rc = rtDirOpenCommon(ppDir, pszPath, pszFilter, enmFilter);

    LogFlow(("RTDirOpenFiltered(%p:{%p}, %p:{%s}, %d): return %Rrc\n",
             ppDir, *ppDir, pszPath, pszPath, enmFilter, rc));
    return rc;
}


/**
 * Recursion worker for RTDirRemoveRecursive.
 *
 * @returns IPRT status code.
 * @param   pszBuf              The path buffer.  Contains the abs path to the
 *                              directory to recurse into.  Trailing slash.
 * @param   cchDir              The length of the directory we're cursing into,
 *                              including the trailing slash.
 * @param   pDirEntry           The dir entry buffer.  (Shared to save stack.)
 * @param   pObjInfo            The object info buffer.  (ditto)
 */
static int rtDirRemoveRecursiveSub(char *pszBuf, size_t cchDir, PRTDIRENTRY pDirEntry, PRTFSOBJINFO pObjInfo)
{
    AssertReturn(RTPATH_IS_SLASH(pszBuf[cchDir - 1]), VERR_INTERNAL_ERROR_4);

    /*
     * Enumerate the directory content and dispose of it.
     */
    PRTDIR pDir;
    int rc = RTDirOpen(&pDir, pszBuf);
    if (RT_FAILURE(rc))
        return rc;
    while (RT_SUCCESS(rc = RTDirRead(pDir, pDirEntry, NULL)))
    {
        if (   pDirEntry->szName[0] != '.'
            || pDirEntry->cbName > 2
            || (   pDirEntry->cbName == 2
                && pDirEntry->szName[1] != '.')
           )
        {
            /* Construct the full name of the entry. */
            if (cchDir + pDirEntry->cbName + 1 /* dir slash */ >= RTPATH_MAX)
            {
                rc = VERR_FILENAME_TOO_LONG;
                break;
            }
            memcpy(&pszBuf[cchDir], pDirEntry->szName, pDirEntry->cbName + 1);

            /* Deal with the unknown type. */
            if (pDirEntry->enmType == RTDIRENTRYTYPE_UNKNOWN)
            {
                rc = RTPathQueryInfoEx(pszBuf, pObjInfo, RTFSOBJATTRADD_NOTHING, RTPATH_F_ON_LINK);
                if (RT_SUCCESS(rc) && RTFS_IS_DIRECTORY(pObjInfo->Attr.fMode))
                    pDirEntry->enmType = RTDIRENTRYTYPE_DIRECTORY;
                else if (RT_SUCCESS(rc) && RTFS_IS_FILE(pObjInfo->Attr.fMode))
                    pDirEntry->enmType = RTDIRENTRYTYPE_FILE;
                else if (RT_SUCCESS(rc) && RTFS_IS_SYMLINK(pObjInfo->Attr.fMode))
                    pDirEntry->enmType = RTDIRENTRYTYPE_SYMLINK;
            }

            /* Try the delete the fs object. */
            switch (pDirEntry->enmType)
            {
                case RTDIRENTRYTYPE_FILE:
                    rc = RTFileDelete(pszBuf);
                    break;

                case RTDIRENTRYTYPE_DIRECTORY:
                {
                    size_t cchSubDir = cchDir + pDirEntry->cbName;
                    pszBuf[cchSubDir++] = '/';
                    pszBuf[cchSubDir]   = '\0';
                    rc = rtDirRemoveRecursiveSub(pszBuf, cchSubDir, pDirEntry, pObjInfo);
                    if (RT_SUCCESS(rc))
                    {
                        pszBuf[cchSubDir] = '\0';
                        rc = RTDirRemove(pszBuf);
                    }
                    break;
                }

                //case RTDIRENTRYTYPE_SYMLINK:
                //    rc = RTSymlinkDelete(pszBuf);
                //    break;

                default:
                    /** @todo not implemented yet. */
                    rc = VINF_SUCCESS;
                    break;
            }
            if (RT_FAILURE(rc))
               break;
        }
    }
    if (rc == VERR_NO_MORE_FILES)
        rc = VINF_SUCCESS;
    RTDirClose(pDir);
    return rc;
}


RTDECL(int) RTDirRemoveRecursive(const char *pszPath, uint32_t fFlags)
{
    AssertReturn(!(fFlags & ~RTDIRRMREC_F_VALID_MASK), VERR_INVALID_PARAMETER);

    /* Get an absolute path because this is easier to work with. */
    /** @todo use RTPathReal here instead? */
    char szAbsPath[RTPATH_MAX];
    int rc = RTPathAbs(pszPath, szAbsPath, sizeof(szAbsPath));
    if (RT_FAILURE(rc))
        return rc;

    /* This API is not permitted applied to the root of anything. */
    if (RTPathCountComponents(szAbsPath) <= 1)
        return VERR_ACCESS_DENIED;

    /* Because of the above restriction, we never have to deal with the root
       slash problem and can safely strip any trailing slashes and add a
       definite one. */
    RTPathStripTrailingSlash(szAbsPath);
    size_t cchAbsPath = strlen(szAbsPath);
    if (cchAbsPath + 1 >= RTPATH_MAX)
        return VERR_FILENAME_TOO_LONG;
    szAbsPath[cchAbsPath++] = '/';
    szAbsPath[cchAbsPath] = 0;

    /* Check if it exists so we can return quietly if it doesn't. */
    RTFSOBJINFO SharedObjInfoBuf;
    rc = RTPathQueryInfoEx(szAbsPath, &SharedObjInfoBuf, RTFSOBJATTRADD_NOTHING, RTPATH_F_ON_LINK);
    if (   rc == VERR_PATH_NOT_FOUND
        || rc == VERR_FILE_NOT_FOUND)
        return VINF_SUCCESS;
    if (RT_FAILURE(rc))
        return rc;
    if (!RTFS_IS_DIRECTORY(SharedObjInfoBuf.Attr.fMode))
        return VERR_NOT_A_DIRECTORY;

    /* We're all set for the recursion now, so get going. */
    RTDIRENTRY  SharedDirEntryBuf;
    rc = rtDirRemoveRecursiveSub(szAbsPath, cchAbsPath, &SharedDirEntryBuf, &SharedObjInfoBuf);

    /* Remove the specified directory if desired and removing the content was
       successful. */
    if (   RT_SUCCESS(rc)
        && !(fFlags & RTDIRRMREC_F_CONTENT_ONLY))
    {
        szAbsPath[cchAbsPath] = 0;
        rc = RTDirRemove(szAbsPath);
    }
    return rc;
}


RTDECL(int) RTDirFlushParent(const char *pszChild)
{
    char szPath[RTPATH_MAX];
    int rc = RTStrCopy(szPath, sizeof(szPath), pszChild);
    if (RT_SUCCESS(rc))
    {
        RTPathStripFilename(szPath);
        rc = RTDirFlush(szPath);
    }
    return rc;
}

