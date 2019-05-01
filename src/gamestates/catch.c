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
	struct Character *bg, *hand, *glow, *key;
	ALLEGRO_BITMAP* dell[6];
	int pos;
	char ch;
	ALLEGRO_SAMPLE* sample;
	ALLEGRO_SAMPLE_INSTANCE* sound;

	int keyposx, keyposy;
};

int Gamestate_ProgressCount = 11; // number of loading steps as reported by Gamestate_Load

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	double delta = 1 / 60.0;
	AnimateCharacter(game, data->bg, delta, 1);
	AnimateCharacter(game, data->glow, delta, 1);
	data->pos++;
	if (data->pos % 64 == 0) {
		al_stop_sample_instance(data->sound);
		al_play_sample_instance(data->sound);
	}
	if (data->pos >= 288) {
		data->pos = 287;
		SwitchCurrentGamestate(game, "notfine");
	}
	//PrintConsole(game, "pos: %f, hand: %f, minus: %f", (float)data->pos, 300 + data->hand->x, 300 + data->hand->x - data->pos);

	if (300 + GetCharacterX(game, data->hand) - data->pos > 10) {
		SwitchCurrentGamestate(game, "fine");
	}

	if (game->data->touch) {
		SetCharacterPosition(game, data->glow, data->keyposx - 139, 0, 0);
		SetCharacterPosition(game, data->key, data->keyposx, 136, 0);
	} else {
		SetCharacterPosition(game, data->glow, 0, 0, 0);
		SetCharacterPosition(game, data->key, 139, 136, 0);
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.

	DrawCharacter(game, data->bg);

	int i = data->pos / 64 + 1;
	float remainder = (data->pos / 64.0) - (i - 1);
	al_draw_tinted_bitmap(data->dell[i - 1], al_map_rgba_f(1.0 - remainder, 1.0 - remainder, 1.0 - remainder, 1.0 - remainder), 0, 0, 0);
	al_draw_bitmap(data->dell[i], 0, 0, 0);

	DrawCharacter(game, data->hand);

	DrawCharacter(game, data->glow);
	DrawCharacter(game, data->glow);
	DrawCharacter(game, data->key);

#ifndef ALLEGRO_ANDROID
	char text[2];
	text[0] = data->ch + 'A' - 'a';
	text[1] = 0;
#else
	char text[] = "!";
#endif

	if (strcmp(data->key->spritesheet->name, "ready") != 0) {
		al_draw_text(data->font, al_map_rgb(0, 0, 0), GetCharacterX(game, data->key) + 19, GetCharacterY(game, data->key) + 15, ALLEGRO_ALIGN_LEFT, text);
	} else {
		al_draw_text(data->font, al_map_rgb(0, 0, 0), GetCharacterX(game, data->key) + 17, GetCharacterY(game, data->key) + 13, ALLEGRO_ALIGN_LEFT, text);
	}

	//al_draw_filled_rectangle(0, 0, data->pos + 30, 180, al_map_rgba(128,128,128,128));
}

void MoveHand(struct Game* game, struct GamestateResources* data) {
	al_stop_sample_instance(game->data->button);
	al_play_sample_instance(game->data->button);
	MoveCharacter(game, data->hand, 9, 0, 0);
	if (GetCharacterX(game, data->hand) > 0) {
		SetCharacterPosition(game, data->hand, 0, 0, 0);
	}
	SelectSpritesheet(game, data->key, "pressed");
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		SwitchCurrentGamestate(game, "logo"); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_BACK)) {
		SwitchCurrentGamestate(game, "logo");
		// When there are no active gamestates, the engine will quit.
	}
	if (ev->type == ALLEGRO_EVENT_KEY_DOWN) {
		const char* keyname = al_keycode_to_name(ev->keyboard.keycode);
		PrintConsole(game, "Pressed: %s", keyname);
		if ((keyname[0] == data->ch) || (keyname[0] == data->ch + 'A' - 'a')) {
			MoveHand(game, data);
		}
		//PrintConsole(game, "%s", al_keycode_to_name(ev->keyboard.keycode));
	}
	if (ev->type == ALLEGRO_EVENT_KEY_UP) {
		const char* keyname = al_keycode_to_name(ev->keyboard.keycode);
		if (keyname[0] == data->ch) {
			SelectSpritesheet(game, data->key, "ready");
		}
	}
	if (ev->type == ALLEGRO_EVENT_TOUCH_BEGIN) {
		if (ev->touch.primary) {
			int x = ev->touch.x, y = ev->touch.y;
			WindowCoordsToViewport(game, &x, &y);
			if (IsOnCharacter(game, data->key, x, y, false)) {
				MoveHand(game, data);
			}
		}
	}
	if ((ev->type == ALLEGRO_EVENT_TOUCH_CANCEL) || (ev->type == ALLEGRO_EVENT_TOUCH_END)) {
		SelectSpritesheet(game, data->key, "ready");
	}
}

void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct GamestateResources* data = malloc(sizeof(struct GamestateResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar
	data->bg = CreateCharacter(game, "bg");
	RegisterSpritesheet(game, data->bg, "bg");
	LoadSpritesheets(game, data->bg, progress);
	progress(game);

	data->hand = CreateCharacter(game, "hand");
	RegisterSpritesheet(game, data->hand, "hand");
	LoadSpritesheets(game, data->hand, progress);
	progress(game);

	data->glow = CreateCharacter(game, "glow");
	RegisterSpritesheet(game, data->glow, "glow");
	LoadSpritesheets(game, data->glow, progress);
	progress(game);

	data->key = CreateCharacter(game, "key");
	RegisterSpritesheet(game, data->key, "ready");
	RegisterSpritesheet(game, data->key, "pressed");
	LoadSpritesheets(game, data->key, progress);
	progress(game);

	data->ch = 'a' + (rand() % ('z' - 'a'));
	data->keyposx = rand() % (game->viewport.width - al_get_bitmap_width(data->key->spritesheets->bitmap));
	data->keyposy = game->viewport.height / 2 + rand() % (game->viewport.height / 2 - al_get_bitmap_height(data->key->spritesheets->bitmap));

	data->dell[0] = al_load_bitmap(GetDataFilePath(game, "dell0.png"));
	data->dell[1] = al_load_bitmap(GetDataFilePath(game, "dell1.png"));
	data->dell[2] = al_load_bitmap(GetDataFilePath(game, "dell2.png"));
	data->dell[3] = al_load_bitmap(GetDataFilePath(game, "dell3.png"));
	data->dell[4] = al_load_bitmap(GetDataFilePath(game, "dell4.png"));
	data->dell[5] = al_load_bitmap(GetDataFilePath(game, "dell5.png"));
	progress(game);

	data->sample = al_load_sample(GetDataFilePath(game, "bdzium.flac"));
	data->sound = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->sound, game->audio.fx);

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	DestroyCharacter(game, data->bg);
	DestroyCharacter(game, data->hand);
	DestroyCharacter(game, data->glow);
	DestroyCharacter(game, data->key);
	for (int i = 0; i < 6; i++) {
		al_destroy_bitmap(data->dell[i]);
	}
	al_destroy_sample_instance(data->sound);
	al_destroy_sample(data->sample);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	SelectSpritesheet(game, data->bg, "bg");
	SetCharacterPosition(game, data->bg, 0, 0, 0);
	SelectSpritesheet(game, data->hand, "hand");
	SetCharacterPosition(game, data->hand, -300, 0, 0);
	SelectSpritesheet(game, data->glow, "glow");
	SelectSpritesheet(game, data->key, "ready");
	if (game->data->touch) {
		SetCharacterPosition(game, data->glow, data->keyposx - 139, 0, 0);
		SetCharacterPosition(game, data->key, data->keyposx, 136, 0);
	} else {
		SetCharacterPosition(game, data->glow, 0, 0, 0);
		SetCharacterPosition(game, data->key, 139, 136, 0);
	}
	al_play_sample_instance(data->sound);
	data->pos = 0;
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game* game, struct GamestateResources* data) {}
void Gamestate_Pause(struct Game* game, struct GamestateResources* data) {}
void Gamestate_Resume(struct Game* game, struct GamestateResources* data) {}
