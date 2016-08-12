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

#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <libsuperderpy.h>
#include "slavic.h"

int Gamestate_ProgressCount = 1;

//==================================Timeline manager actions BEGIN


bool End(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	if (state == TM_ACTIONSTATE_RUNNING) {
		SwitchGamestate(game, "slavic", "walk");
		LoadGamestate(game, "fall");
	}
	return true;
}

//==================================Timeline manager actions END


void Gamestate_Logic(struct Game *game, struct SlavicResources* data) {
	TM_Process(data->timeline);
}

void Gamestate_Draw(struct Game *game, struct SlavicResources* data) {
	al_draw_bitmap(data->slavic, 0, 0, 0);
}

void Gamestate_Start(struct Game *game, struct SlavicResources* data) {
	TM_AddDelay(data->timeline, 3000);
	TM_AddAction(data->timeline, End, NULL, "end");
//	al_play_sample_instance(data->sound);
}

void Gamestate_ProcessEvent(struct Game *game, struct SlavicResources* data, ALLEGRO_EVENT *ev) {
	TM_HandleEvent(data->timeline, ev);
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		SwitchGamestate(game, "slavic", "walk");
		LoadGamestate(game, "fall");
	}
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
	struct SlavicResources *data = malloc(sizeof(struct SlavicResources));
	data->timeline = TM_Init(game, "main");
	data->slavic = al_load_bitmap(GetDataFilePath(game, "slavic.png"));
	(*progress)(game);

	return data;
}

void Gamestate_Stop(struct Game *game, struct SlavicResources* data) {
}

void Gamestate_Unload(struct Game *game, struct SlavicResources* data) {
	TM_Destroy(data->timeline);
	free(data);
}

void Gamestate_Reload(struct Game *game, struct SlavicResources* data) {}

void Gamestate_Pause(struct Game *game, struct SlavicResources* data) {
	TM_Pause(data->timeline);
}
void Gamestate_Resume(struct Game *game, struct SlavicResources* data) {
	TM_Resume(data->timeline);
}
