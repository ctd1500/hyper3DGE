//----------------------------------------------------------------------------
//  EDGE Global State Variables
//----------------------------------------------------------------------------
// 
//  Copyright (c) 1999-2007  The EDGE Team.
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
//
// -MH- 1998/07/02 "lookupdown" --> "true3dgameplay"
//
// -ACB- 1999/10/07 Removed Sound Parameters - New Sound API
//

#ifndef __D_STATE__
#define __D_STATE__

// We need globally shared data structures,
//  for defining the global state variables.
#include "dm_data.h"

// We need the playr data structure as well.
#include "e_player.h"

#include "epi/utility.h"

extern bool devparm;  // DEBUG: launched with -devparm

extern gameflags_t level_flags;
extern gameflags_t global_flags;

// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern skill_t startskill;
extern char *startmap;

extern int startplayers;
extern int startbots;

extern bool autostart;

// Selected by user. 
extern skill_t gameskill;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern int deathmatch;

// -------------------------
// Status flags for refresh.
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern bool statusbaractive;
extern int automapactive;  // In AutoMap mode?
extern bool menuactive;  // Menu overlayed?
extern bool rts_menuactive;
extern bool paused;  // Game Pause?
extern bool viewactive;
extern bool nodrawers;
extern bool noblit;

// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern angle_t viewanglebaseoffset;

// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern int totalkills;
extern int totalitems;
extern int totalsecret;

// Timer, for scores.
extern int leveltime;  // tics in game play for par

// --------------------------------------
// DEMO playback/recording related stuff.

//?
extern bool demoplayback;
extern bool demorecording;

// -AJA- 2000/12/07: auto quick-load feature
extern bool autoquickload;

// Quit after playing a demo from cmdline.
extern bool singledemo;

//?
extern gamestate_e gamestate;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern int gametic;

// The maximum number of players, multiplayer/networking.
#define MAXPLAYERS  30

// Pointer to each player in the game.
extern player_t *players[MAXPLAYERS];
extern int numplayers;

// Player taking events, and displaying.
extern int consoleplayer;
extern int displayplayer;

#define DEATHMATCH()  (deathmatch > 0)
#define COOP_MATCH()  (deathmatch == 0 && numplayers > 1)
#define SP_MATCH()    (deathmatch == 0 && numplayers <= 1)

#define MAXHEALTH 200
#define MAXARMOUR 200

#define CHEATARMOUR      MAXARMOUR
#define CHEATARMOURTYPE  ARMOUR_Blue

// Player spawn spots for deathmatch, coop.
extern spawnpointarray_c dm_starts;
extern spawnpointarray_c coop_starts;
extern spawnpointarray_c voodoo_doll_starts;

// Intermission stats.
// Parameters for world map / intermission.
extern wbstartstruct_t wminfo;

//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern epi::strent_c cfgfile;

extern epi::strent_c iwad_base;

extern epi::strent_c cache_dir;
extern epi::strent_c ddf_dir;
extern epi::strent_c game_dir;
extern epi::strent_c save_dir;
extern epi::strent_c shot_dir;

// if true, load DDF/RTS as external files (instead of from EDGE.WAD)
extern bool external_ddf;

// if true, load all graphics at level load
extern bool precache;

// if true, prefer to crash out on various errors
extern bool strict_errors;

// if true, prefer to ignore or fudge various (serious) errors
extern bool lax_errors;

// if true, disable warning messages
extern bool no_warnings;

// if true, disable obsolete warning messages
extern bool no_obsoletes;

// if true, enable HOM detection (hall of mirrors effect)
extern bool hom_detect;

extern int mouseSensitivity;
extern int save_page;

extern int quickSaveSlot;

// debug flag to cancel adaptiveness
extern bool singletics;

extern int bodyqueslot;

// Needed to store the number of the dummy sky flat.
// Used for rendering, as well as tracking projectiles etc.

extern const struct image_s *skyflatimage;

#define IS_SKY(plane)  ((plane).image == skyflatimage)


//---------------------------------------------------
// Netgame stuff (buffers and pointers, i.e. indices).

extern int maketic;

//misc stuff
extern bool map_overlay;
extern bool rotatemap;
extern bool showstats;
extern bool swapstereo;
extern bool infight;
extern bool png_scrshots;

typedef enum
{
	HUD_Full = 0,
	HUD_None,
	HUD_Overlay,

	NUMHUD
}
hud_type_e;

extern int crosshair;
extern int screen_hud;
extern int sky_stretch;
extern int menunormalfov, menuzoomedfov;

// -ES- 1999/08/15 Added teleport effects
extern int telept_effect;
extern int telept_flash;
extern int telept_reverse;

//mlook stuff
extern int mlookspeed;
extern bool invertmouse; // -ACB- 1999/09/03 Must be true or false - becomes boolean

// -KM- 1998/09/01 Analogue binding stuff, These hold what axis they bind to.
extern int joy_xaxis;
extern int joy_yaxis;
extern int mouse_xaxis;
extern int mouse_yaxis;

//
// -ACB- 1998/09/06 Analogue binding:
//                   Two stage turning, angleturn control
//                   horzmovement control, vertmovement control
//                   strafemovediv;
//
extern bool stageturn;
extern int forwardmovespeed;
extern int angleturnspeed;
extern int sidemovespeed;

#endif // __D_STATE__

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
