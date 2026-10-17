#ifndef ARCADE_H
#define ARCADE_H
#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include <string.h>

#include "catalog.h"
#define T_STAR 96
#define T_SHIP 97
#define T_ALIEN 98
#define T_BULLET 99
#define T_BALL 100
#define T_PADDLE 101
#define T_APPLE 102
#define T_HEAD 103
#define T_BODY 104
#define T_BRICK 105
#define T_WALL 106
#define T_WATER 107
#define T_GEM 108
#define T_FLAG 109
#define T_MINE 110
#define T_CURSOR 111
#define T_HIDDEN 112
#define T_LAMP 113
#define T_DARK 114
#define T_ROCK 115
#define T_GATE 116
#define T_LIFE 117
#define T_LINE 118
#define T_DOT 119
#define T_LEFT 120
#define T_RIGHT 121
#define T_FILL 122
#define T_COVER 123
#define T_NUMBER 127
#define T_BIGFLAG 163
#define T_BIGMINE 167
#define T_BIGCURSOR 171
#define T_PANEL 175
#define T_CRATE 183
#define T_DISC 187
#define T_PERSON 191
#define TILE_COUNT 192

extern uint8_t keys, pressed, sound_on;
#define SAVE_SLOT_COUNT 16
extern uint16_t clock_frames, score, best[SAVE_SLOT_COUNT];
extern uint8_t selected_game, tiles[360], attrs[360];
extern const unsigned char graphics[];
extern const char * const game_names[GAME_COUNT];
extern const char * const game_descriptions[GAME_COUNT];
extern const uint8_t game_save_slots[GAME_COUNT];

void init_video(void);
void clear_screen(uint8_t palette);
void cell(uint8_t x, uint8_t y, uint8_t tile, uint8_t palette);
void text(uint8_t x, uint8_t y, const char *s, uint8_t palette);
void number(uint8_t x, uint8_t y, uint16_t n, uint8_t width, uint8_t palette);
void box(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t palette);
void panel(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t palette);
void flush(void);
void read_frame(void);
uint8_t game_frame(void);
uint8_t random8(uint8_t bound);
void seed_random(uint16_t seed);
void sfx(uint8_t kind);
void add_score(uint16_t n);
void save_settings(void);
void load_settings(void);
void record_score(uint8_t game);
uint16_t best_score(uint8_t game);
uint8_t choice(const char *title,const char *detail,const char *left,const char *right);
void hud(const char *label);
void hide_objects(void);
void object(uint8_t id, uint8_t tile, int16_t x, int16_t y, uint8_t palette);
uint8_t result(const char *title, const char *detail);
uint8_t result_action(const char *title, const char *detail, const char *action);
uint8_t intro(uint8_t game) BANKED;

void star_skimmer(void) BANKED;
void block_drop(void) BANKED;
void brick_flight(void) BANKED;
void orbit_rally(void) BANKED;
void river_run(void) BANKED;
void orchard_snake(void) BANKED;
void orbit_salvage(void) BANKED;
void lunar_lander(void) BANKED;
#endif
