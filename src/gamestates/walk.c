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
#include "walk.h"

int Gamestate_ProgressCount = 67; // number of loading steps as reported by Gamestate_Load

bool Move(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = action->arguments->value;
	if (state == TM_ACTIONSTATE_RUNNING) {
		data->offset+=0.09;
	}
	return false;
}

bool Skew(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = action->arguments->value;
	if (state == TM_ACTIONSTATE_RUNNING) {
		if (data->skew < 0) {
			data->skew -= data->level;
		} else {
			data->skew += data->level;
		}
		data->level += 0.000015;
		data->points++;
	}
	return false;
}

bool ZoomOut(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = action->arguments->value;
	if (state == TM_ACTIONSTATE_RUNNING) {
		data->zoom+=0.01;
		if (data->zoom >= 2) {
			return true;
		}
	}
	return false;
}

bool ShowMeter(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = action->arguments->value;
	if (state == TM_ACTIONSTATE_START) {
		data->skew = 0;
	}
	if (state == TM_ACTIONSTATE_RUNNING) {
		data->meteroffset+=1;
		if (data->meteroffset >= 0) {
			return true;
		}
	}
	return false;
}

bool ShowMaks(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = action->arguments->value;
	if (state == TM_ACTIONSTATE_RUNNING) {
		SetCharacterPosition(game, data->maks, 12, 80, 0);
	}
	return true;
}


void Gamestate_Logic(struct Game *game, struct WalkResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	AnimateCharacter(game, data->maks, 1);
	TM_Process(data->timeline);

	if (fabs(data->skew) >= 1) {
		SwitchGamestate(game, "walk", "fall");
	}
}

inline int min(int a, int b) {
	return (a > b) ? b : a;
}
inline int max(int a, int b) {
	return (a < b) ? b : a;
}

void Gamestate_Draw(struct Game *game, struct WalkResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.

	al_set_target_bitmap(data->area);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	al_draw_bitmap(data->bg, 0, 0, 0);
	al_draw_bitmap(data->sits, 117, 88, 0);

	float spacing = 10, x = 117, y = 88;
	int i = 0;

	while (y < 180) {
		for (int j=0; j<8; j++) {
			SetCharacterPosition(game, data->people[i*8+j], x+40*j + 9 - *((int*)data->people[i*8+j]->data) + 2, y-18, 0);
			DrawCharacter(game, data->people[i*8+j], al_map_rgb(255,255,255), 0);
		}

		al_draw_bitmap(data->sits, (int)x, (int)y, 0);
		x -= spacing;
		y += spacing;
		spacing += 0.5;

		i++;
	}

	al_set_target_bitmap(data->pixelator);
	al_draw_bitmap(data->area, 0, 0, 0);
	al_draw_scaled_bitmap(data->area, 0, 0, 320, 180, -(int)data->offset, -(180*(data->zoom-1)) + (int)data->offset, 320*data->zoom, 180*data->zoom, 0);
	DrawCharacter(game, data->maks, al_map_rgb(255,255,255), 0);

	al_set_target_bitmap(data->m);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	al_draw_bitmap(data->meter, 11, 6, 0);
	al_draw_filled_rectangle(11 + 4, 6 + 7, 309 - 4, 25 - 7, al_map_rgb(0,0,0));
	al_draw_bitmap(data->marker, (309 - 4 - (11+4)) / 2 + 11 + 4 - 5, 6 + 2, 0);

	al_draw_filled_rectangle((309 - 4 - (11+4)) / 2 + 11 + 4 + min(0, ((309 - 4 - (11+4)) / 2) * data->skew),
	                         6 + 7 + 1,
	                         (309 - 4 - (11+4)) / 2 + 11 + 4 + max(0, ((309 - 4 - (11+4)) / 2) * data->skew),
	                         25 - 7 - 1,
	                         al_map_rgb(255,0,0));

	al_set_target_bitmap(data->pixelator);
	al_draw_bitmap(data->m, 0, data->meteroffset, 0);

	al_set_target_backbuffer(game->display);
	al_draw_bitmap(data->pixelator, 0, 0, 0);
}

void Gamestate_ProcessEvent(struct Game *game, struct WalkResources* data, ALLEGRO_EVENT *ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	TM_HandleEvent(data->timeline, ev);
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadGamestate(game, "walk"); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)) {
		data->skew -= 0.1;
		if (data->skew < -1) data->skew = -1;
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)) {
		data->skew += 0.1;
		if (data->skew > 1) data->skew = 1;
	}
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct WalkResources *data = malloc(sizeof(struct WalkResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar
	data->maks = CreateCharacter(game, "maks");
	RegisterSpritesheet(game, data->maks, "walk");
	LoadSpritesheets(game, data->maks);
	progress(game);

	data->person = CreateCharacter(game, "person");
	RegisterSpritesheet(game, data->person, "dos");
	LoadSpritesheets(game, data->person);
	progress(game);

	for (int i=0; i<64; i++) {
		data->people[i] = CreateCharacter(game, "person");
		data->people[i]->spritesheets = data->person->spritesheets;
		data->people[i]->data = malloc(sizeof(int));
		*((int*)data->people[i]->data) = rand() % 5;
		SelectSpritesheet(game, data->people[i], "dos");
		progress(game);
	}

	data->bg = al_load_bitmap(GetDataFilePath(game, "bg.png"));
	data->sits = al_load_bitmap(GetDataFilePath(game, "sits.png"));
	data->meter = al_load_bitmap(GetDataFilePath(game, "meter.png"));
	data->marker = al_load_bitmap(GetDataFilePath(game, "marker.png"));
	data->area = al_create_bitmap(640, 360);
	data->m = al_create_bitmap(320, 180);
	data->pixelator = al_create_bitmap(320, 180);

	data->timeline = TM_Init(game, "timeline");
	return data;
}

void Gamestate_Unload(struct Game *game, struct WalkResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	free(data);
}

void Gamestate_Start(struct Game *game, struct WalkResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	SelectSpritesheet(game, data->maks, "walk");
	SetCharacterPosition(game, data->maks, -120, 80, 0);
	data->offset = 0;
	data->skew = 0;
	data->level = 0.00001;
	data->points = 0;
	data->zoom = 1;
	data->meteroffset = -25;
	TM_AddDelay(data->timeline, 1000);
	TM_AddQueuedBackgroundAction(data->timeline, ShowMeter, TM_AddToArgs(NULL, 1, data), 1000, "showmeter");
	TM_AddAction(data->timeline, ZoomOut, TM_AddToArgs(NULL, 1, data), "zoom");
	TM_AddQueuedBackgroundAction(data->timeline, ShowMaks, TM_AddToArgs(NULL, 1, data), 0, "showmaks");
	TM_AddQueuedBackgroundAction(data->timeline, Move, TM_AddToArgs(NULL, 1, data), 0, "move");
	TM_AddQueuedBackgroundAction(data->timeline, Skew, TM_AddToArgs(NULL, 1, data), 0, "skew");
}

void Gamestate_Stop(struct Game *game, struct WalkResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game *game, struct WalkResources* data) {}
void Gamestate_Pause(struct Game *game, struct WalkResources* data) {}
void Gamestate_Resume(struct Game *game, struct WalkResources* data) {}
