/* $Id: VBoxPrintGuid.c 28800 2010-04-27 08:22:32Z vboxsync $ */
/** @file
 * VBoxPrintGuid.c - Implementation of the VBoxPrintGuid() debug logging routine.
 */

/*
 * Copyright (C) 2009-2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */




/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include "VBoxDebugLib.h"
#include "DevEFI.h"


/**
 * Prints a EFI GUID.
 *
 * @returns Number of bytes printed.
 *
 * @param   pGuid           The GUID to print
 */
size_t VBoxPrintGuid(CONST EFI_GUID *pGuid)
{
    VBoxPrintHex(pGuid->Data1, sizeof(pGuid->Data1));
    VBoxPrintChar('-');
    VBoxPrintHex(pGuid->Data2, sizeof(pGuid->Data2));
    VBoxPrintChar('-');
    VBoxPrintHex(pGuid->Data3, sizeof(pGuid->Data3));
    VBoxPrintChar('-');
    VBoxPrintHex(pGuid->Data4[0], sizeof(pGuid->Data4[0]));
    VBoxPrintHex(pGuid->Data4[1], sizeof(pGuid->Data4[1]));
    VBoxPrintChar('-');
    VBoxPrintHex(pGuid->Data4[2], sizeof(pGuid->Data4[2]));
    VBoxPrintHex(pGuid->Data4[3], sizeof(pGuid->Data4[3]));
    VBoxPrintHex(pGuid->Data4[4], sizeof(pGuid->Data4[4]));
    VBoxPrintHex(pGuid->Data4[5], sizeof(pGuid->Data4[5]));
    VBoxPrintHex(pGuid->Data4[6], sizeof(pGuid->Data4[6]));
    VBoxPrintHex(pGuid->Data4[7], sizeof(pGuid->Data4[7]));
    return 37;
}

