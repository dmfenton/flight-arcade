#pragma bank 6
#include "arcade.h"
static uint8_t xs[100],ys[100],length,fx,fy,power,phase,arena,eaten,dir,next,gate_on;
static uint16_t turn;
static uint8_t obstacle(uint8_t x,uint8_t y){return gate_on&&arena>1&&(x==5||x==14)&&y>=7&&y<=11;}
static uint8_t occupied(uint8_t x,uint8_t y){uint8_t i;for(i=0;i<length;i++)if(xs[i]==x&&ys[i]==y)return 1;return 0;}
static void fruit(void){uint8_t i;for(i=0;i<240;i++){fx=1+random8(18);fy=3+random8(13);if(!occupied(fx,fy)&&!((fx==5||fx==14)&&fy>=7&&fy<=11))return;}for(fy=3;fy<16;fy++)for(fx=1;fx<19;fx++)if(!occupied(fx,fy)&&!obstacle(fx,fy))return;}
static void render(void){
    uint8_t x,y,i;
    for(y=2;y<17;y++)for(x=0;x<20;x++)cell(x,y,(x==0||x==19||y==2||y==16||obstacle(x,y))?T_WALL:32,obstacle(x,y)?3:1);
    cell(fx,fy,T_APPLE,2);for(i=length;i;i--)cell(xs[i-1],ys[i-1],i==1?T_HEAD:T_BODY,phase?3:1);
}
static void coil(void){uint8_t i;for(i=0;i<length;i++){xs[i]=i<12?12-i:i-11;ys[i]=i<12?9:10;}dir=next=0;gate_on=0;turn=0;phase=0;fruit();render();}
void orchard_snake(void) BANKED {
    uint8_t i,delay,interval,dead,grow,won,nx,ny,pick;
    do{
        score=0;length=4;arena=1;eaten=0;power=6;delay=dead=won=0;clear_screen(0);hide_objects();coil();
        while(!dead&&!won){
            if(!game_frame())return;
            if(pressed&J_RIGHT&&dir!=2)next=0;if(pressed&J_DOWN&&dir!=3)next=1;
            if(pressed&J_LEFT&&dir!=0)next=2;if(pressed&J_UP&&dir!=1)next=3;
            if(pressed&J_B&&!phase&&power>=3){power-=3;phase=4;sfx(3);render();}
            interval=(keys&J_A)&&power?4:11-arena;
            if(++delay>=interval){
                delay=0;dir=next;nx=xs[0];ny=ys[0];if(dir==0)nx++;if(dir==1)ny++;if(dir==2)nx--;if(dir==3)ny--;
                if(phase){if(nx==0)nx=18;if(nx==19)nx=1;if(ny==2)ny=15;if(ny==16)ny=3;}
                grow=nx==fx&&ny==fy;
                if(!phase){if(nx<1||nx>18||ny<3||ny>15||obstacle(nx,ny))dead=1;for(i=0;i<length-(grow?0:1);i++)if(xs[i]==nx&&ys[i]==ny)dead=1;}
                if(!dead){
                    if(keys&J_A&&power)power--;
                    if(grow){add_score((keys&J_A?20:10)*(phase?2:1));power=power<7?power+3:9;eaten++;if(length<98)length++;sfx(1);}
                    for(i=length-1;i;i--){xs[i]=xs[i-1];ys[i]=ys[i-1];}xs[0]=nx;ys[0]=ny;
                    turn++;gate_on=(turn/8)&1;if(phase)phase--;
                    if(grow){
                        if(eaten==5){
                            if(arena==3){won=1;add_score(500);}
                            else{pick=choice("ARENA HARVESTED","CHOOSE YOUR COIL","LONGER +200 PTS","SHORTER +3 POWER");if(pick==255)return;
                                if(pick==0){add_score(200);length+=2;}else{length=4;power=power<7?power+3:9;}arena++;eaten=0;coil();}
                        }else fruit();
                    }
                    render();
                }
            }
            hud("SERPENT");text(0,1,"P",6);number(1,1,power,1,5);text(4,1,"PH",6);number(6,1,phase,1,3);text(9,1,"AREA",6);number(13,1,arena,1,4);number(16,1,eaten,1,1);text(17,1,"/5",6);flush();
        }
    }while(result(won?"PULSE MASTER!":"COIL COLLISION",won?"THREE ARENAS FED":"KEEP A PHASE READY"));
}
