/* $XFree86: xc/programs/Xserver/hw/xfree86/ramdac/xf86Cursor.h,v 1.7 2001/05/18 20:22:31 tsi Exp $ */

#ifndef _XF86CURSOR_H
#define _XF86CURSOR_H

#include "xf86str.h"
#include "mipointer.h"

typedef struct _xf86CursorInfoRec {
    ScrnInfoPtr pScrn;
    int Flags;
    int MaxWidth;
    int MaxHeight;
    void (*SetCursorColors)(ScrnInfoPtr pScrn, int bg, int fg);
    void (*SetCursorPosition)(ScrnInfoPtr pScrn, int x, int y);
    void (*LoadCursorImage)(ScrnInfoPtr pScrn, unsigned char *bits);
    void (*HideCursor)(ScrnInfoPtr pScrn);
    void (*ShowCursor)(ScrnInfoPtr pScrn);
    unsigned char* (*RealizeCursor)(struct _xf86CursorInfoRec *, CursorPtr);
    Bool (*UseHWCursor)(ScreenPtr, CursorPtr);

} xf86CursorInfoRec, *xf86CursorInfoPtr;

Bool xf86InitCursor(
   ScreenPtr pScreen, 
   xf86CursorInfoPtr infoPtr
);

xf86CursorInfoPtr xf86CreateCursorInfoRec(void);
void xf86DestroyCursorInfoRec(xf86CursorInfoPtr);


void xf86SetCursor(ScreenPtr pScreen, CursorPtr pCurs, int x, int y);
void xf86MoveCursor(ScreenPtr pScreen, int x, int y);
void xf86RecolorCursor(ScreenPtr pScreen, CursorPtr pCurs, Bool displayed);
Bool xf86InitHardwareCursor(ScreenPtr pScreen, xf86CursorInfoPtr infoPtr);

extern int xf86CursorScreenIndex;

#define HARDWARE_CURSOR_INVERT_MASK 			0x00000001
#define HARDWARE_CURSOR_AND_SOURCE_WITH_MASK		0x00000002
#define HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK		0x00000004
#define HARDWARE_CURSOR_SOURCE_MASK_NOT_INTERLEAVED	0x00000008
#define HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1	0x00000010
#define HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_8	0x00000020
#define HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_16	0x00000040
#define HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_32	0x00000080
#define HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_64	0x00000100
#define HARDWARE_CURSOR_TRUECOLOR_AT_8BPP		0x00000200
#define HARDWARE_CURSOR_BIT_ORDER_MSBFIRST		0x00000400
#define HARDWARE_CURSOR_NIBBLE_SWAPPED			0x00000800
#define HARDWARE_CURSOR_SHOW_TRANSPARENT		0x00001000
#define HARDWARE_CURSOR_UPDATE_UNHIDDEN			0x00002000

#endif /* _XF86CURSOR_H */