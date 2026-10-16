#pragma bank 16
#include "arcade.h"
static uint8_t terrain[20],pad,width,fuel;
static int16_t fx,fy;
static int8_t vx,vy;
static void landscape(uint16_t level) {
    uint8_t x,y;
    width=level<3?5:level<6?4:3;pad=level==1?7:3+random8(10);if(pad+width>19)pad=19-width;
    clear_screen(0);hide_objects();
    for(y=3;y<14;y++)for(x=0;x<20;x++)if(random8(18)==0)cell(x,y,T_DOT,6);
    for(x=0;x<20;x++) {
        terrain[x]=x>=pad&&x<pad+width?15:15+random8(2);
        for(y=terrain[x];y<17;y++)cell(x,y,x>=pad&&x<pad+width?T_WALL:T_ROCK,x>=pad&&x<pad+width?1:6);
    }
    text(pad,16,"PAD",1);
}
void lunar_lander(void) BANKED {
    uint16_t level=1,frame;uint8_t landed,ended,thrust,replay=1;int16_t px,py;int8_t vertical,horizontal;
    score=0;
    while(replay) {
        fx=76*16;fy=24*16;vx=vy=0;fuel=100;frame=0;landed=ended=0;landscape(level);
        while(!ended) {
            if(!game_frame())return;frame++;thrust=0;
            if(!(frame&3)&&vy<64)vy++;
            if(fuel) {
                if((keys&(J_A|J_UP))&&vy>-32){vy--;thrust=1;}
                if((keys&J_LEFT)&&vx>-24){vx--;thrust=1;}
                if((keys&J_RIGHT)&&vx<24){vx++;thrust=1;}
                if(thrust&&!(frame&3))fuel--;
            }
            fx+=vx;fy+=vy;
            if(fx<0){fx=0;vx=0;}if(fx>152*16){fx=152*16;vx=0;}if(fy<16*16){fy=16*16;if(vy<0)vy=0;}
            px=fx/16;py=fy/16;vertical=vy<0?-vy:vy;horizontal=vx<0?-vx:vx;
            if(py+8>=terrain[(px+4)/8]*8) {
                ended=1;landed=px>=pad*8&&px+8<=(pad+width)*8&&vertical<=15&&horizontal<=12;
            }
            object(0,T_SHIP,px,py,5);object(1,T_BULLET,px+1,thrust&&(frame&2)?py+7:160,2);
            hud("LANDER");text(0,1,"FUEL",6);number(5,1,fuel,3,fuel>20?1:2);text(10,1,vertical<=15&&horizontal<=12?"SLOW":"FAST",vertical<=15&&horizontal<=12?1:2);
            text(15,1,"NO",6);number(18,1,level,2,4);flush();
        }
        if(landed)add_score(500+fuel*2);
        replay=result_action(landed?"TOUCHDOWN!":"A ROUGH LANDING",landed?"WELCOME TO THE MOON":"BRAKE BEFORE THE PAD",landed?"A NEXT LANDING":"A TRY AGAIN");
        if(landed)level++;else {level=1;score=0;}
    }
}
