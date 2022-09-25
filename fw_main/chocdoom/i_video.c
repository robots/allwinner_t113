// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include "config.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_video.h"
#include "z_zone.h"

#include "tables.h"
#include "doomkeys.h"

#include "de.h"
#include "event.h"

#include <stdint.h>
#include <stdbool.h>

// The screen buffer; this is modified to draw things to the screen

byte *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = 2.0;
int mouse_threshold = 10;

// Gamma correction level to use

int usegamma = 0;

int usemouse = 0;

// If true, keyboard mapping is ignored, like in Vanilla Doom.
// The sensible thing to do is to disable this if you have a non-US
// keyboard.

int vanilla_keyboard_mapping = true;


typedef struct
{
	byte r;
	byte g;
	byte b;
} col_t;

// Palette converted to RGB888

static uint32_t rgb888_palette[256];

// Last button state

static bool last_button_state;

// run state

static bool run;

void I_InitGraphics (void)
{


	I_VideoBuffer = (byte*)Z_Malloc (SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

	screenvisible = true;
}

void I_ShutdownGraphics (void)
{
	Z_Free (I_VideoBuffer);
}

void I_StartFrame (void)
{

}

uint8_t tr[256] =
{
	[0x28] = KEY_ENTER,
	[0x29] = KEY_ESCAPE,
	[0x2a] = KEY_BACKSPACE,
	[0x2b] = KEY_TAB,
	[0x2c] = KEY_USE,
	[0x2d] = KEY_MINUS,
	[0x2e] = KEY_EQUALS,

	[0x3a] = KEY_F1,
	[0x3b] = KEY_F2,
	[0x3c] = KEY_F3,
	[0x3d] = KEY_F4,
	[0x3e] = KEY_F5,
	[0x3f] = KEY_F6,
	[0x40] = KEY_F7,
	[0x41] = KEY_F8,
	[0x42] = KEY_F9,
	[0x43] = KEY_F10,
	[0x44] = KEY_F11,
	[0x45] = KEY_F12,

	[0x48] = KEY_PAUSE,
	[0x49] = KEY_INS,
	[0x4f] = KEY_RIGHTARROW,
	[0x50] = KEY_LEFTARROW,
	[0x51] = KEY_DOWNARROW,
	[0x52] = KEY_UPARROW,

	['a'] = 'a',
	['c'] = 'c',
	['d'] = 'd',
	['f'] = 'f',
	['g'] = 'g',
	['h'] = 'h',
	['i'] = 'i',
	['k'] = 'k',
	['m'] = 'm',
	['n'] = 'n',
	['q'] = 'q',
	['w'] = 'w',
	['y'] = 'y',
	['z'] = 'z',

	['0'] = '0',
	['1'] = '1',
	['2'] = '2',
	['3'] = '3',
	['4'] = '4',
	['5'] = '5',
	['6'] = '6',
	['7'] = '7',
	['8'] = '8',
	['9'] = '9',

};

uint8_t tr_mod[8] = {
	[0] = KEY_FIRE,
	[1] = KEY_RSHIFT,
	[2] = KEY_RALT,
};

void I_GetEvent (void)
{
	kbd_event_t ev;
	event_t event;

	BaseType_t ret = xQueueReceive(kbd_queue, &ev, 0);

	if (ret == pdFALSE) return;

	event.type = ev.down ? ev_keydown: ev_keyup;
	event.data2 = -1;
	event.data3 = -1;

	if (ev.is_modifier) {
		uint8_t c = tr_mod[ev.code];
		if (c == 0) return;

		event.data1 = c;

	} else {
		uint8_t c = tr[ev.code];
		if (c == 0) return;

		event.data1 = c;
		if (ev.down && ev.ch) {
			event.data2 = ev.ch;
			event.data3 = ev.ch;
		}
	}

	D_PostEvent(&event);
}

void I_StartTic (void)
{
	I_GetEvent();
}

void I_UpdateNoBlit (void)
{
}

void I_FinishUpdate (void)
{
	int x, y;
	int xo, yo;
	byte index;

#define SCALE 2

	uint32_t *fb = (uint32_t *)de_layer_get_fb();
	uint32_t w = de_layer_get_w();
	uint32_t h = de_layer_get_h();


	for (y = 0, yo = 0; y < SCREENHEIGHT; y++, yo += SCALE)
	{
		for (x = 0, xo = 0; x < SCREENWIDTH; x++, xo += SCALE)
		{
			index = I_VideoBuffer[y * SCREENWIDTH + x];

			fb[(yo + 0) * w + (xo + 0)] = rgb888_palette[index];
			fb[(yo + 1) * w + (xo + 0)] = rgb888_palette[index];
			fb[(yo + 0) * w + (xo + 1)] = rgb888_palette[index];
			fb[(yo + 1) * w + (xo + 1)] = rgb888_palette[index];
		}
	}

}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
	int i;
	col_t* c;

	for (i = 0; i < 256; i++)
	{
		c = (col_t*)palette;

		rgb888_palette[i] = 
			0xff000000 | // alpha
			((gammatable[usegamma][c->r] & 0xff) << 16) |
			((gammatable[usegamma][c->g] & 0xff) << 8) |
			(gammatable[usegamma][c->b] & 0xff);

		palette += 3;
	}
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex (int r, int g, int b)
{
    int best, best_diff, diff;
    int i;
    col_t color;

    best = 0;
    best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
    	color.r = (rgb888_palette[i] >> 16) & 0xff;
    	color.g = (rgb888_palette[i] >> 8) & 0xff;
    	color.b = (rgb888_palette[i] & 0xff);

        diff = (r - color.r) * (r - color.r)
             + (g - color.g) * (g - color.g)
             + (b - color.b) * (b - color.b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}

void I_BeginRead (void)
{
}

void I_EndRead (void)
{
}

void I_SetWindowTitle (char *title)
{
}

void I_GraphicsCheckCommandLine (void)
{
}

void I_SetGrabMouseCallback (grabmouse_callback_t func)
{
}

void I_EnableLoadingDisk (void)
{
}

void I_BindVideoVariables (void)
{
}

void I_DisplayFPSDots (boolean dots_on)
{
}

void I_CheckIsScreensaver (void)
{
}
