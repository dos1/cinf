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

#include <allegro5/allegro_primitives.h>
#include <libsuperderpy.h>
#include <math.h>

struct GamestateResources {
	// This struct is for every resource allocated and used by your gamestate.
	// It gets created on load and then gets passed around to all other function calls.
	struct Character* maks;
	ALLEGRO_SAMPLE* sample;
	ALLEGRO_SAMPLE_INSTANCE* sound;
};

int Gamestate_ProgressCount = 4; // number of loading steps as reported by Gamestate_Load

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	double delta = 1 / 60.0;
	AnimateCharacter(game, data->maks, delta, 1);

	if (!data->maks->successor) {
		SwitchCurrentGamestate(game, "catch");
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.

	DrawCharacter(game, data->maks);
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		SwitchCurrentGamestate(game, "logo");
		// When there are no active gamestates, the engine will quit.
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_BACK)) {
		SwitchCurrentGamestate(game, "logo");
		// When there are no active gamestates, the engine will quit.
	}
}

void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct GamestateResources* data = malloc(sizeof(struct GamestateResources));
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar
	data->maks = CreateCharacter(game, "fall");
	RegisterSpritesheet(game, data->maks, "fall");
	RegisterSpritesheet(game, data->maks, "blank");
	LoadSpritesheets(game, data->maks, progress);
	progress(game);

	data->sample = al_load_sample(GetDataFilePath(game, "fall.flac"));
	data->sound = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->sound, game->audio.fx);

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	DestroyCharacter(game, data->maks);
	al_destroy_sample_instance(data->sound);
	al_destroy_sample(data->sample);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	SelectSpritesheet(game, data->maks, "fall");
	SetCharacterPosition(game, data->maks, 0, 0, 0);
	al_play_sample_instance(data->sound);
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	al_stop_sample_instance(data->sound);
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game* game, struct GamestateResources* data) {}
void Gamestate_Pause(struct Game* game, struct GamestateResources* data) {}
void Gamestate_Resume(struct Game* game, struct GamestateResources* data) {}
