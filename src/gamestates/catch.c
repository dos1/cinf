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
#include "catch.h"

int Gamestate_ProgressCount = 2; // number of loading steps as reported by Gamestate_Load

void Gamestate_Logic(struct Game *game, struct CatchResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	AnimateCharacter(game, data->bg, 1);
	AnimateCharacter(game, data->glow, 1);
	data->pos++;
	if (data->pos % 64 == 0) {
		al_stop_sample_instance(data->sound);
		al_play_sample_instance(data->sound);
	}
	if (data->pos >= 288) {
		data->pos = 287;
		SwitchGamestate(game, "catch", "notfine");
	}
	//PrintConsole(game, "pos: %f, hand: %f, minus: %f", (float)data->pos, 300 + data->hand->x, 300 + data->hand->x - data->pos);

	if (300 + data->hand->x - data->pos > 10) {
		SwitchGamestate(game, "catch", "fine");
	}
}

void Gamestate_Draw(struct Game *game, struct CatchResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.

	al_set_target_backbuffer(game->display);
	DrawCharacter(game, data->bg, al_map_rgb(255,255,255), 0);

	int i = data->pos / 64 + 1;
	float remainder = (data->pos / 64.0) - (i - 1);
	al_draw_tinted_bitmap(data->dell[i-1], al_map_rgba_f(1.0-remainder,1.0-remainder,1.0-remainder,1.0-remainder), 0, 0, 0);
	al_draw_bitmap(data->dell[i], 0, 0, 0);

	DrawCharacter(game, data->hand, al_map_rgb(255,255,255), 0);

	DrawCharacter(game, data->glow, al_map_rgb(255,255,255), 0);
	DrawCharacter(game, data->glow, al_map_rgb(255,255,255), 0);
	DrawCharacter(game, data->key, al_map_rgb(255,255,255), 0);

	//al_draw_filled_rectangle(0, 0, data->pos + 30, 180, al_map_rgba(128,128,128,128));

}

void Gamestate_ProcessEvent(struct Game *game, struct CatchResources* data, ALLEGRO_EVENT *ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		SwitchGamestate(game, "catch", "logo"); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}
	if (ev->type==ALLEGRO_EVENT_KEY_DOWN) {
		const char* keyname = al_keycode_to_name(ev->keyboard.keycode);
		PrintConsole(game, "Pressed: %s", keyname);
		if ((keyname[0] == data->ch) || (keyname[0] == data->ch + 'A' - 'a')) {
			al_stop_sample_instance(game->data->button);
			al_play_sample_instance(game->data->button);
			MoveCharacter(game, data->hand, 9, 0, 0);
			if (data->hand->x > 0) {
				SetCharacterPosition(game, data->hand, 0, 0, 0);
			}
			SelectSpritesheet(game, data->key, "pressed");
		}
		//PrintConsole(game, "%s", al_keycode_to_name(ev->keyboard.keycode));
	}
	if (ev->type==ALLEGRO_EVENT_KEY_UP) {
		const char* keyname = al_keycode_to_name(ev->keyboard.keycode);
		if (keyname[0] == data->ch) {
			SelectSpritesheet(game, data->key, "ready");
		}
	}
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct CatchResources *data = malloc(sizeof(struct CatchResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar
	data->bg = CreateCharacter(game, "bg");
	RegisterSpritesheet(game, data->bg, "bg");
	LoadSpritesheets(game, data->bg);
	progress(game);

	data->hand = CreateCharacter(game, "hand");
	RegisterSpritesheet(game, data->hand, "hand");
	LoadSpritesheets(game, data->hand);

	data->glow = CreateCharacter(game, "glow");
	RegisterSpritesheet(game, data->glow, "glow");
	LoadSpritesheets(game, data->glow);

	data->key = CreateCharacter(game, "key");
	RegisterSpritesheet(game, data->key, "ready");
	RegisterSpritesheet(game, data->key, "pressed");
	LoadSpritesheets(game, data->key);

	data->ch = 'a' + (rand() % ('z'-'a'));

	char text[2];
	text[0] = data->ch + 'A'-'a';
	text[1] = 0;

	al_set_target_bitmap(data->key->spritesheets->bitmap);
	al_draw_text(data->font, al_map_rgb(0,0,0), 19, 15, ALLEGRO_ALIGN_LEFT, text);
	al_set_target_bitmap(data->key->spritesheets->next->bitmap);
	al_draw_text(data->font, al_map_rgb(0,0,0), 17, 13, ALLEGRO_ALIGN_LEFT, text);

	data->dell[0] = al_load_bitmap(GetDataFilePath(game, "dell0.png"));
	data->dell[1] = al_load_bitmap(GetDataFilePath(game, "dell1.png"));
	data->dell[2] = al_load_bitmap(GetDataFilePath(game, "dell2.png"));
	data->dell[3] = al_load_bitmap(GetDataFilePath(game, "dell3.png"));
	data->dell[4] = al_load_bitmap(GetDataFilePath(game, "dell4.png"));
	data->dell[5] = al_load_bitmap(GetDataFilePath(game, "dell5.png"));

	data->sample = al_load_sample(GetDataFilePath(game, "bdzium.flac"));
	data->sound = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->sound, game->audio.fx);

	return data;
}

void Gamestate_Unload(struct Game *game, struct CatchResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	free(data);
}

void Gamestate_Start(struct Game *game, struct CatchResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	SelectSpritesheet(game, data->bg, "bg");
	SetCharacterPosition(game, data->bg, 0, 0, 0);
	SelectSpritesheet(game, data->hand, "hand");
	SetCharacterPosition(game, data->hand, -300, 0, 0);
	SelectSpritesheet(game, data->glow, "glow");
	SetCharacterPosition(game, data->glow, 0, 0, 0);
	SelectSpritesheet(game, data->key, "ready");
	SetCharacterPosition(game, data->key, 139, 136, 0);
	al_play_sample_instance(data->sound);
	data->pos = 0;
}

void Gamestate_Stop(struct Game *game, struct CatchResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game *game, struct CatchResources* data) {}
void Gamestate_Pause(struct Game *game, struct CatchResources* data) {}
void Gamestate_Resume(struct Game *game, struct CatchResources* data) {}
