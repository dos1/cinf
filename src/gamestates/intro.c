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

#include <libsuperderpy.h>
#include <math.h>
#include <allegro5/allegro_primitives.h>
#include "intro.h"

int Gamestate_ProgressCount = 2; // number of loading steps as reported by Gamestate_Load

bool Switch(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	if (state == TM_ACTIONSTATE_RUNNING) {
		SwitchGamestate(game, "intro", "walk");
	}
	return true;
}

void Gamestate_Logic(struct Game *game, struct IntroResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	TM_Process(data->timeline);
}

void Gamestate_Draw(struct Game *game, struct IntroResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.

	al_set_target_backbuffer(game->display);
	al_clear_to_color(al_map_rgb(0,0,0));

	al_draw_text(data->font, al_map_rgb(255,255,255), 15, 180-40, ALLEGRO_ALIGN_LEFT, "Slavic Game Jam");
	al_draw_text(data->font, al_map_rgb(255,255,255), 15, 180-30, ALLEGRO_ALIGN_LEFT, "CZIITT, Warsaw, Poland");
	al_draw_text(data->font, al_map_rgb(255,255,255), 15, 180-20, ALLEGRO_ALIGN_LEFT, "August 7, 2016");
}

void Gamestate_ProcessEvent(struct Game *game, struct IntroResources* data, ALLEGRO_EVENT *ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	TM_HandleEvent(data->timeline, ev);
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		SwitchGamestate(game, "intro", "walk");
	}
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct IntroResources *data = malloc(sizeof(struct IntroResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar
	data->bitmap = al_load_bitmap(GetDataFilePath(game, "fine.png"));
	progress(game);

	data->timeline = TM_Init(game, "timeline");

	return data;
}

void Gamestate_Unload(struct Game *game, struct IntroResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	free(data);
}

void Gamestate_Start(struct Game *game, struct IntroResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	TM_AddDelay(data->timeline, 3000);
	TM_AddAction(data->timeline, Switch, NULL, "switch");
}

void Gamestate_Stop(struct Game *game, struct IntroResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game *game, struct IntroResources* data) {}
void Gamestate_Pause(struct Game *game, struct IntroResources* data) {}
void Gamestate_Resume(struct Game *game, struct IntroResources* data) {}
