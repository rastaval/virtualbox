/** $Id: counters.h $ */
/** @file
 * Counters macro invocation template.
 *
 * This is included with different PROFILE_COUNTER and COUNTING_COUNTER
 * implementations to instantiate data members, create function prototypes and
 * implement these prototypes.
 */

/*
 * Copyright (C) 2007-2009 Sun Microsystems, Inc.
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

#ifndef PROFILE_COUNTER
# error PROFILE_COUNTER is not defied
#endif
#ifndef COUNTING_COUNTER
# error COUNTING_COUNTER is not defined
#endif

PROFILE_COUNTER(Fill, "Profiling slirp fills");
PROFILE_COUNTER(Poll, "Profiling slirp polls");
PROFILE_COUNTER(FastTimer, "Profiling slirp fast timer");
PROFILE_COUNTER(SlowTimer, "Profiling slirp slow timer");
PROFILE_COUNTER(IOwrite, "Profiling IO sowrite");
PROFILE_COUNTER(IOread, "Profiling IO soread");

COUNTING_COUNTER(TCP, "TCP sockets");
COUNTING_COUNTER(TCPHot, "TCP sockets active");
COUNTING_COUNTER(UDP, "UDP sockets");
COUNTING_COUNTER(UDPHot, "UDP sockets active");

COUNTING_COUNTER(IORead_in_1, "SB IORead_in_1");
COUNTING_COUNTER(IORead_in_1_bytes, "SB IORead_in_1_bytes");
COUNTING_COUNTER(IORead_in_2, "SB IORead_in_2");
COUNTING_COUNTER(IORead_in_2_1st_bytes, "SB IORead_in_2_1st_bytes");
COUNTING_COUNTER(IORead_in_2_2nd_bytes, "SB IORead_in_2_2nd_bytes");
COUNTING_COUNTER(IOWrite_in_1, "SB IOWrite_in_1");
COUNTING_COUNTER(IOWrite_in_1_bytes, "SB IOWrite_in_1_bytes");
COUNTING_COUNTER(IOWrite_in_2, "SB IOWrite_in_2");
COUNTING_COUNTER(IOWrite_in_2_1st_bytes, "SB IOWrite_in_2_1st_bytes");
COUNTING_COUNTER(IOWrite_in_2_2nd_bytes, "SB IOWrite_in_2_2nd_bytes");
COUNTING_COUNTER(IOWrite_no_w, "SB IOWrite_no_w");
COUNTING_COUNTER(IOWrite_rest, "SB IOWrite_rest");
COUNTING_COUNTER(IOWrite_rest_bytes, "SB IOWrite_rest_bytes");

PROFILE_COUNTER(IOSBAppend_pf, "Profiling sbuf::append common");
PROFILE_COUNTER(IOSBAppend_pf_wa, "Profiling sbuf::append all writen in network");
PROFILE_COUNTER(IOSBAppend_pf_wf, "Profiling sbuf::append writen fault");
PROFILE_COUNTER(IOSBAppend_pf_wp, "Profiling sbuf::append writen partly");
COUNTING_COUNTER(IOSBAppend, "SB: Append total");
COUNTING_COUNTER(IOSBAppend_wa, "SB: Append all is written to network ");
COUNTING_COUNTER(IOSBAppend_wf, "SB: Append nothing is written");
COUNTING_COUNTER(IOSBAppend_wp, "SB: Append is written partly");
COUNTING_COUNTER(IOSBAppend_zm, "SB: Append mbuf is zerro or less");

COUNTING_COUNTER(IOSBAppendSB, "SB: AppendSB total");
COUNTING_COUNTER(IOSBAppendSB_w_l_r, "SB: AppendSB (sb_wptr < sb_rptr)");
COUNTING_COUNTER(IOSBAppendSB_w_ge_r, "SB: AppendSB (sb_wptr >= sb_rptr)");
COUNTING_COUNTER(IOSBAppendSB_w_alter, "SB: AppendSB (altering of sb_wptr)");

PROFILE_COUNTER(TCP_reassamble, "TCP::reasamble");
PROFILE_COUNTER(TCP_input, "TCP::input");
