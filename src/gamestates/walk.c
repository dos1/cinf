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
	struct Character *maks, *people[64], *person, *leftkey, *rightkey;
	ALLEGRO_BITMAP *bg, *sits, *area, *meter, *marker, *pixelator, *audience;
	float offset, skew, level;
	struct Timeline* timeline;
	int meteroffset;
	float zoom;
	bool started;
	ALLEGRO_SAMPLE* sample;
	ALLEGRO_SAMPLE_INSTANCE* chimpology;
};

const int MAKS = 64 - 16;

int Gamestate_ProgressCount = 101; // number of loading steps as reported by Gamestate_Load

static TM_ACTION(Move) {
	if (action->state == TM_ACTIONSTATE_RUNNING) {
		data->offset += 0.09;
	}
	return false;
}

static TM_ACTION(Skew) {
	if (action->state == TM_ACTIONSTATE_START) {
		data->started = true;
	}
	if (action->state == TM_ACTIONSTATE_RUNNING) {
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

static TM_ACTION(ZoomOut) {
	if (action->state == TM_ACTIONSTATE_RUNNING) {
		data->zoom += 0.01;
		if (data->zoom >= 2) {
			return true;
		}
	}
	return false;
}

static TM_ACTION(ShowMeter) {
	if (action->state == TM_ACTIONSTATE_START) {
		data->skew = 0;
	}
	if (action->state == TM_ACTIONSTATE_RUNNING) {
		data->meteroffset += 1;
		if (data->meteroffset >= 0) {
			return true;
		}
	}
	return false;
}

static TM_ACTION(ShowMaks) {
	if (action->state == TM_ACTIONSTATE_RUNNING) {
		SetCharacterPosition(game, data->maks, 16, 82, 0);
		SetCharacterPosition(game, data->people[MAKS], -100, -100, 0);
	}
	return true;
}

static TM_ACTION(PrepMaks) {
	if (action->state == TM_ACTIONSTATE_START) {
		SelectSpritesheet(game, data->people[MAKS], "maks-prep");
		MoveCharacter(game, data->people[MAKS], -2, -5, 0);
	}
	return true;
}

static TM_ACTION(MovePrepingMaks) {
	int* pos;
	if (action->state == TM_ACTIONSTATE_INIT) {
		pos = malloc(sizeof(int));
		action->arguments = TM_AddToArgs(action->arguments, 1, pos);
		*pos = 0;
	} else {
		pos = TM_GetArg(action->arguments, 0);
	}
	if (action->state == TM_ACTIONSTATE_RUNNING) {
		(*pos)++;
		if (*pos == 10) {
			*pos = 0;
			MoveCharacter(game, data->people[MAKS], -3, 0, 0);

			if (GetCharacterX(game, data->people[MAKS]) <= 5) {
				return true;
			}
		}
	}
	if (action->state == TM_ACTIONSTATE_DESTROY) {
		free(pos);
	}
	return false;
}

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Called 60 times per second. Here you should do all your game logic.
	AnimateCharacter(game, data->maks, delta, 1);
	AnimateCharacter(game, data->person, delta, 1);

#ifdef ALLEGRO_ANDROID
	SetCharacterPosition(game, data->leftkey, GetCharacterX(game, data->leftkey), 180 - data->meteroffset - 42, 0);
	SetCharacterPosition(game, data->rightkey, GetCharacterX(game, data->rightkey), 180 - data->meteroffset - 42, 0);
#else
	SetCharacterPosition(game, data->leftkey, GetCharacterX(game, data->leftkey), data->meteroffset + 28, 0);
	SetCharacterPosition(game, data->rightkey, GetCharacterX(game, data->rightkey), data->meteroffset + 28, 0);
#endif

	TM_Process(data->timeline, delta);

	if (fabsf(data->skew) >= 1) {
		SwitchCurrentGamestate(game, "fall");
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.

	al_set_target_bitmap(data->pixelator);

	al_set_target_bitmap(data->area);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	DrawCharacter(game, data->person);

	float spacing = 10, x = 117, y = 88;
	int i = 0;

	while (y < 180) {
		for (int j = 0; j < 8; j++) {
			DrawCharacter(game, data->people[i * 8 + j]);
		}

		al_draw_bitmap(data->sits, (int)x, (int)y, 0);
		x -= spacing;
		y += spacing;
		spacing += 0.5;

		i++;
	}

	al_set_target_bitmap(data->pixelator);
	al_draw_scaled_bitmap(data->bg, 0, 0, 320, 180, -(int)data->offset, -(180 * (data->zoom - 1)) + (int)data->offset, 320 * data->zoom, 180 * data->zoom, 0);

	DrawCharacter(game, data->maks);

	al_draw_scaled_bitmap(data->area, 0, 0, 320, 180, -(int)data->offset, -(180 * (data->zoom - 1)) + (int)data->offset, 320 * data->zoom, 180 * data->zoom, 0);

	al_draw_bitmap(data->meter, 11, 6 + data->meteroffset, 0);
	al_draw_filled_rectangle(11 + 4, 6 + 7 + data->meteroffset, 309 - 4, 25 - 7 + data->meteroffset, al_map_rgb(0, 0, 0));
	al_draw_bitmap(data->marker, (309 - 4 - (11 + 4)) / 2 + 11 + 4 - 5, 6 + 2 + data->meteroffset, 0);

	al_draw_filled_rectangle((309 - 4 - (11 + 4)) / 2 + 11 + 4 + fmin(0, ((309 - 4 - (11 + 4)) / 2) * data->skew),
		6 + 7 + 1 + data->meteroffset,
		(309 - 4 - (11 + 4)) / 2 + 11 + 4 + fmax(0, ((309 - 4 - (11 + 4)) / 2) * data->skew),
		25 - 7 - 1 + data->meteroffset,
		al_map_rgb(255, 0, 0));

	DrawCharacter(game, data->leftkey);
	DrawCharacter(game, data->rightkey);

	if (strcmp(data->leftkey->spritesheet->name, "ready") != 0) {
		al_draw_text(data->font, al_map_rgb(0, 0, 0), GetCharacterX(game, data->leftkey) + 18, GetCharacterY(game, data->leftkey) + 15, ALLEGRO_ALIGN_LEFT, "<");
		al_draw_text(data->font, al_map_rgb(0, 0, 0), GetCharacterX(game, data->leftkey) + 19, GetCharacterY(game, data->leftkey) + 15, ALLEGRO_ALIGN_LEFT, "-");
	} else {
		al_draw_text(data->font, al_map_rgb(0, 0, 0), GetCharacterX(game, data->leftkey) + 16, GetCharacterY(game, data->leftkey) + 13, ALLEGRO_ALIGN_LEFT, "<");
		al_draw_text(data->font, al_map_rgb(0, 0, 0), GetCharacterX(game, data->leftkey) + 17, GetCharacterY(game, data->leftkey) + 13, ALLEGRO_ALIGN_LEFT, "-");
	}

	if (strcmp(data->rightkey->spritesheet->name, "ready") != 0) {
		al_draw_text(data->font, al_map_rgb(0, 0, 0), GetCharacterX(game, data->rightkey) + 21, GetCharacterY(game, data->rightkey) + 15, ALLEGRO_ALIGN_LEFT, ">");
		al_draw_text(data->font, al_map_rgb(0, 0, 0), GetCharacterX(game, data->rightkey) + 18, GetCharacterY(game, data->rightkey) + 15, ALLEGRO_ALIGN_LEFT, "-");
	} else {
		al_draw_text(data->font, al_map_rgb(0, 0, 0), GetCharacterX(game, data->rightkey) + 19, GetCharacterY(game, data->rightkey) + 13, ALLEGRO_ALIGN_LEFT, ">");
		al_draw_text(data->font, al_map_rgb(0, 0, 0), GetCharacterX(game, data->rightkey) + 16, GetCharacterY(game, data->rightkey) + 13, ALLEGRO_ALIGN_LEFT, "-");
	}

	SetFramebufferAsTarget(game);
	al_draw_bitmap(data->pixelator, 0, 0, 0);
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		SwitchCurrentGamestate(game, "logo"); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_BACK)) {
		SwitchCurrentGamestate(game, "logo"); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}
	if (!data->started) return;
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)) {
		SelectSpritesheet(game, data->leftkey, "pressed");
		data->skew -= 0.1;
		if (data->skew < -1) data->skew = -1;
		al_stop_sample_instance(game->data->button);
		al_play_sample_instance(game->data->button);
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)) {
		SelectSpritesheet(game, data->rightkey, "pressed");
		data->skew += 0.1;
		if (data->skew > 1) data->skew = 1;
		al_stop_sample_instance(game->data->button);
		al_play_sample_instance(game->data->button);
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)) {
		SelectSpritesheet(game, data->leftkey, "ready");
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)) {
		SelectSpritesheet(game, data->rightkey, "ready");
	}
	if (ev->type == ALLEGRO_EVENT_TOUCH_BEGIN) {
		int x = ev->touch.x, y = ev->touch.y;
		WindowCoordsToViewport(game, &x, &y);
		if (IsOnCharacter(game, data->leftkey, x, y, false)) {
			SelectSpritesheet(game, data->leftkey, "pressed");
			data->skew -= 0.1;
			if (data->skew < -1) data->skew = -1;
			al_stop_sample_instance(game->data->button);
			al_play_sample_instance(game->data->button);
		} else if (IsOnCharacter(game, data->rightkey, x, y, false)) {
			SelectSpritesheet(game, data->rightkey, "pressed");
			data->skew += 0.1;
			if (data->skew > 1) data->skew = 1;
			al_stop_sample_instance(game->data->button);
			al_play_sample_instance(game->data->button);
		}
	}
	if ((ev->type == ALLEGRO_EVENT_TOUCH_CANCEL) || (ev->type == ALLEGRO_EVENT_TOUCH_END)) {
		SelectSpritesheet(game, (ev->touch.x < (al_get_display_width(game->display) / 2)) ? data->leftkey : data->rightkey, "ready");
	}
}
void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct GamestateResources* data = malloc(sizeof(struct GamestateResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar
	data->maks = CreateCharacter(game, "maks");
	RegisterSpritesheet(game, data->maks, "walk");
	LoadSpritesheets(game, data->maks, progress);

	progress(game);

	data->person = CreateCharacter(game, "person");
	char* sprites[] = {"dorota", "dos", "green", "jagoda", "jukio", "maciej", "dalton",
		"random1", "random2", "random3",
		"random4", "random5", "random6",
		"random8", "random7", "random9",
		"raxter", "shark", "sos", "threef", "tyr", "vera", "szyszka"};
	for (int i = 0; i < sizeof(sprites) / sizeof(sprites[0]); i++) {
		RegisterSpritesheet(game, data->person, sprites[i]);
	}
	RegisterSpritesheet(game, data->person, "maks");
	RegisterSpritesheet(game, data->person, "maks-prep");
	RegisterSpritesheet(game, data->person, "kacpi");
	LoadSpritesheets(game, data->person, progress);
	progress(game);

	for (int i = 0; i < 64; i++) {
		data->people[i] = CreateCharacter(game, "person");
		data->people[i]->spritesheets = data->person->spritesheets;
		data->people[i]->data = malloc(sizeof(int));
		*((int*)data->people[i]->data) = rand() % 5;
		SelectSpritesheet(game, data->people[i], sprites[rand() % (sizeof(sprites) / sizeof(sprites[0]))]);
		progress(game);
	}
	SelectSpritesheet(game, data->people[MAKS], "maks");

	data->bg = al_load_bitmap(GetDataFilePath(game, "bg.png"));
	data->sits = al_load_bitmap(GetDataFilePath(game, "sits.png"));
	data->meter = al_load_bitmap(GetDataFilePath(game, "meter.png"));
	data->marker = al_load_bitmap(GetDataFilePath(game, "marker.png"));

	int flags = al_get_new_bitmap_flags();
	al_add_new_bitmap_flag(ALLEGRO_NO_PRESERVE_TEXTURE);
	data->area = al_create_bitmap(640, 360);
	data->pixelator = al_create_bitmap(320, 180);
	al_set_new_bitmap_flags(flags);

	progress(game);

	data->leftkey = CreateCharacter(game, "key");
	RegisterSpritesheet(game, data->leftkey, "ready");
	RegisterSpritesheet(game, data->leftkey, "pressed");
	LoadSpritesheets(game, data->leftkey, progress);
	progress(game);

	data->rightkey = CreateCharacter(game, "key");
	RegisterSpritesheet(game, data->rightkey, "ready");
	RegisterSpritesheet(game, data->rightkey, "pressed");
	LoadSpritesheets(game, data->rightkey, progress);
	progress(game);

	data->sample = al_load_sample(GetDataFilePath(game, "chimpology.flac"));
	data->chimpology = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->chimpology, game->audio.voice);

	data->timeline = TM_Init(game, data, "timeline");
	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	DestroyCharacter(game, data->maks);
	for (int i = 0; i < 64; i++) {
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
	TM_Destroy(data->timeline);
	al_destroy_sample_instance(data->chimpology);
	al_destroy_sample(data->sample);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	SelectSpritesheet(game, data->maks, "walk");
	SetCharacterPosition(game, data->maks, -120, 80, 0);
	SetCharacterPosition(game, data->leftkey, 9, -128, 0);
	SetCharacterPosition(game, data->rightkey, 320 - 2 - 48, -128, 0);
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
	TM_AddDelay(data->timeline, 2);
	TM_AddQueuedBackgroundAction(data->timeline, ShowMeter, NULL, 2);
	TM_AddQueuedBackgroundAction(data->timeline, ZoomOut, NULL, 1);
	TM_AddAction(data->timeline, PrepMaks, NULL);
	TM_AddDelay(data->timeline, 0.5);
	TM_AddAction(data->timeline, MovePrepingMaks, NULL);
	TM_AddQueuedBackgroundAction(data->timeline, ShowMaks, NULL, 0);
	TM_AddQueuedBackgroundAction(data->timeline, Move, NULL, 0);
	TM_AddQueuedBackgroundAction(data->timeline, Skew, NULL, 0);

	al_set_audio_stream_playing(game->data->music, true);

	float spacing = 10, x = 117, y = 88;
	int i = 0;

	while (y < 180) {
		for (int j = 0; j < 8; j++) {
			SetCharacterPosition(game, data->people[i * 8 + j], x + 40 * j + 9 - *((int*)data->people[i * 8 + j]->data) + 2, y - 18, 0);
		}

		x -= spacing;
		y += spacing;
		spacing += 0.5;

		i++;
	}
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	game->data->score -= 366;
	if (game->data->score < 0) {
		game->data->score = 0;
	}
	al_stop_sample_instance(data->chimpology);
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game* game, struct GamestateResources* data) {
	int flags = al_get_new_bitmap_flags();
	al_add_new_bitmap_flag(ALLEGRO_NO_PRESERVE_TEXTURE);
	data->area = al_create_bitmap(640, 360);
	data->pixelator = al_create_bitmap(320, 180);
	al_set_new_bitmap_flags(flags);
}
void Gamestate_Pause(struct Game* game, struct GamestateResources* data) {}
void Gamestate_Resume(struct Game* game, struct GamestateResources* data) {}
