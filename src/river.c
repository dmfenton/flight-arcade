#pragma bank 5
#include "arcade.h"
static uint8_t river[14][18],channel_left,hull,power,leg,route,rescues,invuln,flow_timer;
static int16_t fx;
static int8_t vx,current;
static uint16_t distance,frame;
static void new_row(uint8_t row,uint8_t obstacles){
    uint8_t x,gap=channel_left+5;
    for(x=0;x<18;x++)river[row][x]=(x<channel_left||x>=channel_left+11)?T_WALL:T_WATER;
    if(obstacles){
        x=channel_left+1+random8(9);if(x!=gap)river[row][x]=T_ROCK;
        if(distance%5==0)river[row][channel_left+2+random8(7)]=T_GEM;
    }
}
static void render(void){
    uint8_t x,y,t,p;
    for(y=0;y<14;y++)for(x=0;x<18;x++){
        t=river[y][x];p=t==T_WALL?1:t==T_GEM?4:t==T_ROCK?6:5;cell(x+1,y+2,t,p);
    }
}
static void reset_reach(void){uint8_t y;clear_screen(0);hide_objects();channel_left=3;fx=80*16;vx=0;current=leg&1?1:-1;flow_timer=0;for(y=0;y<14;y++)new_row(y,0);render();}
void river_run(void) BANKED {
    uint8_t interval,x,t,i,hit,pick,won;
    do{
        score=distance=frame=0;hull=4;power=100;leg=1;route=rescues=invuln=won=0;reset_reach();
        while(hull&&!won){
            if(!game_frame())return;frame++;if(invuln)invuln--;
            if(keys&J_LEFT)vx-=2;if(keys&J_RIGHT)vx+=2;
            if(!(frame&3)){if(vx)vx+=vx>0?-1:1;vx+=current;}
            if(keys&J_B&&power){vx/=2;if(!(frame&3))power--;}
            else if(keys&J_A&&power){if(!(frame&1))power--;}
            else if(!(frame&3)&&power<100)power++;
            if(vx>28)vx=28;if(vx< -28)vx=-28;fx+=vx;
            if(fx<8*16){fx=8*16;vx=0;}if(fx>144*16){fx=144*16;vx=0;}
            interval=(keys&J_B)&&power?12:(keys&J_A)&&power?3:route?4:7;
            if(++flow_timer>=interval){
                flow_timer=0;memmove(river[1],river[0],13*18);distance++;add_score(route?2:1);
                if(distance%10==0){if(random8(2)&&channel_left<6)channel_left++;else if(channel_left)channel_left--;}
                if(distance%20==0)current=-current;
                new_row(0,1);render();
            }
            x=(fx/16-8+4)/8;if(x>17)x=17;hit=0;
            for(i=0;i<2;i++){
                t=river[12+i][x];if(t==T_GEM){river[12+i][x]=T_WATER;rescues++;add_score(50);power=power<75?power+25:100;sfx(1);render();}
                if(t==T_ROCK||t==T_WALL){hit=1;if(t==T_ROCK){river[12+i][x]=T_WATER;render();}}
            }
            if(hit&&!invuln){hull--;invuln=75;fx=(channel_left+6)*8*16;vx=0;sfx(2);}
            if(distance>=leg*60){
                if(leg==3){won=1;add_score(300+rescues*25);}
                else{
                    pick=choice("RIVER FORK","CHOOSE YOUR REACH","BACKWATER +1 HULL","RAPIDS DOUBLE PTS");if(pick==255)return;
                    route=pick;if(!route&&hull<5)hull++;leg++;invuln=60;reset_reach();
                }
            }
            object(0,T_SHIP,fx/16,invuln&&(frame&4)?160:114,4);
            hud("TIDE");text(0,1,"H",6);number(1,1,hull,1,1);text(3,1,"P",6);number(4,1,power,3,5);text(8,1,current>0?">>>":"<<<",3);text(12,1,"L",6);number(13,1,leg,1,4);text(15,1,"R",6);number(16,1,rescues,2,4);flush();
        }
    }while(result(won?"ESTUARY REACHED!":"BOAT LOST",won?"A RIVER OF STORIES":"READ THE CURRENT"));
}
