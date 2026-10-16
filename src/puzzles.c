#pragma bank 17
#include "arcade.h"
uint8_t choose_puzzle(uint8_t game,uint8_t current,uint8_t count) BANKED {
    uint8_t redraw=1;
    hide_objects();
    while(1) {
        if(redraw) {
            clear_screen(0);text(2,2,game_names[game],4);text(3,4,"CHOOSE PUZZLE",0);
            number(6,7,current+1,2,0);cell(9,7,'/',6);number(11,7,count,2,0);
            text(2,10,"LEFT/RIGHT CHOOSE",6);text(2,12,"EVERY LEVEL OPEN",6);
            text(2,16,"A PLAY   B MENU",0);flush();redraw=0;
        }
        read_frame();
        if(pressed&J_LEFT){current=current?current-1:count-1;redraw=1;sfx(0);}
        if(pressed&J_RIGHT){current=(current+1)%count;redraw=1;sfx(0);}
        if(pressed&J_A)return current;
        if(pressed&J_B)return 255;
    }
}
