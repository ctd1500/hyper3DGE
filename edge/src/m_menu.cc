//----------------------------------------------------------------------------
//  EDGE Main Menu Code
//----------------------------------------------------------------------------
// 
//  Copyright (c) 1999-2001  The EDGE Team.
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
// See M_Option.C for text built menus.
//
// -KM- 1998/07/21 Add support for message input.
//

#include "i_defs.h"
#include "m_menu.h"

#include "dm_defs.h"
#include "dm_state.h"

#include "con_main.h"
#include "ddf_main.h"
#include "dstrings.h"
#include "e_main.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "m_argv.h"
#include "m_inline.h"
#include "m_misc.h"
#include "m_option.h"
#include "m_swap.h"
#include "m_random.h"
#include "r_local.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "sv_chunk.h"
#include "sv_main.h"
#include "v_ctx.h"
#include "v_res.h"
#include "v_colour.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "z_zone.h"

//
// defaulted values
//
int mouseSensitivity;  // has default

// Show messages has default, 0 = off, 1 = on
int showMessages;

int screenblocks;  // has default

int darken_screen;

// 1 = message to be printed
int messageToPrint;

// ...and here is the message string!
const char *messageString;

// -KM- 1998/07/21  This string holds what the user has typed in
char *messageInputString;

// message x & y
int messx;
int messy;
int messageLastMenuActive;

bool inhelpscreens;
bool menuactive;

#define SKULLXOFF   -24
#define LINEHEIGHT   15

// timed message = no input from user
static bool messageNeedsInput;

static void (* message_key_routine)(int response) = NULL;
static void (* message_input_routine)(char *response) = NULL;

static int chosen_epi;

const char *gammamsg[5];


//
//  IMAGES USED
//
const image_t *therm_l;
const image_t *therm_m;
const image_t *therm_r;
const image_t *therm_o;

static const image_t *menu_loadg;
static const image_t *menu_saveg;
static const image_t *menu_svol;
static const image_t *menu_doom;
static const image_t *menu_newgame;
static const image_t *menu_skill;
static const image_t *menu_episode;
static const image_t *menu_skull[2];
static const image_t *menu_readthis[2];


//
//  SAVE STUFF
//
#define SAVESTRINGSIZE 	24

#define SAVE_SLOTS  8
#define SAVE_PAGES  100  // more would be rather unwieldy

// -1 = no quicksave slot picked!
int quickSaveSlot;
int quickSavePage;

// 25-6-98 KM Lots of save games... :-)
static int save_page = 0;
static int save_slot = 0;

// we are going to be entering a savegame string
static int saveStringEnter;

// which char we're editing
static int saveCharIndex;

// old save description before edit
static char saveOldString[SAVESTRINGSIZE];

typedef struct slot_extra_info_s
{
	bool empty;
	bool corrupt;

	char desc[SAVESTRINGSIZE];
	char timestr[32];
  
	char mapname[10];
	char gamename[32];
  
	int skill;
	int netgame;
	bool has_view;
}
slot_extra_info_t;

static slot_extra_info_t ex_slots[SAVE_SLOTS];

// 98-7-10 KM New defines for slider left.
// Part of savegame changes.
#define SLIDERLEFT  -1
#define SLIDERRIGHT -2


//
// MENU TYPEDEFS
//
typedef struct
{
	// 0 = no cursor here, 1 = ok, 2 = arrows ok
	int status;

  // image for menu entry
	char patch_name[10];
	const image_t *image;

  // choice = menu item #.
  // if status = 2, choice can be SLIDERLEFT or SLIDERRIGHT
	void (* select_func)(int choice);

	// hotkey in menu
	char alpha_key;
}
menuitem_t;

typedef struct menu_s
{
	// # of menu items
	int numitems;

  // previous menu
	struct menu_s *prevMenu;

	// menu items
	menuitem_t *menuitems;

	// draw routine
	void (* draw_func)(void);

	// x,y of menu
	int x, y;

	// last item user was on in menu
	int lastOn;
}
menu_t;

// menu item skull is on
static int itemOn;

// skull animation counter
static int skullAnimCounter;

// which skull to draw
static int whichSkull;

// current menudef
static menu_t *currentMenu;

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);

// 25-6-98 KM
void M_LoadSavePage(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);

void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x, int y, int len);
void M_SetupNextMenu(menu_t * menudef);
void M_DrawEmptyCell(menu_t * menu, int item);
void M_DrawSelCell(menu_t * menu, int item);
void M_StartControlPanel(void);
void M_StopMessage(void);
void M_ClearMenus(void);

//
// DOOM MENU
//
enum
{
	newgame = 0,
	options,
	loadgame,
	savegame,
	readthis,
	quitdoom,
	main_end
}
main_e;

static menuitem_t MainMenu[] =
{
	{1, "M_NGAME",   NULL, M_NewGame, 'n'},
	{1, "M_OPTION",  NULL, M_Options, 'o'},
	{1, "M_LOADG",   NULL, M_LoadGame, 'l'},
	{1, "M_SAVEG",   NULL, M_SaveGame, 's'},
	// Another hickup with Special edition.
	{1, "M_RDTHIS",  NULL, M_ReadThis, 'r'},
	{1, "M_QUITG",   NULL, M_QuitEDGE, 'q'}
};

static menu_t MainDef =
{
	main_end,
	NULL,
	MainMenu,
	M_DrawMainMenu,
	97, 64,
	0
};

//
// EPISODE SELECT
//
// -KM- 1998/12/16 This is generated dynamically.
//
static menuitem_t *EpisodeMenu = NULL;

static menuitem_t DefaultEpiMenu =
{
	1,  // status
	"Working",  // name
	NULL,  // image
	NULL,  // select_func
	'w'  // alphakey
};

static menu_t EpiDef =
{
	0,  //ep_end,  // # of menu items
	&MainDef,  // previous menu
	&DefaultEpiMenu,  // menuitem_t ->
	M_DrawEpisode,  // drawing routine ->
	48, 63,  // x,y
	0  // lastOn
};

static menuitem_t NewGameMenu[] =
{
	{1, "M_JKILL", NULL, M_ChooseSkill, 'p'},
	{1, "M_ROUGH", NULL, M_ChooseSkill, 'r'},
	{1, "M_HURT",  NULL, M_ChooseSkill, 'h'},
	{1, "M_ULTRA", NULL, M_ChooseSkill, 'u'},
	{1, "M_NMARE", NULL, M_ChooseSkill, 'n'}
};

static menu_t NewDef =
{
	sk_numtypes,  // # of menu items
	&EpiDef,  // previous menu
	NewGameMenu,  // menuitem_t ->
	M_DrawNewGame,  // drawing routine ->
	48, 63,  // x,y
	sk_medium  // lastOn
};

//
// OPTIONS MENU
//
enum
{
	endgame,
	messages,
	scrnsize,
	option_empty1,
	mousesens,
	option_empty2,
	soundvol,
	opt_end
}
options_e;

//
// Read This! MENU 1 & 2
//
enum
{
	rdthsempty1,
	read1_end
}
read_e;

static menuitem_t ReadMenu1[] =
{
	{1, "", NULL, M_ReadThis2, 0}
};

static menu_t ReadDef1 =
{
	read1_end,
	&MainDef,
	ReadMenu1,
	M_DrawReadThis1,
	280, 185,
	0
};

enum
{
	rdthsempty2,
	read2_end
}
read_e2;

static menuitem_t ReadMenu2[] =
{
	{1, "", NULL, M_FinishReadThis, 0}
};

static menu_t ReadDef2 =
{
	read2_end,
	&ReadDef1,
	ReadMenu2,
	M_DrawReadThis2,
	330, 175,
	0
};

//
// SOUND VOLUME MENU
//
enum
{
	sfx_vol,
	sfx_empty1,
	music_vol,
	sfx_empty2,
	sound_end
}
sound_e;

static menuitem_t SoundMenu[] =
{
	{2, "M_SFXVOL", NULL, M_SfxVol, 's'},
	{-1, "", NULL, 0},
	{2, "M_MUSVOL", NULL, M_MusicVol, 'm'},
	{-1, "", NULL, 0}
};

static menu_t SoundDef =
{
	sound_end,
	&MainDef,  ///  &OptionsDef,
	SoundMenu,
	M_DrawSound,
	80, 64,
	0
};

//
// LOAD GAME MENU
//
// Note: upto 10 slots per page
//
static menuitem_t LoadingMenu[] =
{
	{2, "", NULL, M_LoadSelect, '1'},
	{2, "", NULL, M_LoadSelect, '2'},
	{2, "", NULL, M_LoadSelect, '3'},
	{2, "", NULL, M_LoadSelect, '4'},
	{2, "", NULL, M_LoadSelect, '5'},
	{2, "", NULL, M_LoadSelect, '6'},
	{2, "", NULL, M_LoadSelect, '7'},
	{2, "", NULL, M_LoadSelect, '8'},
	{2, "", NULL, M_LoadSelect, '9'},
	{2, "", NULL, M_LoadSelect, '0'}
};

static menu_t LoadDef =
{
	SAVE_SLOTS,
	&MainDef,
	LoadingMenu,
	M_DrawLoad,
	30, 34,
	0
};

//
// SAVE GAME MENU
//
static menuitem_t SavingMenu[] =
{
	{2, "", NULL, M_SaveSelect, '1'},
	{2, "", NULL, M_SaveSelect, '2'},
	{2, "", NULL, M_SaveSelect, '3'},
	{2, "", NULL, M_SaveSelect, '4'},
	{2, "", NULL, M_SaveSelect, '5'},
	{2, "", NULL, M_SaveSelect, '6'},
	{2, "", NULL, M_SaveSelect, '7'},
	{2, "", NULL, M_SaveSelect, '8'},
	{2, "", NULL, M_SaveSelect, '9'},
	{2, "", NULL, M_SaveSelect, '0'}
};

static menu_t SaveDef =
{
	SAVE_SLOTS,
	&MainDef,
	SavingMenu,
	M_DrawSave,
	30, 34,
	0
};

// 98-7-10 KM Chooses the page of savegames to view
void M_LoadSavePage(int choice)
{
	switch (choice)
	{
		case SLIDERLEFT:
			// -AJA- could use `OOF' sound...
			if (save_page == 0)
				return;

			save_page--;
			break;
      
		case SLIDERRIGHT:
			if (save_page >= SAVE_PAGES-1)
				return;

			save_page++;
			break;
	}

	S_StartSound(NULL, sfx_swtchn);
	M_ReadSaveStrings();
}

//
// M_ReadSaveStrings
//
// Read the strings from the savegame files
//
// 98-7-10 KM Savegame slots increased
//
void M_ReadSaveStrings(void)
{
	int i, version;
  
	char *filename;
	saveglobals_t *globs;

	for (i=0; i < SAVE_SLOTS; i++)
	{
		ex_slots[i].empty = false;
		ex_slots[i].corrupt = true;

		ex_slots[i].skill = -1;
		ex_slots[i].netgame = -1;
		ex_slots[i].has_view = false;

		ex_slots[i].desc[0] = 0;
		ex_slots[i].timestr[0] = 0;
		ex_slots[i].mapname[0] = 0;
		ex_slots[i].gamename[0] = 0;
    
		filename = G_FileNameFromSlot(save_page * SAVE_SLOTS + i);

		if (! SV_OpenReadFile(filename))
		{
			ex_slots[i].empty = true;
			ex_slots[i].corrupt = false;
			Z_Free(filename);
			continue;
		}

		Z_Free(filename);

		if (! SV_VerifyHeader(&version))
		{
			SV_CloseReadFile();
			continue;
		}

		globs = SV_LoadGLOB();

		// close file now -- we only need the globals
		SV_CloseReadFile();

		if (! globs)
			continue;

		// --- pull info from global structure ---

		if (!globs->game || !globs->level || !globs->description)
		{
			SV_FreeGLOB(globs);
			continue;
		}

		ex_slots[i].corrupt = false;

		Z_StrNCpy(ex_slots[i].gamename, globs->game,  32-1);
		Z_StrNCpy(ex_slots[i].mapname,  globs->level, 10-1);

		Z_StrNCpy(ex_slots[i].desc, globs->description, SAVESTRINGSIZE-1);

		if (globs->desc_date)
			Z_StrNCpy(ex_slots[i].timestr, globs->desc_date, 32-1);

		ex_slots[i].skill   = globs->skill;
		ex_slots[i].netgame = globs->netgame;

		SV_FreeGLOB(globs);
    
#if 0
		// handle screenshot
		if (globs->view_pixels)
		{
			int x, y;
      
			for (y=0; y < 100; y++)
				for (x=0; x < 160; x++)
				{
					save_screenshot[x][y] = SV_GetShort();
				}
		}
#endif
	}

	// fix up descriptions
	for (i=0; i < SAVE_SLOTS; i++)
	{
		if (ex_slots[i].corrupt)
		{
			strncpy(ex_slots[i].desc, DDF_LanguageLookup("CorruptSlot"),
					SAVESTRINGSIZE - 1);
			continue;
		}
		else if (ex_slots[i].empty)
		{
			strncpy(ex_slots[i].desc, DDF_LanguageLookup("EmptySlot"),
					SAVESTRINGSIZE - 1);
			continue;
		}
	}
}

static void M_DrawSaveLoadCommon(int row, int row2)
{
	int y = LoadDef.y + LINEHEIGHT * row;

	slot_extra_info_t *info;

	char mbuffer[200];


	sprintf(mbuffer, "PAGE %d", save_page + 1);

	// -KM-  1998/06/25 This could quite possibly be replaced by some graphics...
	if (save_page > 0)
		HL_WriteTextTrans(LoadDef.x - 4, y, text_white_map, "< PREV");

	HL_WriteTextTrans(LoadDef.x + 94 - HL_StringWidth(mbuffer) / 2, y,
					  text_white_map, mbuffer);

	if (save_page < SAVE_PAGES-1)
		HL_WriteTextTrans(LoadDef.x + 192 - HL_StringWidth("NEXT >"), y,
						  text_white_map, "NEXT >");
 
	info = ex_slots + itemOn;
	DEV_ASSERT2(0 <= itemOn && itemOn < SAVE_SLOTS);

	if (saveStringEnter || info->empty || info->corrupt)
		return;

	// show some info about the savegame

	y = LoadDef.y + LINEHEIGHT * (row2 + 1);

	mbuffer[0] = 0;

	strcat(mbuffer, info->timestr);

	HL_WriteTextTrans(310 - HL_StringWidth(mbuffer), y, 
					  text_green_map, mbuffer);


	y -= LINEHEIGHT;
    
	mbuffer[0] = 0;

	// FIXME: LDF-itise these
	switch (info->skill)
	{
		case 0: strcat(mbuffer, "Too Young To Die"); break;
		case 1: strcat(mbuffer, "Not Too Rough"); break;
		case 2: strcat(mbuffer, "Hurt Me Plenty"); break;
		case 3: strcat(mbuffer, "Ultra Violence"); break;
		default: strcat(mbuffer, "NIGHTMARE"); break;
	}

	HL_WriteTextTrans(310 - HL_StringWidth(mbuffer), y,
					  text_green_map, mbuffer);


	y -= LINEHEIGHT;
  
	mbuffer[0] = 0;

	switch (info->netgame)
	{
		case 0: strcat(mbuffer, "SP MODE"); break;
		case 1: strcat(mbuffer, "COOP MODE"); break;
		default: strcat(mbuffer, "DM MODE"); break;
	}
  
	HL_WriteTextTrans(310 - HL_StringWidth(mbuffer), y,
					  text_green_map, mbuffer);


	y -= LINEHEIGHT;
  
	mbuffer[0] = 0;

	strcat(mbuffer, info->mapname);

	HL_WriteTextTrans(310 - HL_StringWidth(mbuffer), y,
					  text_green_map, mbuffer);
}

//
// M_LoadGame
//
// 98-7-10 KM Savegame slots increased
//
void M_DrawLoad(void)
{
	int i;

	VCTX_ImageEasy320(72, 8, menu_loadg);
      
	for (i = 0; i < SAVE_SLOTS; i++)
		M_DrawSaveLoadBorder(LoadDef.x + 8, LoadDef.y + LINEHEIGHT * (i), 24);

	// draw screenshot ?

	for (i = 0; i < SAVE_SLOTS; i++)
		HL_WriteText(LoadDef.x + 8, LoadDef.y + LINEHEIGHT * (i), ex_slots[i].desc);

	M_DrawSaveLoadCommon(i, i+1);
}


//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x, int y, int len)
{
	int i;

	const image_t *L = W_ImageFromPatch("M_LSLEFT");
	const image_t *C = W_ImageFromPatch("M_LSCNTR");
	const image_t *R = W_ImageFromPatch("M_LSRGHT");

	VCTX_ImageEasy320(x - IM_WIDTH(L), y + 7, L);

	for (i = 0; i < len; i++, x += IM_WIDTH(C))
		VCTX_ImageEasy320(x, y + 7, C);

	VCTX_ImageEasy320(x, y + 7, R);
}

//
// User wants to load this game
//
// 98-7-10 KM Savegame slots increased
//
void M_LoadSelect(int choice)
{
	if (choice < 0)
	{
		M_LoadSavePage(choice);
		return;
	}

	G_LoadGame(save_page * SAVE_SLOTS + choice);
	M_ClearMenus();
}

//
// Selected from DOOM menu
//
void M_LoadGame(int choice)
{
	if (netgame)
	{
		M_StartMessage(DDF_LanguageLookup("NoLoadInNetGame"), NULL, false);
		return;
	}

	M_SetupNextMenu(&LoadDef);
	M_ReadSaveStrings();
}

//
//  M_SaveGame
//
// 98-7-10 KM Savegame slots increased
//
void M_DrawSave(void)
{
	int i, len;

	VCTX_ImageEasy320(72, 8, menu_saveg);

	for (i = 0; i < SAVE_SLOTS; i++)
	{
		M_DrawSaveLoadBorder(LoadDef.x + 8, LoadDef.y + LINEHEIGHT * (i), 24);

		if (saveStringEnter && i == save_slot)
		{
			len = HL_StringWidth(ex_slots[save_slot].desc);

			HL_WriteTextTrans(LoadDef.x + 8, LoadDef.y + LINEHEIGHT * (i), 
							  text_yellow_map, ex_slots[i].desc);

			HL_WriteTextTrans(LoadDef.x + len + 8, LoadDef.y + LINEHEIGHT * 
							  (i), text_yellow_map, "_");
		}
		else
			HL_WriteText(LoadDef.x + 8, LoadDef.y + LINEHEIGHT * (i), ex_slots[i].desc);
	}

	M_DrawSaveLoadCommon(i, i+1);
}

//
// M_Responder calls this when user is finished
//
// 98-7-10 KM Savegame slots increased
//
void M_DoSave(int page, int slot)
{
	G_SaveGame(page * SAVE_SLOTS + slot, ex_slots[slot].desc);
	M_ClearMenus();

	// PICK QUICKSAVE SLOT YET?
	if (quickSaveSlot == -2)
	{
		quickSavePage = page;
		quickSaveSlot = slot;
	}
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
	if (choice < 0)
	{
		M_LoadSavePage(choice);
		return;
	}

	// we are going to be intercepting all chars
	saveStringEnter = 1;

	save_slot = choice;
	strcpy(saveOldString, ex_slots[choice].desc);

	if (ex_slots[choice].empty)
		ex_slots[choice].desc[0] = 0;

	saveCharIndex = strlen(ex_slots[choice].desc);
}

//
// Selected from DOOM menu
//
void M_SaveGame(int choice)
{
	if (!usergame)
	{
		M_StartMessage(DDF_LanguageLookup("SaveWhenNotPlaying"), NULL, false);
		return;
	}

	if (gamestate != GS_LEVEL)
		return;

	M_ReadSaveStrings();
	M_SetupNextMenu(&SaveDef);

	need_save_screenshot = true;
	save_screenshot_valid = false;
}

//
//   M_QuickSave
//
static char tempstring[80];

static void QuickSaveResponse(int ch)
{
	if (ch == 'y')
	{
		M_DoSave(quickSavePage, quickSaveSlot);
		S_StartSound(NULL, sfx_swtchx);
	}
}

void M_QuickSave(void)
{
	if (!usergame)
	{
		S_StartSound(NULL, sfx_oof);
		return;
	}

	if (gamestate != GS_LEVEL)
		return;

	if (quickSaveSlot < 0)
	{
		M_StartControlPanel();
		M_ReadSaveStrings();
		M_SetupNextMenu(&SaveDef);

		need_save_screenshot = true;
		save_screenshot_valid = false;

		quickSaveSlot = -2;  // means to pick a slot now
		return;
	}

	sprintf(tempstring, DDF_LanguageLookup("QuickSaveOver"),
			ex_slots[quickSaveSlot].desc);

	M_StartMessage(tempstring, QuickSaveResponse, true);
}

//
// M_QuickLoad
//
static void QuickLoadResponse(int ch)
{
	if (ch == 'y')
	{
		int tempsavepage = save_page;

		save_page = quickSavePage;
		M_LoadSelect(quickSaveSlot);

		save_page = tempsavepage;
		S_StartSound(NULL, sfx_swtchx);
	}
}

void M_QuickLoad(void)
{
	if (netgame)
	{
		M_StartMessage(DDF_LanguageLookup("NoQLoadInNet"), NULL, false);
		return;
	}

	if (quickSaveSlot < 0)
	{
		M_StartMessage(DDF_LanguageLookup("NoQuickSaveSlot"), NULL, false);
		return;
	}

	sprintf(tempstring, DDF_LanguageLookup("QuickLoad"),
			ex_slots[quickSaveSlot].desc);
  
	M_StartMessage(tempstring, QuickLoadResponse, true);
}

//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
	inhelpscreens = true;
  
	VCTX_Image(0, 0, SCREENWIDTH, SCREENHEIGHT, menu_readthis[0]);
}

//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
	inhelpscreens = true;

	VCTX_Image(0, 0, SCREENWIDTH, SCREENHEIGHT, menu_readthis[1]);
}

//
// M_DrawSound
//
// Change Sfx & Music volumes
//
// -ACB- 1999/10/10 Sound API Volume re-added
// -ACB- 1999/11/13 Music API Volume re-added
//
void M_DrawSound(void)
{
	int musicvol;
	int soundvol;

	musicvol = S_GetMusicVolume();
	soundvol = S_GetSfxVolume();

	VCTX_ImageEasy320(60, 38, menu_svol);

	M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (sfx_vol + 1), 16, soundvol, 1);
	M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (music_vol + 1), 16, musicvol, 1);
}

//
// M_Sound
//
void M_Sound(int choice)
{
	M_SetupNextMenu(&SoundDef);
}

//
// M_SfxVol
//
// -ACB- 1999/10/10 Sound API Volume re-added
//
void M_SfxVol(int choice)
{
	int soundvol;

	soundvol = S_GetSfxVolume();

	switch (choice)
	{
		case SLIDERLEFT:
			if (soundvol > S_MIN_VOLUME)
				soundvol--;

			break;

		case SLIDERRIGHT:
			if (soundvol < S_MAX_VOLUME)
				soundvol++;

			break;
	}

	S_SetSfxVolume(soundvol);
}

//
// M_MusicVol
//
// -ACB- 1999/10/07 Removed sound references: New Sound API
//
void M_MusicVol(int choice)
{
	int musicvol;

	musicvol = S_GetMusicVolume();

	switch (choice)
	{
		case SLIDERLEFT:
			if (musicvol > S_MIN_VOLUME)
				musicvol--;

			break;

		case SLIDERRIGHT:
			if (musicvol < S_MAX_VOLUME)
				musicvol++;

			break;
	}

	S_SetMusicVolume(musicvol);
}

//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
	VCTX_ImageEasy320(94, 2, menu_doom);
}

//
// M_NewGame
//
void M_DrawNewGame(void)
{
	VCTX_ImageEasy320(96, 14, menu_newgame);
	VCTX_ImageEasy320(54, 38, menu_skill);
}

void M_NewGame(int choice)
{
	if (netgame && !demoplayback)
	{
		M_StartMessage(DDF_LanguageLookup("NewNetGame"), NULL, false);
		return;
	}

	M_SetupNextMenu(&EpiDef);
}

//
//      M_Episode
//

// -KM- 1998/12/16 Generates EpiDef menu dynamically.
static void CreateEpisodeMenu(void)
{
	int i, j, k, e;
	char alpha;

	EpisodeMenu = Z_ClearNew(menuitem_t, num_wi_maps);

	for (i = 0, e = 0; i < num_wi_maps; i++)
	{
		wi_map_t *wi = wi_maps[i];

		if (W_CheckNumForName(wi->firstmap) == -1)
			continue;

		k = 0;
		EpisodeMenu[e].status = 1;
		EpisodeMenu[e].select_func = M_Episode;
		Z_StrNCpy(EpisodeMenu[e].patch_name, wi->namegraphic, 8);
		EpisodeMenu[e].image = NULL;
		alpha = EpisodeMenu[e].patch_name[0];

		// ????
		for (j = 0; j < e; j++)
		{
			if (alpha == EpisodeMenu[j].alpha_key)
			{
				while (EpisodeMenu[e].patch_name[k] && EpisodeMenu[e].patch_name[k] != ' ')
					k++;

				k++;

				if (EpisodeMenu[e].patch_name[k])
					alpha = EpisodeMenu[e].patch_name[k];

				j = 0;
			}
		}
		EpisodeMenu[e].alpha_key = alpha;
		e++;
	}

	if (e == 0)
		I_Error("No available episodes !\n");

	EpiDef.numitems = e;
	EpiDef.menuitems = EpisodeMenu;
}


void M_DrawEpisode(void)
{
	if (!EpisodeMenu)
		CreateEpisodeMenu();
    
	VCTX_ImageEasy320(54, 38, menu_episode);
}

static void VerifyNightmare(int ch)
{
	int i;

	if (ch != 'y')
		return;

	// -KM- 1998/12/17 Clear the intermission.
	WI_MapInit(NULL);
  
	// find episode (???)
	for (i = 0; strcmp(wi_maps[i]->namegraphic, EpisodeMenu[chosen_epi].patch_name); i++) 
	{ /* nothing here */ }

	if (! G_DeferedInitNew(sk_nightmare, wi_maps[i]->firstmap, false))
	{
		// 23-6-98 KM Fixed this.
		M_SetupNextMenu(&EpiDef);
		M_StartMessage(DDF_LanguageLookup("EpisodeNonExist"), NULL, false);
		return;
	}

	M_ClearMenus();
}

void M_ChooseSkill(int choice)
{
	int i;

	if (choice == sk_nightmare)
	{
		M_StartMessage(DDF_LanguageLookup("NightMareCheck"), VerifyNightmare, true);
		return;
	}
	// -KM- 1998/12/17 Clear the intermission
	WI_MapInit(NULL);

	// find episode (???)
	for (i = 0; strcmp(wi_maps[i]->namegraphic, EpisodeMenu[chosen_epi].patch_name); i++)
	{ /* nothing here */ }

	if (! G_DeferedInitNew((skill_t)choice, wi_maps[i]->firstmap, false))
	{
		// 23-6-98 KM Fixed this.
		M_SetupNextMenu(&EpiDef);
		M_StartMessage(DDF_LanguageLookup("EpisodeNonExist"), NULL, false);
		return;
	}

	M_ClearMenus();
}

void M_Episode(int choice)
{
	chosen_epi = choice;
	M_SetupNextMenu(&NewDef);
}

//
// M_Options
//
void M_Options(int choice)
{
	optionsmenuon = 1;
}

//
//      Toggle messages on/off
//
void M_ChangeMessages(int choice)
{
	// warning: unused parameter `int choice'
	(void) choice;

	if (!showMessages)
		CON_Printf("%s\n", DDF_LanguageLookup("MessagesOn"));
	else
		CON_Printf("%s\n", DDF_LanguageLookup("MessagesOff"));

	showMessages = 1 - showMessages;

	message_dontfuckwithme = true;
}

//
// M_EndGame
//
static void EndGameResponse(int ch)
{
	if (ch != 'y')
		return;

	currentMenu->lastOn = itemOn;
	M_ClearMenus();
	E_StartTitle();
}

void M_EndGame(int choice)
{
	if (!usergame)
	{
		S_StartSound(NULL, sfx_oof);
		return;
	}

	if (netgame)
	{
		M_StartMessage(DDF_LanguageLookup("EndNetGame"), NULL, false);
		return;
	}

	M_StartMessage(DDF_LanguageLookup("EndGameCheck"), EndGameResponse, true);
}

//
// M_ReadThis
//
void M_ReadThis(int choice)
{
	M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int choice)
{
	M_SetupNextMenu(&ReadDef2);
}

void M_FinishReadThis(int choice)
{
	M_SetupNextMenu(&MainDef);
}

//
// M_QuitDOOM
//
// -KM- 1998/12/16 Handle sfx that don't exist in this version.
// -KM- 1999/01/31 Generate quitsounds from default.ldf
//
static void QuitResponse(int ch)
{
	if (ch != 'y')
		return;
	if (!netgame)
	{
		int numsounds = 0;
		char refname[16];
		char sound[16];
		int i, start;

		// Count the quit messages
		do
		{
			sprintf(refname, "QuitSnd%d", numsounds + 1);
			if (DDF_LanguageValidRef(refname))
				numsounds++;
			else
				break;
		}
		while (true);

		if (numsounds)
		{
			// cycle through all the quit sounds, until one of them exists
			// (some of the default quit sounds do not exist in DOOM 1)
			start = i = M_Random() % numsounds;
			do
			{
				sprintf(refname, "QuitSnd%d", i + 1);
				sprintf(sound, "DS%s", DDF_LanguageLookup(refname));
				if (W_CheckNumForName(sound) != -1)
				{
					S_StartSound(NULL, DDF_SfxLookupSound(DDF_LanguageLookup(refname)));
					break;
				}
				i = (i + 1) % numsounds;
			}
			while (i != start);
		}

		I_WaitVBL(105);
	}

	// -ACB- 1999/09/20 New exit code order
	// Write the default config file first
	M_SaveDefaults();
	I_SystemShutdown();
	I_DisplayExitScreen();
	I_CloseProgram(0);
}

//
// -ACB- 1998/07/19 Removed offensive messages selection (to some people);
//     Better Random Selection.
// -KM- 1998/07/21 Reinstated counting quit messages, so adding them to dstrings.c
//                   is all you have to do.  Using P_Random for the random number
//                   automatically kills the demo sync...
//                   (hence M_Random()... -AJA-).
//
// -KM- 1998/07/31 Removed Limit. So there.
// -KM- 1999/01/31 Load quit messages from default.ldf
//
void M_QuitEDGE(int choice)
{
	static char *endstring = NULL;
	const char *DOSY;
	const char *chosen_msg;
	char refname[16];

	int num_quitmessages;

	DOSY = DDF_LanguageLookup("PressToQuit");

	num_quitmessages = 0;

	// Count the quit messages
	// -ES- 2000/02/04 Cleaned Up.
	do
	{
		sprintf(refname, "QUITMSG%d", num_quitmessages + 1);
		if (DDF_LanguageValidRef(refname))
			num_quitmessages++;
		else
			break;
	}
	while (true);

	//
	// We pick index 0 which is language sensitive,
	// or one at random, between 1 and maximum number.
	//
	sprintf(refname, "QUITMSG%d", (M_Random() % num_quitmessages) + 1);
	chosen_msg = DDF_LanguageLookup(refname);
	Z_Resize(endstring, char, strlen(chosen_msg) + 3 + strlen(DOSY));
	sprintf(endstring, "%s\n\n%s", chosen_msg, DOSY);

	M_StartMessage(endstring, QuitResponse, true);
}

// 98-7-10 KM Use new defines
void M_ChangeSensitivity(int choice)
{
	switch (choice)
	{
		case SLIDERLEFT:
			if (mouseSensitivity)
				mouseSensitivity--;
			break;
		case SLIDERRIGHT:
			if (mouseSensitivity < 9)
				mouseSensitivity++;
			break;
	}
}

// 98-7-10 KM Use new defines
void M_SizeDisplay(int choice)
{
	switch (choice)
	{
		case SLIDERLEFT:
			if (screen_size > 0)
			{
				screenblocks--;
				screen_size--;
			}
			break;
		case SLIDERRIGHT:
			if (screen_size < 8)
			{
				screenblocks++;
				screen_size++;
			}
			break;
	}

	R_SetViewSize(screenblocks);
}

//
//   MENU FUNCTIONS
//

//
// M_DrawThermo
//
void M_DrawThermo(int x, int y, int thermWidth, int thermDot, int div)
{
	int i, basex = x;
	int step = (8 / div);

	// Note: the (step+1) here is for compatibility with the original
	// code.  It seems required to make the thermo bar tile properly.

	VCTX_Image320(x, y, step+1, IM_HEIGHT(therm_l)/div, therm_l);

	for (i=0, x += step; i < thermWidth; i++, x += step)
	{
		VCTX_Image320(x, y, step+1, IM_HEIGHT(therm_m)/div, therm_m);
	}

	VCTX_Image320(x, y, step+1, IM_HEIGHT(therm_r)/div, therm_r);

	x = basex + step + thermDot * step;

	VCTX_Image320(x, y, step+1, IM_HEIGHT(therm_o)/div, therm_o);
}

void M_StartMessage(const char *string, void (* routine)(int response), 
					bool input)
{
	messageLastMenuActive = menuactive;
	messageToPrint = 1;
	messageString = string;
	message_key_routine = routine;
	message_input_routine = NULL;
	messageNeedsInput = input;
	menuactive = true;
	CON_SetVisible(vs_notvisible);
	return;
}

//
// M_StartMessageInput
//
// -KM- 1998/07/21 Call M_StartMesageInput to start a message that needs a
//                 string input. (You can convert it to a number if you want to.)
//                 
// string:  The prompt, eg "What is your name\n\n" must be either
//          static or globally visible.
//
// routine: Format is void routine(char *s)  Routine will be called
//          with a pointer to the input in s.  s will be NULL if the user
//          pressed ESCAPE to cancel the input.
//  
//          String is allocated by Z_Malloc, it is your responsibility to
//          Z_Free it.
//
void M_StartMessageInput(const char *string,
						 void (* routine)(char *response))
{
	messageLastMenuActive = menuactive;
	messageToPrint = 2;
	messageString = string;
	message_input_routine = routine;
	message_key_routine = NULL;
	messageNeedsInput = true;
	menuactive = true;
	CON_SetVisible(vs_notvisible);
	return;
}

void M_StopMessage(void)
{
	menuactive = messageLastMenuActive;
	messageToPrint = 0;
  
	if (!menuactive)
		save_screenshot_valid = false;
}

//
// CONTROL PANEL
//

//
// M_Responder
//
// -KM- 1998/09/01 Analogue binding, and hat support
//
bool M_Responder(event_t * ev)
{
	int ch;
	int i;

	if (ev->type != ev_keydown)
		return false;

	ch = ev->value.key;

	// -ACB- 1999/10/11 F1 is responsible for print screen at any time
	if (ch == KEYD_F1)
	{
		G_ScreenShot();
		return true;
	}

	// Take care of any messages that need input
	// -KM- 1998/07/21 Message Input
	if (messageToPrint == 1)
	{
		if (messageNeedsInput == true &&
			!(ch == ' ' || ch == 'n' || ch == 'y' || ch == KEYD_ESCAPE))
			return false;

		messageToPrint = 0;
		// -KM- 1998/07/31 Moved this up here to fix bugs.
		menuactive = messageLastMenuActive;

		if (message_key_routine)
			(* message_key_routine)(ch);

		S_StartSound(NULL, sfx_swtchx);
		return true;
	}
	else if (messageToPrint == 2)
	{
		static int messageLength;
		static int messageP;

		if (!messageInputString)
		{
			messageInputString = Z_New(char, 32);
			messageLength = 32;
			messageP = 0;
			Z_Clear(messageInputString, char, messageLength);
		}
		if (ch == KEYD_ENTER)
		{
			menuactive = messageLastMenuActive;
			messageToPrint = 0;

			if (message_input_routine)
				(* message_input_routine)(messageInputString);

			messageInputString = NULL;
			menuactive = false;
			S_StartSound(NULL, sfx_swtchx);
			return true;
		}

		if (ch == KEYD_ESCAPE)
		{
			menuactive = messageLastMenuActive;
			messageToPrint = 0;
      
			if (message_input_routine)
				(* message_input_routine)(NULL);

			menuactive = false;
			save_screenshot_valid = false;

			Z_Free(messageInputString);
			messageInputString = NULL;
			S_StartSound(NULL, sfx_swtchx);
			return true;
		}
		if (ch == KEYD_BACKSPACE)
		{
			if (messageP > 0)
			{
				messageP--;
				messageInputString[messageP] = 0;
			}
			return true;
		}
		ch = toupper(ch);
		if (ch == '-')
			ch = '_';
		if (ch != 32)
			if (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE)
				return true;
		if ((ch >= 32) && (ch <= 127) &&
			(HL_StringWidth(messageInputString) < 300))
		{
			messageInputString[messageP++] = ch;
			messageInputString[messageP] = 0;
		}
		if (messageP == (messageLength - 1))
			Z_Resize(messageInputString, char, ++messageLength);
		return true;
	}

	// new options menu on - use that responder
	if (optionsmenuon)
		return M_OptResponder(ev, ch);

	// Save Game string input
	if (saveStringEnter)
	{
		switch (ch)
		{
			case KEYD_BACKSPACE:
				if (saveCharIndex > 0)
				{
					saveCharIndex--;
					ex_slots[save_slot].desc[saveCharIndex] = 0;
				}
				break;

			case KEYD_ESCAPE:
				saveStringEnter = 0;
				strcpy(ex_slots[save_slot].desc, saveOldString);
				break;

			case KEYD_ENTER:
				saveStringEnter = 0;
				if (ex_slots[save_slot].desc[0])
					M_DoSave(save_page, save_slot);
				break;

			default:
				ch = toupper(ch);
				if (ch != 32)
					if (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE)
						break;
				if (ch >= 32 && ch <= 127 &&
					saveCharIndex < SAVESTRINGSIZE - 1 &&
					HL_StringWidth(ex_slots[save_slot].desc) <
					(SAVESTRINGSIZE - 2) * 8)
				{
					ex_slots[save_slot].desc[saveCharIndex++] = ch;
					ex_slots[save_slot].desc[saveCharIndex] = 0;
				}
				break;
		}
		return true;
	}

	// F-Keys
	if (!menuactive)
	{
		switch (ch)
		{
			case KEYD_MINUS:  // Screen size down

				if (automapactive || chat_on)
					return false;
				// 98-7-10 KM Use new defines
				M_SizeDisplay(SLIDERLEFT);
				S_StartSound(NULL, sfx_stnmov);
				return true;

			case KEYD_EQUALS:  // Screen size up

				if (automapactive || chat_on)
					return false;
				// 98-7-10 KM Use new defines
				M_SizeDisplay(SLIDERRIGHT);
				S_StartSound(NULL, sfx_stnmov);
				return true;

/*
  case KEYD_F1:  // Help key

  M_StartControlPanel();

  //if ( gamemode == retail )
  //  currentMenu = &ReadDef2;
  //else
  currentMenu = &ReadDef1;

  itemOn = 0;
  S_StartSound(NULL, sfx_swtchn);
  return true;
*/

			case KEYD_F2:  // Save

				M_StartControlPanel();
				S_StartSound(NULL, sfx_swtchn);
				M_SaveGame(0);
				return true;

			case KEYD_F3:  // Load

				M_StartControlPanel();
				S_StartSound(NULL, sfx_swtchn);
				M_LoadGame(0);
				return true;

			case KEYD_F4:  // Sound Volume

				M_StartControlPanel();
				currentMenu = &SoundDef;
				itemOn = sfx_vol;
				S_StartSound(NULL, sfx_swtchn);
				return true;

			case KEYD_F5:  // Detail toggle, now loads options menu
				// -KM- 1998/07/31 F5 now loads options menu, detail is obsolete.

				S_StartSound(NULL, sfx_swtchn);
				M_StartControlPanel();
				M_Options(0);
				return true;

			case KEYD_F6:  // Quicksave

				S_StartSound(NULL, sfx_swtchn);
				M_QuickSave();
				return true;

			case KEYD_F7:  // End game

				S_StartSound(NULL, sfx_swtchn);
				M_EndGame(0);
				return true;

			case KEYD_F8:  // Toggle messages

				M_ChangeMessages(0);
				S_StartSound(NULL, sfx_swtchn);
				return true;

			case KEYD_F9:  // Quickload

				S_StartSound(NULL, sfx_swtchn);
				M_QuickLoad();
				return true;

			case KEYD_F10:  // Quit DOOM

				S_StartSound(NULL, sfx_swtchn);
				M_QuitEDGE(0);
				return true;

			case KEYD_F11:  // gamma toggle

				current_gamma++;
				if (current_gamma > 4)
					current_gamma = 0;
				CON_Printf("%s\n", gammamsg[current_gamma]);

				// -AJA- 1999/07/03: removed PLAYPAL reference.
				return true;

		}

		// Pop-up menu?
		if (ch == KEYD_ESCAPE)
		{
			M_StartControlPanel();
			S_StartSound(NULL, sfx_swtchn);
			return true;
		}
		return false;
	}

	// Keys usable within menu
	switch (ch)
	{
		case KEYD_DOWNARROW:
			do
			{
				if (itemOn + 1 > currentMenu->numitems - 1)
					itemOn = 0;
				else
					itemOn++;
				S_StartSound(NULL, sfx_pstop);
			}
			while (currentMenu->menuitems[itemOn].status == -1);
			return true;

		case KEYD_UPARROW:
			do
			{
				if (!itemOn)
					itemOn = currentMenu->numitems - 1;
				else
					itemOn--;
				S_StartSound(NULL, sfx_pstop);
			}
			while (currentMenu->menuitems[itemOn].status == -1);
			return true;

		case KEYD_PGUP:
		case KEYD_LEFTARROW:
			if (currentMenu->menuitems[itemOn].select_func &&
				currentMenu->menuitems[itemOn].status == 2)
			{
				S_StartSound(NULL, sfx_stnmov);
				// 98-7-10 KM Use new defines
				(* currentMenu->menuitems[itemOn].select_func)(SLIDERLEFT);
			}
			return true;

		case KEYD_PGDN:
		case KEYD_RIGHTARROW:
			if (currentMenu->menuitems[itemOn].select_func &&
				currentMenu->menuitems[itemOn].status == 2)
			{
				S_StartSound(NULL, sfx_stnmov);
				// 98-7-10 KM Use new defines
				(* currentMenu->menuitems[itemOn].select_func)(SLIDERRIGHT);
			}
			return true;

		case KEYD_ENTER:
			if (currentMenu->menuitems[itemOn].select_func &&
				currentMenu->menuitems[itemOn].status)
			{
				currentMenu->lastOn = itemOn;
				(* currentMenu->menuitems[itemOn].select_func)(itemOn);
				S_StartSound(NULL, sfx_pistol);
			}
			return true;

		case KEYD_ESCAPE:
			currentMenu->lastOn = itemOn;
			M_ClearMenus();
			S_StartSound(NULL, sfx_swtchx);
			return true;

		case KEYD_BACKSPACE:
			currentMenu->lastOn = itemOn;
			if (currentMenu->prevMenu)
			{
				currentMenu = currentMenu->prevMenu;
				itemOn = currentMenu->lastOn;
				S_StartSound(NULL, sfx_swtchn);
			}
			return true;

		default:
			for (i = itemOn + 1; i < currentMenu->numitems; i++)
				if (currentMenu->menuitems[i].alpha_key == ch)
				{
					itemOn = i;
					S_StartSound(NULL, sfx_pstop);
					return true;
				}
			for (i = 0; i <= itemOn; i++)
				if (currentMenu->menuitems[i].alpha_key == ch)
				{
					itemOn = i;
					S_StartSound(NULL, sfx_pstop);
					return true;
				}
			break;

	}

	return false;
}

//
// M_StartControlPanel
//
void M_StartControlPanel(void)
{
	// intro might call this repeatedly
	if (menuactive)
		return;

	menuactive = 1;
	CON_SetVisible(vs_notvisible);
	currentMenu = &MainDef;  // JDC

	itemOn = currentMenu->lastOn;  // JDC
}

//
// M_Drawer
//
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer(void)
{
	static short x;
	static short y;
	unsigned int i;
	unsigned int max;
	int start;

	inhelpscreens = false;

	// 1998/07/10 KM Darken screen added for all messages (quit messages)
	if (!menuactive)
		return;

	if (darken_screen)
	{
		vctx.SolidBox(0, 0, SCREENWIDTH, SCREENHEIGHT, pal_black, 0.5);
		stbar_update = true;
	}

	// Horiz. & Vertically center string and print it.
	if (messageToPrint)
	{
		// -KM- 1998/06/25 Remove limit.
		// -KM- 1998/07/21 Add space for input
		// -ACB- 1998/06/09 More space for message.
		// -KM- 1998/07/31 User input in different colour.
		char *string;
		char *Mstring;
		int len;
		int input_start;

		// Reserve space for prompt
		len = input_start = strlen(messageString);

		// Reserve space for what the user typed in
		len += messageInputString ? strlen(messageInputString) : 0;

		// Reserve space for '_' cursor
		len += (messageToPrint == 2) ? 1 : 0;

		// Reserve space for NULL Terminator
		len++;

		string = Z_New(char, len);
		Mstring = Z_New(char, len);

		strcpy(Mstring, messageString);

		if (messageToPrint == 2)
		{
			if (messageInputString)
				strcpy(Mstring + strlen(messageString), messageInputString);

			Mstring[i = strlen(Mstring)] = '_';
			Mstring[i + 1] = 0;
		}

		start = 0;

		y = 100 - HL_StringHeight(Mstring) / 2;

		while (*(Mstring + start))
		{
			for (i = 0; i < strlen(Mstring + start); i++)
			{
				if (*(Mstring + start + i) == '\n')
				{
					// copy substring and apply terminator
					Z_MoveData(string, Mstring + start, char, i);
					string[i] = 0;
					start += (i + 1);
					break;
				}
			}

			if (i == strlen(Mstring + start))
			{
				strcpy(string, Mstring + start);
				start += i;
			}

			x = 160 - HL_StringWidth(string) / 2;

			// -KM- 1998/07/31 Colour should be a define or something...
			// -ACB- 1998/09/01 Colour is now a define
			if (start > input_start)
				HL_WriteTextTrans(x, y, text_yellow_map, string);
			else
				HL_WriteText(x, y, string);

			y += hu_font.height;
		}

		Z_Free(string);
		Z_Free(Mstring);

		return;
	}

	// new options menu enable, use that drawer instead
	if (optionsmenuon)
	{
		M_OptDrawer();
		return;
	}

	// call Draw routine
	if (currentMenu->draw_func)
		(* currentMenu->draw_func)();

	// DRAW MENU
	x = currentMenu->x;
	y = currentMenu->y;
	max = currentMenu->numitems;

	for (i = 0; i < max; i++, y += LINEHEIGHT)
	{
		const image_t *image;
    
		// ignore blank lines
		if (! currentMenu->menuitems[i].patch_name[0])
			continue;

		if (! currentMenu->menuitems[i].image)
			currentMenu->menuitems[i].image = W_ImageFromPatch(
				currentMenu->menuitems[i].patch_name);
    
		image = currentMenu->menuitems[i].image;

		VCTX_ImageEasy320(x, y, image);
	}

	// DRAW SKULL
	{
		int sx = x + SKULLXOFF;
		int sy = currentMenu->y - 5 + itemOn * LINEHEIGHT;

		VCTX_ImageEasy320(sx, sy, menu_skull[whichSkull]);
	}
}

//
// M_ClearMenus
//
void M_ClearMenus(void)
{
	menuactive = 0;
	save_screenshot_valid = false;
}

//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t * menudef)
{
	currentMenu = menudef;
	itemOn = currentMenu->lastOn;
}

//
// M_Ticker
//
void M_Ticker(void)
{
	if (optionsmenuon)
	{
		M_OptTicker();
		return;
	}

	if (--skullAnimCounter <= 0)
	{
		whichSkull ^= 1;
		skullAnimCounter = 8;
	}
}

//
// M_Init
//
bool M_Init(void)
{
	currentMenu = &MainDef;
	menuactive = 0;
	itemOn = currentMenu->lastOn;
	whichSkull = 0;
	skullAnimCounter = 10;
	screen_size = screenblocks - 3;
	messageToPrint = 0;
	messageString = NULL;
	messageLastMenuActive = menuactive;
	quickSaveSlot = -1;

	// lookup required images
	therm_l = W_ImageFromPatch("M_THERML");
	therm_m = W_ImageFromPatch("M_THERMM");
	therm_r = W_ImageFromPatch("M_THERMR");
	therm_o = W_ImageFromPatch("M_THERMO");

	menu_loadg    = W_ImageFromPatch("M_LOADG");
	menu_saveg    = W_ImageFromPatch("M_SAVEG");
	menu_svol     = W_ImageFromPatch("M_SVOL");
	menu_doom     = W_ImageFromPatch("M_DOOM");
	menu_newgame  = W_ImageFromPatch("M_NEWG");
	menu_skill    = W_ImageFromPatch("M_SKILL");
	menu_episode  = W_ImageFromPatch("M_EPISOD");
	menu_skull[0] = W_ImageFromPatch("M_SKULL1");
	menu_skull[1] = W_ImageFromPatch("M_SKULL2");

	// Here we could catch other version dependencies,
	//  like HELP1/2, and four episodes.
	//    if (W_CheckNumForName("M_EPI4") < 0)
	//      EpiDef.numitems -= 2;
	//    else if (W_CheckNumForName("M_EPI5") < 0)
	//      EpiDef.numitems--;

	if (W_CheckNumForName("HELP") >= 0)
		menu_readthis[0] = W_ImageFromPatch("HELP");
	else
		menu_readthis[0] = W_ImageFromPatch("HELP1");

	if (W_CheckNumForName("HELP2") >= 0)
		menu_readthis[1] = W_ImageFromPatch("HELP2");
	else
	{
		menu_readthis[1] = W_ImageFromPatch("CREDIT");

		// This is used because DOOM 2 had only one HELP
		//  page. I use CREDIT as second page now, but
		//  kept this hack for educational purposes.

		MainMenu[readthis] = MainMenu[quitdoom];
		MainDef.numitems--;
		MainDef.y += 8;
		NewDef.prevMenu = &MainDef;
		ReadDef1.draw_func = M_DrawReadThis1;
		ReadDef1.x = 330;
		ReadDef1.y = 165;
		ReadMenu1[0].select_func = M_FinishReadThis;
	}

	M_InitOptmenu();

	return true;
}

