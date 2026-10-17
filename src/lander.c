#pragma bank 8
#include "arcade.h"
static uint8_t terrain[20],fuel,port,target,mass,fragile,intact,deliveries,docked;
static int16_t fx,fy;
static int8_t vx,vy,wind;
static uint16_t frame;
static const uint8_t pad_x[3]={2,9,17};
static uint8_t pad_at(int16_t x){uint8_t i;for(i=0;i<3;i++)if(x>=pad_x[i]*8-8&&x+8<=(pad_x[i]+2)*8)return i;return 255;}
static void landscape(void){
    uint8_t x,y,p;clear_screen(0);hide_objects();
    for(y=4;y<12;y++)for(x=0;x<20;x++)if(random8(20)==0)cell(x,y,T_DOT,6);
    for(x=0;x<20;x++){
        p=pad_at(x*8);terrain[x]=p!=255?15:x==5||x==6||x==12||x==13?12:14;
        for(y=terrain[x];y<17;y++)cell(x,y,p!=255?T_WALL:T_ROCK,p==target?4:p==0?5:p!=255?1:6);
    }
    for(x=0;x<3;x++)number(pad_x[x],16,x+1,1,x==target?4:5);
}
static uint8_t contract(void){
    uint8_t pick;
    pick=choice("CARGO CONTRACT","B CASH OUT / MENU","ORE: NEXT PORT","GLASS: FAR PORT");if(pick==255)return 0;
    target=(port+1+pick)%3;mass=pick?1:3;fragile=pick;intact=1;docked=1;vx=vy=0;
    fx=pad_x[port]*8*16;fy=112*16;frame=0;landscape();return 1;
}
static void render(uint8_t thrust){
    object(0,T_SHIP,fx/16,fy/16,5);object(1,T_BULLET,fx/16+1,thrust&&(frame&2)?fy/16+7:160,2);
    hud("COURIER");text(0,1,"F",6);number(1,1,fuel,3,fuel>30?1:2);text(6,1,"TO",6);number(9,1,target+1,1,4);text(12,1,"JOB",6);number(16,1,deliveries+1,1,5);text(17,1,"/5",6);
    text(0,2,wind>0?"WIND >":"WIND <",3);text(8,2,intact?(fragile?"GLASS":"ORE  "):"EMPTY",intact?4:6);text(14,2,"M",6);number(15,2,mass,1,6);text(17,2,(vy<0?-vy:vy)<=(fragile?12:18)&&vx>=-10&&vx<=10?"OK ":"HOT",2);
    if(docked){text(1,4,"DOCKED: A TAKE OFF",5);text(1,5,"B ABORT / NEW JOB",6);}else{box(0,4,20,2,0);}
    flush();
}
void lunar_lander(void) BANKED {
    uint8_t ended,won,thrust,landed,p;int16_t x,y;int8_t speed;
    do{
        score=0;fuel=180;port=0;deliveries=0;ended=won=0;wind=1;if(!contract())return;
        while(!ended){
            if(!game_frame())return;frame++;thrust=0;
            if(docked){
                if(pressed&J_B){intact=0;if(!contract())return;}
                if(pressed&J_A){docked=0;vy=-20;fy-=16*4;box(0,4,20,2,0);}
                render(0);continue;
            }
            if(pressed&J_B&&intact){intact=0;mass=0;fragile=0;sfx(2);}
            if(frame%240==0)wind=-wind;
            if(!(frame&3)&&vy<48)vy++;if(mass==3&&frame%8==0&&vy<48)vy++;
            if(frame%16==0)vx+=wind;
            if(fuel){
                if(keys&(J_A|J_UP)){vy-=2;thrust=1;}
                if(keys&J_LEFT){vx--;thrust=1;}if(keys&J_RIGHT){vx++;thrust=1;}
                if(thrust&&!(frame&3))fuel--;
            }
            if(vy< -28)vy=-28;if(vx>24)vx=24;if(vx< -24)vx=-24;
            fx+=vx;fy+=vy;if(fx<0){fx=0;vx=0;}if(fx>152*16){fx=152*16;vx=0;}if(fy<24*16){fy=24*16;if(vy<0)vy=0;}
            x=fx/16;y=fy/16;speed=vy<0?-vy:vy;
            if(y+8>=terrain[(x+4)/8]*8){
                p=pad_at(x);landed=p!=255&&speed<=(fragile?12:18)&&vx>=-10&&vx<=10;
                if(!landed)ended=1;
                else{
                    port=p;fy=112*16;vx=vy=0;docked=1;sfx(1);
                    if(port==0){fuel=fuel<120?fuel+60:180;}
                    if(port==target){
                        deliveries++;add_score(intact?(fragile?500:350):25);
                        if(deliveries==5){won=ended=1;add_score(500+fuel);}
                        else if(!contract())return;
                    }else landscape();
                }
            }
            render(thrust);
        }
    }while(result(won?"ROUTE COMPLETE!":"COURIER DOWN",won?"FIVE CONTRACTS FLOWN":"MASS AND MOMENTUM"));
}
