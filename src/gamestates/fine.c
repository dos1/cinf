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
#include <libsuperderpy.h>
#include <math.h>
#include <stdio.h>
#include <allegro5/allegro_primitives.h>
#include "fine.h"

int Gamestate_ProgressCount = 3; // number of loading steps as reported by Gamestate_Load

void Gamestate_Logic(struct Game *game, struct FineResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
}

void Gamestate_Draw(struct Game *game, struct FineResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.

	al_set_target_backbuffer(game->display);
	al_draw_bitmap(data->bitmap, 0, 0, 0);

	char score[255];
	snprintf(score, 255, "Score: %d", game->data->score * 100);

	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 320/2, 140, ALLEGRO_ALIGN_CENTER, "Computer is fine! :)");
	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 320/2, 7, ALLEGRO_ALIGN_CENTER, score);
}

void Gamestate_ProcessEvent(struct Game *game, struct FineResources* data, ALLEGRO_EVENT *ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.s
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct FineResources *data = malloc(sizeof(struct FineResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar
	data->bitmap = al_load_bitmap(GetDataFilePath(game, "fine.png"));
	progress(game);

	data->fine = al_load_audio_stream(GetDataFilePath(game, "cif.flac"), 4, 1024);
	al_set_audio_stream_playing(data->fine, false);
	al_attach_audio_stream_to_mixer(data->fine, game->audio.voice);
	progress(game);

	data->sample = al_load_sample(GetDataFilePath(game, "end.flac"));
	data->end = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->end, game->audio.fx);

	LoadGamestate(game, "menu");

	return data;
}

void Gamestate_Unload(struct Game *game, struct FineResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	al_destroy_bitmap(data->bitmap);
	al_destroy_audio_stream(data->fine);
	al_destroy_sample(data->sample);
	al_destroy_sample_instance(data->end);
	free(data);
}

void Gamestate_Start(struct Game *game, struct FineResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	al_set_audio_stream_playing(data->fine, true);
	al_play_sample_instance(data->end);
	StartGamestate(game, "menu");
}

void Gamestate_Stop(struct Game *game, struct FineResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	al_set_audio_stream_playing(data->fine, false);
	StopGamestate(game, "menu");
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game *game, struct FineResources* data) {}
void Gamestate_Pause(struct Game *game, struct FineResources* data) {}
void Gamestate_Resume(struct Game *game, struct FineResources* data) {}
