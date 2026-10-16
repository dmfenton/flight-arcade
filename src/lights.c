#pragma bank 6
#include "arcade.h"
static uint8_t lights[25];
static void flip(uint8_t p) {
    lights[p]^=1;if(p%5)lights[p-1]^=1;if(p%5<4)lights[p+1]^=1;if(p>=5)lights[p-5]^=1;if(p<20)lights[p+5]^=1;
}
static uint8_t lit(void){uint8_t i,n=0;for(i=0;i<25;i++)n+=lights[i];return n;}
static void puzzle(void) {uint8_t i;do {memset(lights,0,25);for(i=0;i<12;i++)flip(random8(25));}while(!lit());}
static void render(uint16_t moves,uint8_t level) {
    uint8_t i,x,y,p;
    for(i=0;i<25;i++) {
        x=2+(i%5)*3;y=2+(i/5)*3;p=lights[i]?7:6;
        panel(x,y,3,3,p);
        cell(x+1,y+1,lights[i]?T_LAMP:T_DARK,p);
    }
    hud("LIGHTS");text(0,1,"MOVES",6);number(6,1,moves,3,0);text(11,1,"NO.",6);number(15,1,level,3,4);
}
void lights_out(void) BANKED {
    uint8_t cx=2,cy=2,last=255,level=1,solved=0;uint16_t moves=0;
    score=0;clear_screen(0);hide_objects();puzzle();render(moves,level);
    while(1) {
        if(!game_frame())return;
        if((pressed&J_LEFT)&&cx)cx--;if((pressed&J_RIGHT)&&cx<4)cx++;
        if((pressed&J_UP)&&cy)cy--;if((pressed&J_DOWN)&&cy<4)cy++;
        if((pressed&J_B) && last!=255){flip(last);last=255;if(moves)moves--;solved=0;render(moves,level);sfx(0);}
        if(pressed&J_A) {
            if(solved){add_score(300-(moves>240?240:moves));record_score(selected_game);if(level<255)level++;moves=0;last=255;solved=0;puzzle();}
            else {last=cy*5+cx;flip(last);if(moves<999)moves++;solved=!lit();sfx(solved?3:0);}
            render(moves,level);
        }
        if(solved)text(0,1,"SOLVED! A NEXT",4);
        object(0,T_CURSOR,(3+cx*3)*8,(3+cy*3)*8,4);flush();
    }
}
