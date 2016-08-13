#define LIBSUPERDERPY_DATA_TYPE struct CommonResources
#include <libsuperderpy.h>

struct CommonResources {
		ALLEGRO_AUDIO_STREAM *music;
		ALLEGRO_SAMPLE *button_sample;
		ALLEGRO_SAMPLE_INSTANCE *button;
		int score;
		bool logo;
};

void StartGame(struct Game *game, bool restart);
