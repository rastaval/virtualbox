/* $Id: feedback_context.c $ */

/** @file
 * VBox feedback spu, context tracking.
 */

/*
 * Copyright (C) 2009 Sun Microsystems, Inc.
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

#include "cr_spu.h"
#include "cr_error.h"
#include "feedbackspu.h"

/*@todo Multithreading case. (See feedback_spu.self.RenderMode)*/

GLint FEEDBACKSPU_APIENTRY
feedbackspu_CreateContext( const char *dpyName, GLint visual, GLint shareCtx )
{
    GLint ctx, slot;

#ifdef CHROMIUM_THREADSAFE
    crLockMutex(&feedback_spu.mutex);
#endif

    ctx = feedback_spu.child.CreateContext(dpyName, visual, shareCtx);

    /* find an empty context slot */
    for (slot = 0; slot < feedback_spu.numContexts; slot++) {
        if (!feedback_spu.context[slot].clientState) {
            /* found empty slot */
            break;
        }
    }
    if (slot == feedback_spu.numContexts) {
        feedback_spu.numContexts++;
    }

    feedback_spu.context[slot].clientState = crStateCreateContext(NULL, visual, NULL);
    feedback_spu.context[slot].clientCtx = ctx;

#ifdef CHROMIUM_THREADSAFE
    crUnlockMutex(&feedback_spu.mutex);
#endif

    return ctx;
}

void FEEDBACKSPU_APIENTRY
feedbackspu_MakeCurrent( GLint window, GLint nativeWindow, GLint ctx )
{
#ifdef CHROMIUM_THREADSAFE
    crLockMutex(&feedback_spu.mutex);
#endif
    feedback_spu.child.MakeCurrent(window, nativeWindow, ctx);

    if (ctx) {
        int slot;
        GLint oldmode;

        for (slot=0; slot<feedback_spu.numContexts; ++slot)
            if (feedback_spu.context[slot].clientCtx == ctx) break;
        CRASSERT(slot < feedback_spu.numContexts);

        crStateMakeCurrent(feedback_spu.context[slot].clientState);

        crStateGetIntegerv(GL_RENDER_MODE, &oldmode);

        if (oldmode!=feedback_spu.render_mode)
        {
            feedback_spu.self.RenderMode(oldmode);
        }
    }
    else 
    {
        crStateMakeCurrent(NULL);
    }

#ifdef CHROMIUM_THREADSAFE
    crUnlockMutex(&feedback_spu.mutex);
#endif
}

void FEEDBACKSPU_APIENTRY
feedbackspu_DestroyContext( GLint ctx )
{
#ifdef CHROMIUM_THREADSAFE
    crLockMutex(&feedback_spu.mutex);
#endif
    feedback_spu.child.DestroyContext(ctx);

    if (ctx) {
        int slot;

        for (slot=0; slot<feedback_spu.numContexts; ++slot)
            if (feedback_spu.context[slot].clientCtx == ctx) break;
        CRASSERT(slot < feedback_spu.numContexts);

        crStateDestroyContext(feedback_spu.context[slot].clientState);

        feedback_spu.context[slot].clientState = NULL;
        feedback_spu.context[slot].clientCtx = 0;
    }

#ifdef CHROMIUM_THREADSAFE
    crUnlockMutex(&feedback_spu.mutex);
#endif
}