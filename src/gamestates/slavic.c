/*! \file dosowisko.c
 *  \brief Init animation with dosowisko.net logo.
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
#include <allegro5/allegro_ttf.h>
#include <libsuperderpy.h>
#include <math.h>

struct GamestateResources {
	ALLEGRO_BITMAP* slavic;
	struct Timeline* timeline;
	ALLEGRO_SAMPLE* sample;
	ALLEGRO_SAMPLE_INSTANCE* sound;
};

int Gamestate_ProgressCount = 1;

//==================================Timeline manager actions BEGIN

static TM_ACTION(End) {
	if (action->state == TM_ACTIONSTATE_RUNNING) {
		UnloadAllGamestates(game);
		StartGame(game, false);
	}
	return true;
}

//==================================Timeline manager actions END

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	TM_Process(data->timeline, delta);
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	al_draw_bitmap(data->slavic, 0, 0, 0);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	TM_AddDelay(data->timeline, 3);
	TM_AddAction(data->timeline, End, NULL);
	al_play_sample_instance(data->sound);
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	if (((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) || (ev->type == ALLEGRO_EVENT_TOUCH_END)) {
		UnloadAllGamestates(game);
		StartGame(game, false);
	}
}

void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	struct GamestateResources* data = malloc(sizeof(struct GamestateResources));
	al_set_new_bitmap_flags(al_get_new_bitmap_flags() ^ ALLEGRO_MAG_LINEAR);

	data->timeline = TM_Init(game, data, "main");
	data->slavic = al_load_bitmap(GetDataFilePath(game, "slavic.png"));
	(*progress)(game);

	data->sample = al_load_sample(GetDataFilePath(game, "slavic.flac"));
	data->sound = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->sound, game->audio.music);

	return data;
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	TM_Destroy(data->timeline);
	al_destroy_sample_instance(data->sound);
	al_destroy_sample(data->sample);
	al_destroy_bitmap(data->slavic);
	free(data);
}

void Gamestate_Reload(struct Game* game, struct GamestateResources* data) {}

void Gamestate_Pause(struct Game* game, struct GamestateResources* data) {}
void Gamestate_Resume(struct Game* game, struct GamestateResources* data) {}
