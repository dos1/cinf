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
#include <allegro5/allegro_primitives.h>
#include "walk.h"

const int MAKS = 64-16;

int Gamestate_ProgressCount = 72; // number of loading steps as reported by Gamestate_Load

bool Move(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = TM_GetArg(action->arguments, 0);
	if (state == TM_ACTIONSTATE_RUNNING) {
		data->offset+=0.09;
	}
	return false;
}

bool Skew(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = TM_GetArg(action->arguments, 0);
	if (state == TM_ACTIONSTATE_START) {
		data->started = true;
	}
	if (state == TM_ACTIONSTATE_RUNNING) {
		if (data->skew < 0) {
			data->skew -= data->level;
		} else {
			data->skew += data->level;
		}
		data->level += 0.000015;
		game->data->score++;
	}
	return false;
}

bool ZoomOut(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = TM_GetArg(action->arguments, 0);
	if (state == TM_ACTIONSTATE_RUNNING) {
		data->zoom+=0.01;
		if (data->zoom >= 2) {
			return true;
		}
	}
	return false;
}

bool ShowMeter(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = TM_GetArg(action->arguments, 0);
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
	struct WalkResources *data = TM_GetArg(action->arguments, 0);
	if (state == TM_ACTIONSTATE_RUNNING) {
		SetCharacterPosition(game, data->maks, 16, 82, 0);
		SetCharacterPosition(game, data->people[MAKS], -100, -100, 0);
	}
	return true;
}

bool PrepMaks(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = TM_GetArg(action->arguments, 0);
	if (state == TM_ACTIONSTATE_START) {
		SelectSpritesheet(game, data->people[MAKS], "maks-prep");
		MoveCharacter(game, data->people[MAKS], -2, -5, 0);
	}
	return true;
}

bool MovePrepingMaks(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct WalkResources *data = TM_GetArg(action->arguments, 0);
	int *pos;
	if (state == TM_ACTIONSTATE_INIT) {
		pos = malloc(sizeof(int));
		action->arguments = TM_AddToArgs(action->arguments, 1, pos);
		*pos = 0;
	} else {
		pos = TM_GetArg(action->arguments, 1);
	}
	if (state == TM_ACTIONSTATE_RUNNING) {
		(*pos)++;
		if (*pos == 10) {
			*pos = 0;
			MoveCharacter(game, data->people[MAKS], -3, 0, 0);

			if (GetCharacterX(game, data->people[MAKS]) <= 5) {
				return true;
			}
		}
	}
	if (state == TM_ACTIONSTATE_DESTROY) {
		free(pos);
	}
	return false;
}


void Gamestate_Logic(struct Game *game, struct WalkResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	AnimateCharacter(game, data->maks, 1);
	AnimateCharacter(game, data->person, 1);
	TM_Process(data->timeline);

	if (fabs(data->skew) >= 1) {
		SwitchCurrentGamestate(game, "fall");
	}
}

void Gamestate_Draw(struct Game *game, struct WalkResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.

	al_set_target_bitmap(data->pixelator);
	al_draw_scaled_bitmap(data->bg, 0, 0, 320, 180, -(int)data->offset, -(180*(data->zoom-1)) + (int)data->offset, 320*data->zoom, 180*data->zoom, 0);

	DrawCharacter(game, data->maks, al_map_rgb(255,255,255), 0);

	al_set_target_bitmap(data->area);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	DrawCharacter(game, data->person, al_map_rgb(255,255,255), 0);

	float spacing = 10, x = 117, y = 88;
	int i = 0;

	while (y < 180) {
		for (int j=0; j<8; j++) {
			DrawCharacter(game, data->people[i*8+j], al_map_rgb(255,255,255), 0);
		}

		al_draw_bitmap(data->sits, (int)x, (int)y, 0);
		x -= spacing;
		y += spacing;
		spacing += 0.5;

		i++;
	}

	al_set_target_bitmap(data->m);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	al_draw_bitmap(data->meter, 11, 6, 0);
	al_draw_filled_rectangle(11 + 4, 6 + 7, 309 - 4, 25 - 7, al_map_rgb(0,0,0));
	al_draw_bitmap(data->marker, (309 - 4 - (11+4)) / 2 + 11 + 4 - 5, 6 + 2, 0);

	al_draw_filled_rectangle((309 - 4 - (11+4)) / 2 + 11 + 4 + fmin(0, ((309 - 4 - (11+4)) / 2) * data->skew),
	                         6 + 7 + 1,
	                         (309 - 4 - (11+4)) / 2 + 11 + 4 + fmax(0, ((309 - 4 - (11+4)) / 2) * data->skew),
	                         25 - 7 - 1,
	                         al_map_rgb(255,0,0));

	DrawCharacter(game, data->leftkey, al_map_rgb(255,255,255), 0);
	DrawCharacter(game, data->rightkey, al_map_rgb(255,255,255), 0);

	al_set_target_bitmap(data->pixelator);
	al_draw_scaled_bitmap(data->area, 0, 0, 320, 180, -(int)data->offset, -(180*(data->zoom-1)) + (int)data->offset, 320*data->zoom, 180*data->zoom, 0);
	al_draw_bitmap(data->m, 0, data->meteroffset, 0);

	al_set_target_backbuffer(game->display);
	al_draw_bitmap(data->pixelator, 0, 0, 0);
}

void Gamestate_ProcessEvent(struct Game *game, struct WalkResources* data, ALLEGRO_EVENT *ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	TM_HandleEvent(data->timeline, ev);
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		SwitchCurrentGamestate(game, "logo"); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_BACK)) {
		SwitchCurrentGamestate(game, "logo"); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}
	if (!data->started) return;
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)) {
		SelectSpritesheet(game, data->leftkey, "pressed");
		data->skew -= 0.1;
		if (data->skew < -1) data->skew = -1;
		al_stop_sample_instance(game->data->button);
		al_play_sample_instance(game->data->button);
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)) {
		SelectSpritesheet(game, data->rightkey, "pressed");
		data->skew += 0.1;
		if (data->skew > 1) data->skew = 1;
		al_stop_sample_instance(game->data->button);
		al_play_sample_instance(game->data->button);
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)) {
		SelectSpritesheet(game, data->leftkey, "ready");
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)) {
		SelectSpritesheet(game, data->rightkey, "ready");
	}
	if (ev->type==ALLEGRO_EVENT_TOUCH_BEGIN) {
		int x = ev->touch.x, y = ev->touch.y;
		WindowCoordsToViewport(game, &x, &y);
		if (IsOnCharacter(game, data->leftkey, x, y)) {
			SelectSpritesheet(game, data->leftkey, "pressed");
			data->skew -= 0.1;
			if (data->skew < -1) data->skew = -1;
			al_stop_sample_instance(game->data->button);
			al_play_sample_instance(game->data->button);
		} else if (IsOnCharacter(game, data->rightkey, x, y)) {
			SelectSpritesheet(game, data->rightkey, "pressed");
			data->skew += 0.1;
			if (data->skew > 1) data->skew = 1;
			al_stop_sample_instance(game->data->button);
			al_play_sample_instance(game->data->button);
		}
	}
	if ((ev->type==ALLEGRO_EVENT_TOUCH_CANCEL) || (ev->type==ALLEGRO_EVENT_TOUCH_END)) {
		SelectSpritesheet(game, (ev->touch.x < (al_get_display_width(game->display)/2)) ? data->leftkey : data->rightkey, "ready");
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
	char* sprites[] = { "dorota", "dos", "green", "jagoda", "jukio", "maciej", "dalton",
	                    "random1", "random2", "random3",
	                    "random4", "random5", "random6",
	                    "random8", "random7", "random9",
	                    "raxter", "shark", "sos", "threef", "tyr", "vera", "szyszka"};
	for (int i = 0; i < sizeof(sprites)/sizeof(sprites[0]); i++) {
		RegisterSpritesheet(game, data->person, sprites[i]);
	}
	RegisterSpritesheet(game, data->person, "maks");
	RegisterSpritesheet(game, data->person, "maks-prep");
	RegisterSpritesheet(game, data->person, "kacpi");
	LoadSpritesheets(game, data->person);
	progress(game);

	for (int i=0; i<64; i++) {
		data->people[i] = CreateCharacter(game, "person");
		data->people[i]->spritesheets = data->person->spritesheets;
		data->people[i]->data = malloc(sizeof(int));
		*((int*)data->people[i]->data) = rand() % 5;
		SelectSpritesheet(game, data->people[i], sprites[rand() % (sizeof(sprites)/sizeof(sprites[0]))]);
		progress(game);
	}
	SelectSpritesheet(game, data->people[MAKS], "maks");

	data->bg = al_load_bitmap(GetDataFilePath(game, "bg.png"));
	data->sits = al_load_bitmap(GetDataFilePath(game, "sits.png"));
	data->meter = al_load_bitmap(GetDataFilePath(game, "meter.png"));
	data->marker = al_load_bitmap(GetDataFilePath(game, "marker.png"));
	data->area = al_create_bitmap(640, 360);
	data->m = al_create_bitmap(320, 180);
	data->pixelator = al_create_bitmap(320, 180);
	progress(game);

	data->leftkey = CreateCharacter(game, "key");
	RegisterSpritesheet(game, data->leftkey, "ready");
	RegisterSpritesheet(game, data->leftkey, "pressed");
	LoadSpritesheets(game, data->leftkey);
	progress(game);

	al_set_target_bitmap(data->leftkey->spritesheets->bitmap);
	al_draw_text(data->font, al_map_rgb(0,0,0), 18, 15, ALLEGRO_ALIGN_LEFT, "<");
	al_draw_text(data->font, al_map_rgb(0,0,0), 19, 15, ALLEGRO_ALIGN_LEFT, "-");
	al_set_target_bitmap(data->leftkey->spritesheets->next->bitmap);
	al_draw_text(data->font, al_map_rgb(0,0,0), 16, 13, ALLEGRO_ALIGN_LEFT, "<");
	al_draw_text(data->font, al_map_rgb(0,0,0), 17, 13, ALLEGRO_ALIGN_LEFT, "-");
	progress(game);

	data->rightkey = CreateCharacter(game, "key");
	RegisterSpritesheet(game, data->rightkey, "ready");
	RegisterSpritesheet(game, data->rightkey, "pressed");
	LoadSpritesheets(game, data->rightkey);
	progress(game);

	al_set_target_bitmap(data->rightkey->spritesheets->bitmap);
	al_draw_text(data->font, al_map_rgb(0,0,0), 21, 15, ALLEGRO_ALIGN_LEFT, ">");
	al_draw_text(data->font, al_map_rgb(0,0,0), 18, 15, ALLEGRO_ALIGN_LEFT, "-");
	al_set_target_bitmap(data->rightkey->spritesheets->next->bitmap);
	al_draw_text(data->font, al_map_rgb(0,0,0), 19, 13, ALLEGRO_ALIGN_LEFT, ">");
	al_draw_text(data->font, al_map_rgb(0,0,0), 16, 13, ALLEGRO_ALIGN_LEFT, "-");
	progress(game);

	data->sample = al_load_sample(GetDataFilePath(game, "chimpology.flac"));
	data->chimpology = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->chimpology, game->audio.voice);

	data->timeline = TM_Init(game, "timeline");
	return data;
}

void Gamestate_Unload(struct Game *game, struct WalkResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	DestroyCharacter(game, data->maks);
	for (int i=0; i<64; i++) {
		data->people[i]->spritesheets = NULL; // shared with data->person, so free only once
		free(data->people[i]->data);
		DestroyCharacter(game, data->people[i]);
	}
	DestroyCharacter(game, data->person);
	DestroyCharacter(game, data->leftkey);
	DestroyCharacter(game, data->rightkey);
	al_destroy_bitmap(data->bg);
	al_destroy_bitmap(data->sits);
	al_destroy_bitmap(data->area);
	al_destroy_bitmap(data->meter);
	al_destroy_bitmap(data->marker);
	al_destroy_bitmap(data->pixelator);
	al_destroy_bitmap(data->m);
	TM_Destroy(data->timeline);
	al_destroy_sample_instance(data->chimpology);
	al_destroy_sample(data->sample);
	free(data);
}

void Gamestate_Start(struct Game *game, struct WalkResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	SelectSpritesheet(game, data->maks, "walk");
	SetCharacterPosition(game, data->maks, -120, 80, 0);
	SetCharacterPosition(game, data->leftkey, 9, 28, 0);
	SetCharacterPosition(game, data->rightkey, 320-2-48, 28, 0);
	SelectSpritesheet(game, data->leftkey, "ready");
	SelectSpritesheet(game, data->rightkey, "ready");
	SelectSpritesheet(game, data->person, "kacpi");
	SetCharacterPosition(game, data->person, 173, 3, 0);
	data->offset = 0;
	data->skew = 0;
	data->level = 0.00001;
	game->data->score = 0;
	data->zoom = 1;
	data->started = false;
	data->meteroffset = -100;
	al_play_sample_instance(data->chimpology);
	TM_AddDelay(data->timeline, 2000);
	TM_AddQueuedBackgroundAction(data->timeline, ShowMeter, TM_AddToArgs(NULL, 1, data), 2000, "showmeter");
	TM_AddQueuedBackgroundAction(data->timeline, ZoomOut, TM_AddToArgs(NULL, 1, data), 1000, "zoom");
	TM_AddAction(data->timeline, PrepMaks, TM_AddToArgs(NULL, 1, data), "prepmaks");
	TM_AddDelay(data->timeline, 500);
	TM_AddAction(data->timeline, MovePrepingMaks, TM_AddToArgs(NULL, 1, data), "moveprepingmaks");
	TM_AddQueuedBackgroundAction(data->timeline, ShowMaks, TM_AddToArgs(NULL, 1, data), 0, "showmaks");
	TM_AddQueuedBackgroundAction(data->timeline, Move, TM_AddToArgs(NULL, 1, data), 0, "move");
	TM_AddQueuedBackgroundAction(data->timeline, Skew, TM_AddToArgs(NULL, 1, data), 0, "skew");

	al_set_audio_stream_playing(game->data->music, true);

	float spacing = 10, x = 117, y = 88;
	int i = 0;

	while (y < 180) {
		for (int j=0; j<8; j++) {
			SetCharacterPosition(game, data->people[i*8+j], x+40*j + 9 - *((int*)data->people[i*8+j]->data) + 2, y-18, 0);
		}

		x -= spacing;
		y += spacing;
		spacing += 0.5;

		i++;
	}
}

void Gamestate_Stop(struct Game *game, struct WalkResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	game->data->score -= 366;
	if (game->data->score < 0) {
		game->data->score = 0;
	}
	al_stop_sample_instance(data->chimpology);
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game *game, struct WalkResources* data) {}
void Gamestate_Pause(struct Game *game, struct WalkResources* data) {}
void Gamestate_Resume(struct Game *game, struct WalkResources* data) {}
