#include "arcade.h"
static void menu(void) {
    uint8_t i,index,redraw=1;
    hide_objects();
    while(1) {
        if(redraw) {
            clear_screen(0);text(3,1,"FLIGHT ARCADE",0);text(1,2,"8 WORLDS TO MASTER",6);
            for(i=0;i<8;i++) {
                index=i;
                if(index==selected_game) {box(1,4+i,18,1,7);cell(1,4+i,'>',7);}
                text(3,4+i,game_names[index],index==selected_game?7:0);
            }
            text(1,13,game_descriptions[selected_game],4);
            text(1,14,"BEST",6);number(6,14,best_score(selected_game),5,0);
            number(14,14,selected_game+1,2,0);cell(16,14,'/',6);number(17,14,GAME_COUNT,2,6);
            text(1,16,"A PLAY    B SOUND",0);text(1,17,sound_on?"SFX ON":"SFX OFF",6);
            flush();redraw=0;
        }
        read_frame();
        if(pressed&J_UP){selected_game=selected_game?selected_game-1:GAME_COUNT-1;redraw=1;sfx(0);}
        if(pressed&J_DOWN){selected_game=(selected_game+1)%GAME_COUNT;redraw=1;sfx(0);}
        if(pressed&J_B){sound_on^=1;save_settings();redraw=1;sfx(0);}
        if(pressed&(J_A|J_START)){seed_random(clock_frames^DIV_REG);return;}
    }
}
void main(void) {
    load_settings();init_video();
    while(1) {
        menu();if(!intro(selected_game))continue;score=0;
        switch(selected_game) {
        case 0:star_skimmer();break;case 1:block_drop();break;
        case 2:brick_flight();break;case 3:orbit_rally();break;
        case 4:river_run();break;case 5:orchard_snake();break;
        case 6:orbit_salvage();break;default:lunar_lander();break;
        }
    }
}
