#define LIBSUPERDERPY_DATA_TYPE struct CommonResources
#include <libsuperderpy.h>

struct CommonResources {
	ALLEGRO_AUDIO_STREAM* music;
	ALLEGRO_SAMPLE* button_sample;
	ALLEGRO_SAMPLE_INSTANCE* button;
	int score;
	bool logo;
	bool touch;
};

struct CommonResources* CreateGameData(struct Game* game);
void DestroyGameData(struct Game* game);
bool GlobalEventHandler(struct Game* game, ALLEGRO_EVENT* event);
void StartGame(struct Game* game, bool restart);
