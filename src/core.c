#include "arcade.h"

uint8_t keys, pressed, sound_on = 1;
uint16_t clock_frames, score, best[GAME_COUNT];
uint8_t selected_game, tiles[360], attrs[360];
static uint8_t old_tiles[360], old_attrs[360], pause_tiles[360], pause_attrs[360];
static uint8_t dirty_rows[18], dirty;
static uint8_t previous_keys;
static uint16_t random_state = 24653;

static const uint16_t palettes[] = {
    RGB(2,3,6), RGB(5,10,15), RGB(8,23,24), RGB(26,31,29),
    RGB(2,5,5), RGB(5,12,8), RGB(12,24,12), RGB(27,31,20),
    RGB(6,3,7), RGB(15,6,9), RGB(29,12,12), RGB(31,26,20),
    RGB(4,3,9), RGB(10,7,17), RGB(21,15,30), RGB(30,26,31),
    RGB(6,4,3), RGB(15,10,5), RGB(28,19,6), RGB(31,29,19),
    RGB(2,4,8), RGB(4,10,18), RGB(9,19,28), RGB(25,30,31),
    RGB(2,3,6), RGB(4,6,10), RGB(7,11,16), RGB(13,18,23),
    RGB(26,31,29), RGB(18,26,25), RGB(8,15,19), RGB(2,4,8)
};
typedef struct { uint16_t magic; uint8_t version, sound; uint16_t high[GAME_COUNT]; } Save;
#define SAVE ((volatile Save *)0xA000)

void save_settings(void) {
    uint8_t i;
    ENABLE_RAM;
    SWITCH_RAM(0);
    SAVE->magic = 0xF1A8;
    SAVE->version = 2;
    SAVE->sound = sound_on;
    for (i=0;i<GAME_COUNT;i++) SAVE->high[i] = best[i];
    DISABLE_RAM;
}
void load_settings(void) {
    uint8_t i;
    ENABLE_RAM;
    SWITCH_RAM(0);
    if (SAVE->magic == 0xF1A8 && SAVE->version == 2) {
        sound_on = SAVE->sound ? 1 : 0;
        for(i=0;i<GAME_COUNT;i++) best[i]=SAVE->high[i];
    }
    DISABLE_RAM;
}
void init_video(void) {
    DISPLAY_OFF;
    if (_cpu == CGB_TYPE) {
        cpu_fast();
        set_bkg_palette(0,8,palettes);
        set_sprite_palette(0,8,palettes);
    }
    BGP_REG=0xE4; OBP0_REG=0xE4;
    set_bkg_data(0,TILE_COUNT,graphics);
    set_sprite_data(0,TILE_COUNT,graphics);
    SPRITES_8x8;
    NR52_REG=0x80; NR50_REG=0x77; NR51_REG=0xFF;
    memset(old_tiles,255,sizeof(old_tiles));
    memset(old_attrs,255,sizeof(old_attrs));
    clear_screen(0); flush();
    SHOW_BKG; SHOW_SPRITES;
    DISPLAY_ON;
}
void clear_screen(uint8_t palette) {
    memset(tiles,32,sizeof(tiles));memset(attrs,palette,sizeof(attrs));memset(dirty_rows,1,18);dirty=1;
}
void cell(uint8_t x,uint8_t y,uint8_t tile,uint8_t palette) {
    uint16_t p;
    if(x>=20 || y>=18)return;
    p=(uint16_t)y*20+x;
    if(tiles[p]==tile && attrs[p]==palette)return;
    tiles[p]=tile;attrs[p]=palette;dirty_rows[y]=1;dirty=1;
}
void text(uint8_t x,uint8_t y,const char *s,uint8_t palette) {
    while(*s && x<20) cell(x++,y,(uint8_t)*s++,palette);
}
void number(uint8_t x,uint8_t y,uint16_t n,uint8_t width,uint8_t palette) {
    while(width) { width--; cell(x+width,y,'0'+(n%10),palette); n/=10; }
}
void box(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t palette) {
    uint8_t i,j;
    for(j=0;j<h;j++)for(i=0;i<w;i++)cell(x+i,y+j,32,palette);
}
void panel(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t p) {
    uint8_t i,right=x+w,bottom=y+h;
    right--;bottom--;
    box(x,y,w,h,p);
    cell(x,y,(uint8_t)T_PANEL,p);cell(right,y,(uint8_t)(T_PANEL+1U),p);
    cell(x,bottom,(uint8_t)(T_PANEL+2U),p);cell(right,bottom,(uint8_t)(T_PANEL+3U),p);
    for(i=1;i+1<w;i++){cell(x+i,y,(uint8_t)(T_PANEL+4U),p);cell(x+i,bottom,(uint8_t)(T_PANEL+5U),p);}
    for(i=1;i+1<h;i++){cell(x,y+i,(uint8_t)(T_PANEL+6U),p);cell(right,y+i,(uint8_t)(T_PANEL+7U),p);}
}
void flush(void) {
    uint8_t y,x,start,len; uint16_t p;
    if(!dirty)return;
    for(y=0;y<18;y++) {
        if(!dirty_rows[y])continue;
        dirty_rows[y]=0;
        x=0;
        while(x<20) {
            p=(uint16_t)y*20+x;
            if(tiles[p]==old_tiles[p] && attrs[p]==old_attrs[p]) {x++;continue;}
            start=x;
            do { x++; p++; } while(x<20 && (tiles[p]!=old_tiles[p] || attrs[p]!=old_attrs[p]));
            len=x-start; p=(uint16_t)y*20+start;
            VBK_REG=VBK_TILES; set_bkg_tiles(start,y,len,1,tiles+p);
            if(_cpu==CGB_TYPE) { VBK_REG=VBK_ATTRIBUTES; set_bkg_tiles(start,y,len,1,attrs+p); VBK_REG=VBK_TILES; }
            memcpy(old_tiles+p,tiles+p,len); memcpy(old_attrs+p,attrs+p,len);
        }
    }
    dirty=0;
}
void read_frame(void) {
    vsync(); clock_frames++; keys=joypad(); pressed=keys & ~previous_keys; previous_keys=keys;
}
void seed_random(uint16_t seed) { random_state = seed ? seed : 24653; }
uint8_t random8(uint8_t bound) {
    random_state ^= random_state<<7; random_state ^= random_state>>9; random_state ^= random_state<<8;
    return bound ? (uint8_t)(random_state%bound) : 0;
}
void sfx(uint8_t kind) {
    if(!sound_on)return;
    NR21_REG=0x80; NR22_REG=kind==2 ? 0xA3 : 0x72;
    NR23_REG=kind==0 ? 0xA0 : kind==1 ? 0xD0 : kind==2 ? 0x30 : 0xE0;
    NR24_REG=kind==2 ? 0xC4 : 0xC7;
}
void add_score(uint16_t n) { if(score>65535U-n)score=65535U; else score+=n; }
void record_score(uint8_t game) { if(score>best[game]) {best[game]=score;save_settings();} }
void hide_objects(void) { uint8_t i; for(i=0;i<40;i++)move_sprite(i,0,0); }
void object(uint8_t id,uint8_t tile,int16_t x,int16_t y,uint8_t palette) {
    set_sprite_tile(id,tile); set_sprite_prop(id,palette);
    if(x < -7 || x>159 || y < -7 || y>143)move_sprite(id,0,0);
    else move_sprite(id,(uint8_t)(x+8),(uint8_t)(y+16));
}
void hud(const char *label) {
    text(0,0,label,0); text(12,0,"S:",4);number(14,0,score,5,4);
    text(0,17,"START PAUSE",6);text(13,17,"SEL SFX",6);
}
uint8_t game_frame(void) {
    uint8_t resume=1;
    read_frame();
    if(pressed & J_SELECT) {sound_on^=1;save_settings();sfx(0);}
    if(!(pressed & J_START))return 1;
    memcpy(pause_tiles,tiles,360);memcpy(pause_attrs,attrs,360);
    HIDE_SPRITES;clear_screen(0);
    text(6,4,"PAUSED",0);text(2,7,game_names[selected_game],4);
    text(3,10,"A / START RESUME",0);text(3,12,"B RETURN TO MENU",0);
    text(3,14,"SELECT SOUND",6);flush();
    while(1) {
        read_frame();
        if(pressed&(J_A|J_START))break;
        if(pressed&J_B) {resume=0;record_score(selected_game);break;}
        if(pressed&J_SELECT) {sound_on^=1;save_settings();sfx(0);}
    }
    memcpy(tiles,pause_tiles,360);memcpy(attrs,pause_attrs,360);memset(dirty_rows,1,18);dirty=1;flush();SHOW_SPRITES;
    pressed=0;
    return resume;
}
uint8_t result_action(const char *title,const char *detail,const char *action) {
    record_score(selected_game);hide_objects();clear_screen(0);
    text(2,3,title,4);text(2,5,detail,0);
    text(4,8,"SCORE",6);number(10,8,score,5,0);
    text(4,10,"BEST",6);number(10,10,best[selected_game],5,4);
    text(3,14,action,0);text(3,16,"B ARCADE MENU",6);flush();sfx(2);
    while(1){read_frame();if(pressed&J_A)return 1;if(pressed&J_B)return 0;}
}
uint8_t result(const char *title,const char *detail) { return result_action(title,detail,"A PLAY AGAIN"); }
