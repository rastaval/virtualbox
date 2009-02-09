/* $Id: Pcap.h 10755 2008-07-18 19:57:45Z vboxsync $ */
/** @file
 * Helpers for writing libpcap files.
 */

/*
 * Copyright (C) 2006-2008 Sun Microsystems, Inc.
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

#ifndef ___VBox_Pcap_h
#define ___VBox_Pcap_h

#include <iprt/stream.h>

__BEGIN_DECLS

int PcapStreamHdr(PRTSTREAM pStream, uint64_t StartNanoTS);
int PcapStreamFrame(PRTSTREAM pStream, uint64_t StartNanoTS, const void *pvFrame, size_t cbFrame, size_t cbMax);

int PcapFileHdr(RTFILE File, uint64_t StartNanoTS);
int PcapFileFrame(RTFILE File, uint64_t StartNanoTS, const void *pvFrame, size_t cbFrame, size_t cbMax);

__END_DECLS

#endif
