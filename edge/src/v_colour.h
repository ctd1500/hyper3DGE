//----------------------------------------------------------------------------
//  EDGE Colour Code
//----------------------------------------------------------------------------
// 
//  Copyright (c) 1999-2000  The EDGE Team.
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------
//
//  Based on the DOOM source code, released by Id Software under the
//  following copyright:
//
//    Copyright (C) 1993-1996 by id Software, Inc.
//
//----------------------------------------------------------------------------

#ifndef __V_COLOUR_H__
#define __V_COLOUR_H__

#include "dm_type.h"
#include "dm_defs.h"

#include "ddf_main.h"
#include "r_defs.h"

// -ACB- 1999/10/11 Gets an RGB colour from the current palette
// -AJA- Added `nominal' version, which gets colour from palette 0.
void V_IndexColourToRGB(int indexcol, byte *returncol);
void V_IndexNominalToRGB(int indexcol, byte *returncol);

// -ES- 1998/11/29 Added translucency tables
// -AJA- 1999/06/30: Moved 'em here, from dm_state.h.
extern unsigned long col2rgb16[65][256][2];
extern unsigned long col2rgb8[65][256];
extern unsigned char rgb_32k[32][32][32];  // 32K RGB table, for 8-bit translucency

extern unsigned long hicolourtransmask;  // OR mask used for translucency

// -AJA- 1999/07/03: moved here from v_res.h.
extern byte gammatable[5][256];
extern int usegamma;
extern int current_gamma;
extern int interpolate_colmaps;

extern void V_ReadPalette(void);
extern void V_CalcTranslucencyTables(void);

extern void V_InitColour(void);

// -AJA- 1999/07/03: Some palette stuff. Should be replaced later on with
// some DDF system (e.g. "palette.ddf").

#define PALETTE_NORMAL   0
#define PALETTE_PAIN     1
#define PALETTE_BONUS    2
#define PALETTE_SUIT     3

extern void V_SetPalette(int type, float_t amount);

//
// V_ColourNewFrame
//
// Call this at the start of each frame (before any rendering or
// render-related work has been done).  Will update the palette and/or
// gamma settings if they have changed since the last call.
//
extern void V_ColourNewFrame(void);

// -AJA- 1999/07/05: Added this, to be used instead of the
// Allegro-specific `palette_color' array.

extern long pixel_values[256];

// -AJA- 1999/07/10: Some stuff for colmap.ddf.

typedef enum
{
  VCOL_Sky = 0x0001
}
vcol_flags_e;

const coltable_t *V_GetRawColtable(const colourmap_t * nominal, int level);
const coltable_t *V_GetColtable
 (const colourmap_t * nominal, int lightlevel, vcol_flags_e flags);

// translation support
const byte *V_GetTranslationTable(const colourmap_t * colmap);

// text translation tables
extern const byte *text_colour_white;
extern const byte *text_colour_grey;
extern const byte *text_colour_green;
extern const byte *text_colour_brown;
extern const byte *text_colour_lt_blue;
extern const byte *text_colour_blue;
extern const byte *text_colour_yellow;


#endif
