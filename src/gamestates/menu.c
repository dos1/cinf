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
#include <stdio.h>
#include <math.h>
#include <allegro5/allegro_primitives.h>
#include "menu.h"

int Gamestate_ProgressCount = 1; // number of loading steps as reported by Gamestate_Load

void Gamestate_Logic(struct Game *game, struct MenuResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	data->blink++;
	if (data->blink >= 60) {
		data->blink = 0;
	}
	if (data->offset > 0) {
		data->offset--;
	}
}

void Gamestate_Draw(struct Game *game, struct MenuResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.

	int dy = data->offset;
	if (!game->data->touch) dy = 0;

	al_set_target_backbuffer(game->display);
	al_draw_filled_rectangle(0, 158 + dy, 320, 180, al_map_rgba(0,0,0,64));

	const char* texts[] = { "Play again", "Options", "Extras", "Quit",
	                        "Fullscreen: on", "Music: on", "Sounds: on", "Voice: on", "Back",
	                        "Fullscreen: off", "Music: off", "Sounds: off", "Voice: off", "Back",
	                        "Check out Chimpology", "Check out KARCZOCH", "Check out all SGJ16 games", "Back"
	                      };

	if (data->blink < 45) {
		if (game->data->touch) {
			DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 320/2, 165 + dy, ALLEGRO_ALIGN_CENTER, texts[data->option]);
			DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 10, 165 + dy, ALLEGRO_ALIGN_LEFT, "<");
			DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 310, 165 + dy, ALLEGRO_ALIGN_RIGHT, ">");
		} else {
			char text[255];
			snprintf(text, 255, "< %s >", texts[data->option]);
			DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 320/2, 165, ALLEGRO_ALIGN_CENTER, text);
		}
	}
}

void OpenBrowser(char* url) {
	char* cmd;
#ifdef ALLEGRO_WINDOWS
	cmd = "start \"\" \"%s\"";
#elif defined(ALLEGRO_MACOSX)
	cmd = "open \"%s\"";
#elif defined(ALLEGRO_ANDROID)
	cmd = "am start --user 0 -a android.intent.action.VIEW -d \"%s\"";
#else
	cmd = "xdg-open \"%s\"";
#endif
	char command[255];
	snprintf(command, 255, cmd, url);
	system(command);
}

void MenuSelect(struct Game *game, struct MenuResources* data) {
	al_stop_sample_instance(game->data->button);
	al_play_sample_instance(game->data->button);
	data->blink = 0;
	switch (data->option) {
		case 0:
			UnloadAllGamestates(game);
			StartGame(game, !game->data->logo);
			break;
		case 1:
			data->option = 4;
			break;
		case 2:
			data->option = 14;
			break;
		case 3:
			UnloadAllGamestates(game);
			break;
		case 4:
		case 9:
			// fullscreen
			game->config.fullscreen = !game->config.fullscreen;
			if (game->config.fullscreen) {
				SetConfigOption(game, "SuperDerpy", "fullscreen", "1");
				al_hide_mouse_cursor(game->display);
			} else {
				SetConfigOption(game, "SuperDerpy", "fullscreen", "0");
				al_show_mouse_cursor(game->display);
			}
			al_set_display_flag(game->display, ALLEGRO_FULLSCREEN_WINDOW, game->config.fullscreen);
			SetupViewport(game, game->viewport_config);
			PrintConsole(game, "Fullscreen toggled");
			break;
		case 5:
		case 10:
			// music
			game->config.music = game->config.music ? 0 : 10;
			SetConfigOption(game, "SuperDerpy", "music", game->config.music ? "10" : "0");
			al_set_mixer_gain(game->audio.music, game->config.music/10.0);
			break;
		case 6:
		case 11:
			// sounds
			game->config.fx = game->config.fx ? 0 : 10;
			SetConfigOption(game, "SuperDerpy", "fx", game->config.fx ? "10" : "0");
			al_set_mixer_gain(game->audio.fx, game->config.fx/10.0);
			break;
		case 7:
		case 12:
			// voices
			game->config.voice = game->config.voice ? 0 : 10;
			SetConfigOption(game, "SuperDerpy", "voice", game->config.voice ? "10" : "0");
			al_set_mixer_gain(game->audio.voice, game->config.voice/10.0);
			break;
		case 14:
			// chimpology
			OpenBrowser("https://mgz.itch.io/chimpology");
			UnloadAllGamestates(game);
			break;
		case 15:
			// karczoch
			OpenBrowser("https://dosowisko.net/karczoch/");
			UnloadAllGamestates(game);
			break;
		case 16:
			// sgj16
			OpenBrowser("https://itch.io/jam/sgj16");
			UnloadAllGamestates(game);
			break;
		case 8:
		case 13:
		case 17:
			data->option = 0;
			break;
	}
}

void MenuLeft(struct Game *game, struct MenuResources* data) {
	al_stop_sample_instance(game->data->button);
	al_play_sample_instance(game->data->button);
	data->blink = 0;
	data->option--;
	if (data->option==13) {
		data->option = 17;
	}
	if (data->option==8) {
		data->option = 13;
	}
#ifdef ALLEGRO_ANDROID
	if (data->option==9) {
		data->option = 13;
	}
#endif
	if (data->option==3) {
		data->option = 8;
	}
#ifdef ALLEGRO_ANDROID
	if (data->option==4) {
		data->option = 8;
	}
#endif
	if (data->option==-1) {
		data->option = 3;
	}
}

void MenuRight(struct Game *game, struct MenuResources* data) {
	al_stop_sample_instance(game->data->button);
	al_play_sample_instance(game->data->button);
	data->blink = 0;
	data->option++;
	if (data->option==4) {
		data->option = 0;
	}
	if (data->option==9) {
		data->option=4;
#ifdef ALLEGRO_ANDROID
		  data->option++;
#endif
	}
	if (data->option==14) {
		data->option=9;
#ifdef ALLEGRO_ANDROID
		  data->option++;
#endif
	}
	if (data->option==18) {
		data->option=14;
	}
}

void MenuEscape(struct Game *game, struct MenuResources* data) {
	if (data->option >= 4) {
		al_stop_sample_instance(game->data->button);
		al_play_sample_instance(game->data->button);
		data->blink = 0;
		data->option = 0;
	} else {
		UnloadAllGamestates(game);
	}
}

void Gamestate_ProcessEvent(struct Game *game, struct MenuResources* data, ALLEGRO_EVENT *ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		MenuEscape(game, data);
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_BACK)) {
		MenuEscape(game, data);
	}
	if (ev->type==ALLEGRO_EVENT_KEY_DOWN) {
		if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT) {
			MenuLeft(game, data);
		}
		if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT) {
			MenuRight(game, data);
		}

		if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER) {
			MenuSelect(game, data);
		}
	}

	if (ev->type==ALLEGRO_EVENT_TOUCH_BEGIN) {
		if ((ev->touch.primary) && (data->offset == 0)) {
			int x = ev->touch.x;
			int y = ev->touch.y;
			WindowCoordsToViewport(game, &x, &y);

			if ((y >= 158) && (y <= 180)) {
				if ((x >= 0) && (x < 40)) {
					MenuLeft(game, data);
				} else if ((x > 280) && (x <= 320)) {
					MenuRight(game, data);
				} else {
					MenuSelect(game, data);
				}
			}
		}
	}

	switch (data->option) {
		case 4:
			if (!game->config.fullscreen) {
				data->option += 5;
			}
#ifdef ALLEGRO_ANDROID
			data->option++;
#endif
			break;
		case 5:
			if (!game->config.music) {
				data->option += 5;
			}
			break;
		case 6:
			if (!game->config.fx) {
				data->option += 5;
			}
			break;
		case 7:
			if (!game->config.voice) {
				data->option += 5;
			}
			break;
		case 9:
			if (game->config.fullscreen) {
				data->option -= 5;
			}
#ifdef ALLEGRO_ANDROID
			data->option++;
#endif
			break;
		case 10:
			if (game->config.music) {
				data->option -= 5;
			}
			break;
		case 11:
			if (game->config.fx) {
				data->option -= 5;
			}
			break;
		case 12:
			if (game->config.voice) {
				data->option -= 5;
			}
			break;
	}
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct MenuResources *data = malloc(sizeof(struct MenuResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar

	return data;
}

void Gamestate_Unload(struct Game *game, struct MenuResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	free(data);
}

void Gamestate_Start(struct Game *game, struct MenuResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	data->option = 0;
	data->blink = 0;
	data->offset = 30;
#ifdef ALLEGRO_ANDROID
	game->data->touch = true;
#endif
}

void Gamestate_Stop(struct Game *game, struct MenuResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
}

// Ignore those for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game *game, struct MenuResources* data) {}
void Gamestate_Pause(struct Game *game, struct MenuResources* data) {}
void Gamestate_Resume(struct Game *game, struct MenuResources* data) {}
