#pragma bank 11
#include "arcade.h"
static const uint8_t icons[12]={T_SHIP,T_ALIEN,T_APPLE,T_GEM,T_LAMP,T_LIFE,T_FLAG,T_STAR,T_BALL,T_PERSON,T_CRATE,T_WALL};
static uint8_t cards[24],matched[24],cursor,first,second;
static uint16_t turns;
static void render(void) {
    uint8_t i,x,y,p;
    for(i=0;i<24;i++) {
        x=1+(i%6)*3;y=3+(i/6)*3;
        p=matched[i]?1:(i==first||i==second)?1+cards[i]%5:6;
        panel(x,y,3,3,p);cell(x+1,y+1,(matched[i]||i==first||i==second)?icons[cards[i]]:'?',p);
    }
    hud("PAIRS");text(0,1,"TURNS",6);number(6,1,turns,3,0);
}
void flip_pairs(void) BANKED {
    uint8_t i,j,t,found,timer,redraw,x,y;
    do {
        score=0;turns=0;cursor=0;first=second=255;found=timer=0;
        memset(matched,0,24);for(i=0;i<24;i++)cards[i]=i/2;
        for(i=23;i;i--){j=random8(i+1);t=cards[i];cards[i]=cards[j];cards[j]=t;}
        clear_screen(0);hide_objects();render();flush();
        while(found<12) {
            if(!game_frame())return;redraw=0;
            if((pressed&J_LEFT)&&cursor%6)cursor--;if((pressed&J_RIGHT)&&cursor%6<5)cursor++;
            if((pressed&J_UP)&&cursor>=6)cursor-=6;if((pressed&J_DOWN)&&cursor<18)cursor+=6;
            if(timer){if(!--timer){first=second=255;redraw=1;}}
            else {
                if((pressed&J_B)&&first!=255){first=255;redraw=1;}
                if((pressed&J_A)&&!matched[cursor]&&cursor!=first) {
                    if(first==255)first=cursor;
                    else {second=cursor;turns++;if(cards[first]==cards[second]){matched[first]=matched[second]=1;found++;first=second=255;sfx(1);}else{timer=45;sfx(0);}}
                    redraw=1;
                }
            }
            if(redraw)render();x=(1+(cursor%6)*3)*8;y=(3+(cursor/6)*3)*8;
            object(0,(uint8_t)T_BIGCURSOR,x,y,4);object(1,(uint8_t)(T_BIGCURSOR+1),x+16,y,4);object(2,(uint8_t)(T_BIGCURSOR+2),x,y+16,4);object(3,(uint8_t)(T_BIGCURSOR+3),x+16,y+16,4);flush();
        }
        score=turns>=79?50:1000-turns*12;
    }while(result("ALL PAIRED UP!","A MEMORY WELL KEPT"));
}
