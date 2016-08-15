/*! \file common.c
 *  \brief Common stuff that can be used by all gamestates.
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

#include <stdio.h>
#include <signal.h>
#include "common.h"
#include <libsuperderpy.h>

struct CommonResources* CreateGameData(struct Game *game) {
	return calloc(1, sizeof(struct CommonResources));
}

void DestroyGameData(struct Game *game, struct CommonResources *resources) {
	if (resources->music) al_destroy_audio_stream(resources->music);
	if (resources->button) al_destroy_sample_instance(resources->button);
	if (resources->button_sample) al_destroy_sample(resources->button_sample);
	free(resources);
}

void StartGame(struct Game *game, bool restart) {
	LoadGamestate(game, "intro");
	LoadGamestate(game, "fall");
	LoadGamestate(game, "catch");
	LoadGamestate(game, "fine");
	LoadGamestate(game, "notfine");
	LoadGamestate(game, "logo");
	LoadGamestate(game, "walk");
	StartGamestate(game, restart ? "walk" : "intro");
}
