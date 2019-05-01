/*! \file walk.c
 *  \brief Walking gamestate.
 */
/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "../common.h"
#include <allegro5/allegro_primitives.h>
#include <libsuperderpy.h>
#include <math.h>

struct GamestateResources {
	// This struct is for every resource allocated and used by your gamestate.
	// It gets created on load and then gets passed around to all other function calls.
	ALLEGRO_FONT* font;
	ALLEGRO_BITMAP *bitmap, *bg;
	float pos;
};

int Gamestate_ProgressCount = 2; // number of loading steps as reported by Gamestate_Load

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	data->pos += 0.1;
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.
	al_draw_bitmap(data->bg, 0, 0, 0);
	al_draw_bitmap(data->bitmap, 112, 29 + (int)(10 * sin(data->pos)), 0);
	DrawTextWithShadow(data->font, al_map_rgb(255, 255, 255), 320 / 2, 124, ALLEGRO_ALIGN_CENTER, "by dos");
	DrawTextWithShadow(data->font, al_map_rgb(255, 255, 255), 320 / 2, 140, ALLEGRO_ALIGN_CENTER, "Based on a real story!");
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
}

void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct GamestateResources* data = malloc(sizeof(struct GamestateResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar
	data->bitmap = al_load_bitmap(GetDataFilePath(game, "logo.png"));
	data->bg = al_load_bitmap(GetDataFilePath(game, "logobg.png"));
	progress(game);

	LoadGamestate(game, "menu");
	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	al_destroy_bitmap(data->bitmap);
	al_destroy_bitmap(data->bg);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	game->data->logo = true;
	data->pos = 0;
	StartGamestate(game, "menu");
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	game->data->logo = false;
	StopGamestate(game, "menu");
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game* game, struct GamestateResources* data) {}
void Gamestate_Pause(struct Game* game, struct GamestateResources* data) {}
void Gamestate_Resume(struct Game* game, struct GamestateResources* data) {}
