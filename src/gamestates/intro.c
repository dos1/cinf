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
	struct Timeline* timeline;
	ALLEGRO_SAMPLE* sample;
	ALLEGRO_SAMPLE_INSTANCE* andnow;
};

int Gamestate_ProgressCount = 3; // number of loading steps as reported by Gamestate_Load

static TM_ACTION(Switch) {
	if (action->state == TM_ACTIONSTATE_START) {
		SwitchCurrentGamestate(game, "walk");
	}
	return true;
}

static TM_ACTION(PlayMusic) {
	if (action->state == TM_ACTIONSTATE_START) {
		al_set_audio_stream_playing(game->data->music, true);
	}
	return true;
}

static TM_ACTION(PlaySound) {
	if (action->state == TM_ACTIONSTATE_START) {
		al_play_sample_instance(data->andnow);
	}
	return true;
}

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Called 60 times per second. Here you should do all your game logic.
	TM_Process(data->timeline, delta);
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.
	al_draw_text(data->font, al_map_rgb(255, 255, 255), 15, 180 - 40, ALLEGRO_ALIGN_LEFT, "Slavic Game Jam");
	al_draw_text(data->font, al_map_rgb(255, 255, 255), 15, 180 - 30, ALLEGRO_ALIGN_LEFT, "CZIITT, Warsaw, Poland");
	al_draw_text(data->font, al_map_rgb(255, 255, 255), 15, 180 - 20, ALLEGRO_ALIGN_LEFT, "August 7, 2016");
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		SwitchCurrentGamestate(game, "walk");
	}
}

void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct GamestateResources* data = malloc(sizeof(struct GamestateResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar

	data->timeline = TM_Init(game, data, "timeline");

	if (!game->data->music) {
		game->data->music = al_load_audio_stream(GetDataFilePath(game, "music.flac"), 4, 1024);
		al_attach_audio_stream_to_mixer(game->data->music, game->audio.music);
		al_set_audio_stream_playmode(game->data->music, ALLEGRO_PLAYMODE_LOOP);
		al_set_audio_stream_playing(game->data->music, false);
	}
	progress(game);

	data->sample = al_load_sample(GetDataFilePath(game, "andnow.flac"));
	data->andnow = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->andnow, game->audio.voice);
	progress(game);

	if (!game->data->button) {
		game->data->button_sample = al_load_sample(GetDataFilePath(game, "button.flac"));
		game->data->button = al_create_sample_instance(game->data->button_sample);
		al_attach_sample_instance_to_mixer(game->data->button, game->audio.fx);
	}

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	TM_Destroy(data->timeline);
	al_destroy_sample_instance(data->andnow);
	al_destroy_sample(data->sample);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	TM_AddDelay(data->timeline, 1.4);
	TM_AddAction(data->timeline, PlayMusic, NULL);
	TM_AddDelay(data->timeline, 1);
	TM_AddAction(data->timeline, PlaySound, NULL);
	TM_AddDelay(data->timeline, 1.6);
	TM_AddAction(data->timeline, Switch, NULL);
	if (al_get_audio_stream_playing(game->data->music)) {
		al_set_audio_stream_playing(game->data->music, false);
		al_rewind_audio_stream(game->data->music);
	}
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	al_stop_sample_instance(data->andnow);
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game* game, struct GamestateResources* data) {}
void Gamestate_Pause(struct Game* game, struct GamestateResources* data) {}
void Gamestate_Resume(struct Game* game, struct GamestateResources* data) {}
