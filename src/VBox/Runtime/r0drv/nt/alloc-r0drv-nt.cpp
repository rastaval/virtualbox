/* $Id: alloc-r0drv-nt.cpp 14069 2008-11-10 23:39:34Z vboxsync $ */
/** @file
 * IPRT - Memory Allocation, Ring-0 Driver, NT.
 */

/*
 * Copyright (C) 2006-2007 Sun Microsystems, Inc.
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
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 USA or visit http://www.sun.com if you need
 * additional information or have any questions.
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include "the-nt-kernel.h"

#include <iprt/alloc.h>
#include <iprt/assert.h>
#include "r0drv/alloc-r0drv.h"


/**
 * OS specific allocation function.
 */
PRTMEMHDR rtMemAlloc(size_t cb, uint32_t fFlags)
{
    PRTMEMHDR pHdr = (PRTMEMHDR)ExAllocatePoolWithTag(NonPagedPool, cb + sizeof(*pHdr), IPRT_NT_POOL_TAG);
    if (pHdr)
    {
        pHdr->u32Magic  = RTMEMHDR_MAGIC;
        pHdr->fFlags    = fFlags;
        pHdr->cb        = (uint32_t)cb; Assert(pHdr->cb == cb);
        pHdr->cbReq     = (uint32_t)cb;
    }
    return pHdr;
}


/**
 * OS specific free function.
 */
void rtMemFree(PRTMEMHDR pHdr)
{
    pHdr->u32Magic += 1;
    ExFreePool(pHdr);
}


/**
 * Allocates physical contiguous memory (below 4GB).
 * The allocation is page aligned and its contents is undefined.
 *
 * @returns Pointer to the memory block. This is page aligned.
 * @param   pPhys   Where to store the physical address.
 * @param   cb      The allocation size in bytes. This is always
 *                  rounded up to PAGE_SIZE.
 */
RTR0DECL(void *) RTMemContAlloc(PRTCCPHYS pPhys, size_t cb)
{
    /*
     * validate input.
     */
    Assert(VALID_PTR(pPhys));
    Assert(cb > 0);

    /*
     * Allocate and get physical address.
     * Make sure the return is page aligned.
     */
    PHYSICAL_ADDRESS MaxPhysAddr;
    MaxPhysAddr.HighPart = 0;
    MaxPhysAddr.LowPart = 0xffffffff;
    cb = RT_ALIGN_Z(cb, PAGE_SIZE);
    void *pv = MmAllocateContiguousMemory(cb, MaxPhysAddr);
    if (pv)
    {
        if (!((uintptr_t)pv & PAGE_OFFSET_MASK))    /* paranoia */
        {
            PHYSICAL_ADDRESS PhysAddr = MmGetPhysicalAddress(pv);
            if (!PhysAddr.HighPart)                 /* paranoia */
            {
                *pPhys = (RTCCPHYS)PhysAddr.LowPart;
                return pv;
            }

            /* failure */
            AssertMsgFailed(("MMAllocContiguousMemory returned high address! PhysAddr=%RX64\n", (uint64_t)PhysAddr.QuadPart));
        }
        else
            AssertMsgFailed(("MMAllocContiguousMemory didn't return a page aligned address - %p!\n", pv));

        MmFreeContiguousMemory(pv);
    }

    return NULL;
}


/**
 * Frees memory allocated ysing RTMemContAlloc().
 *
 * @param   pv      Pointer to return from RTMemContAlloc().
 * @param   cb      The cb parameter passed to RTMemContAlloc().
 */
RTR0DECL(void) RTMemContFree(void *pv, size_t cb)
{
    if (pv)
    {
        Assert(cb > 0); NOREF(cb);
        AssertMsg(!((uintptr_t)pv & PAGE_OFFSET_MASK), ("pv=%p\n", pv));
        MmFreeContiguousMemory(pv);
    }
}
