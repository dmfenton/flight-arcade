#pragma bank 2
#include "arcade.h"
static uint8_t xs[234],ys[234];
static uint8_t length,fx,fy;
static void fruit(void) {
    uint8_t i,free;
    do {fx=1+random8(18);fy=3+random8(13);free=1;for(i=0;i<length;i++)if(xs[i]==fx&&ys[i]==fy)free=0;}while(!free);
    cell(fx,fy,T_APPLE,2);
}
void orchard_snake(void) BANKED {
    uint8_t i,dir,next,delay,interval,dead,grow,win;
    uint8_t nx,ny;
    do {
        score=0;length=4;dir=next=0;delay=0;dead=win=0;
        hide_objects();clear_screen(1);
        for(i=0;i<20;i++){cell(i,2,T_WALL,1);cell(i,16,T_WALL,1);}
        for(i=3;i<16;i++){cell(0,i,T_WALL,1);cell(19,i,T_WALL,1);}
        for(i=0;i<4;i++){xs[i]=9-i;ys[i]=9;cell(xs[i],ys[i],i?T_BODY:T_HEAD,1);}
        fruit();
        while(!dead && !win) {
            if(!game_frame())return;
            if((pressed&J_RIGHT)&&dir!=2)next=0;if((pressed&J_DOWN)&&dir!=3)next=1;
            if((pressed&J_LEFT)&&dir!=0)next=2;if((pressed&J_UP)&&dir!=1)next=3;
            interval=keys&J_A?4:score>200?6:score>100?8:10;
            if(++delay>=interval) {
                delay=0;dir=next;nx=xs[0];ny=ys[0];
                if(dir==0)nx++;if(dir==1)ny++;if(dir==2)nx--;if(dir==3)ny--;
                grow=nx==fx && ny==fy;
                if(nx<1||nx>18||ny<3||ny>15)dead=1;
                for(i=0;i<length-(grow?0:1);i++)if(xs[i]==nx && ys[i]==ny)dead=1;
                if(!dead) {
                    if(grow){add_score(10);sfx(1);}else cell(xs[length-1],ys[length-1],32,1);
                    cell(xs[0],ys[0],T_BODY,1);
                    if(grow)length++;
                    for(i=length-1;i;i--){xs[i]=xs[i-1];ys[i]=ys[i-1];}
                    xs[0]=nx;ys[0]=ny;cell(nx,ny,T_HEAD,1);
                    if(length==234)win=1;else if(grow)fruit();
                }
            }
            hud("ORCHARD");text(0,1,"LENGTH",6);number(7,1,length,3,1);flush();
        }
    }while(result(win?"GARDEN COMPLETE!":"THE GARDEN RESTS",win?"PERFECT HARVEST!":"ONE MORE HARVEST?"));
}
