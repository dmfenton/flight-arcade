#pragma bank 8
#include "arcade.h"
static uint8_t river[14][18];
static uint8_t channel_left,channel_width;
static void new_row(uint8_t row,uint8_t obstacles) {
    uint8_t x;
    for(x=0;x<18;x++)river[row][x]=(x<channel_left || x>=channel_left+channel_width)?T_WALL:T_WATER;
    if(obstacles && random8(3)==0)river[row][channel_left+1+random8(channel_width-2)]=T_ROCK;
    if(obstacles && random8(5)==0)river[row][channel_left+1+random8(channel_width-2)]=T_GEM;
}
static void render(void) {
    uint8_t x,y,t,*source=&river[0][0],*display=tiles+41;
    for(y=0;y<14;y++,display+=2)for(x=0;x<18;x++) {
        t=*source++;
        if(*display++!=t)cell(x+1,y+2,t,t==T_WALL?1:t==T_GEM?4:t==T_ROCK?6:5);
    }
}
void river_run(void) BANKED {
    uint8_t hull,invuln,delay,interval,x,y,i,t,left,right,hit;
    int16_t px;
    do {
        score=0;hull=3;invuln=delay=0;channel_left=3;channel_width=12;px=76;
        clear_screen(0);hide_objects();for(y=0;y<14;y++)new_row(y,0);render();
        while(hull) {
            if(!game_frame())return;
            if(invuln)invuln--;
            if((keys&J_LEFT)&&px>8)px-=2;if((keys&J_RIGHT)&&px<144)px+=2;
            interval=keys&J_A?4:score>500?5:7;
            if(++delay>=interval) {
                delay=0;
                for(y=13;y;y--)memcpy(river[y],river[y-1],18);
                if(random8(4)==0) {
                    if(random8(2) && channel_left+channel_width<18)channel_left++;
                    else if(channel_left)channel_left--;
                }
                if(score && score%150==0 && channel_width>8)channel_width--;
                new_row(0,1);add_score(1);render();
            }
            x=(px-8+4)/8;if(x>17)x=17;
            hit=0;
            for(i=0;i<2;i++) {
                t=river[12+i][x];
                if(t==T_GEM){river[12+i][x]=T_WATER;add_score(25);sfx(1);render();}
                if(t==T_ROCK || t==T_WALL){hit=1;if(t==T_ROCK){river[12+i][x]=T_WATER;render();}}
            }
            if(hit && !invuln) {
                hull--;invuln=90;sfx(2);
                left=0;while(left<18&&river[12][left]==T_WALL)left++;
                right=left;while(right<18&&river[12][right]!=T_WALL)right++;
                px=8+(left+(right-left)/2)*8;
            }
            object(0,T_SHIP,px,invuln && (clock_frames&4)?160:114,4);
            hud("RIVER");text(0,1,"HULL",6);number(5,1,hull,1,2);text(10,1,keys&J_A?"BOOST!":"CRUISE",4);flush();
        }
    }while(result("RIVER COMPLETE","THE RIVER GOES ON"));
}
