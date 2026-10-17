#include "arcade.h"
const char * const game_names[GAME_COUNT] = {
    "STAR SKIMMER",
    "REACTOR DROP",
    "RICOCHET WORKS",
    "GRAVITY RALLY",
    "TIDE RIDER",
    "PULSE SERPENT",
    "ORBIT SALVAGE",
    "LUNAR COURIER"
};
const char * const game_descriptions[GAME_COUNT] = {
    "SALVAGE + UPGRADES",
    "PRESSURE + COOLANT",
    "GRAVITY + MAGNETS",
    "SPIN + FORCE FIELDS",
    "CURRENTS + RESCUES",
    "PHASE + TAIL RISKS",
    "GRAVITY + TOW LINES",
    "CARGO + MOON ROUTES"
};
const uint8_t game_save_slots[GAME_COUNT] = {0,8,2,10,6,1,13,15};
