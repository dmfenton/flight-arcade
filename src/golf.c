#pragma bank 15
#include "arcade.h"
static const char ranks[]=" A23456789TJQK";
static const uint8_t suits[4]={T_LIFE,T_GEM,T_LAMP,T_SHIP};
static uint8_t stack_cards[35],stock[17],counts[7],stock_pos,waste,cursor;
static uint8_t rank(uint8_t card){return card%13+1;}
static uint8_t playable(uint8_t card) {uint8_t a=rank(card),b=rank(waste);return a+1==b||b+1==a||(a==1&&b==13)||(a==13&&b==1);}
static uint8_t remaining(void){uint8_t i,n=0;for(i=0;i<7;i++)n+=counts[i];return n;}
static uint8_t moves_left(void){uint8_t i;for(i=0;i<7;i++)if(counts[i]&&playable(stack_cards[i*5+counts[i]-1]))return 1;return 0;}
static void card(uint8_t x,uint8_t y,uint8_t value,uint8_t p) {
    panel(x,y,2,2,p);cell(x,y,ranks[rank(value)],p);cell(x+1,y,suits[value/13],p);
}
static void render(void) {
    uint8_t c,r,i,p,x,y;
    for(c=0;c<7;c++)for(r=0;r<5;r++) {
        x=c*3;y=3+r*2;
        if(r<counts[c]){p=r+1==counts[c]&&playable(stack_cards[c*5+r])?1:7;card(x,y,stack_cards[c*5+r],p);}
        else box(x,y,2,2,0);
    }
    text(2,13,"STOCK",6);text(10,13,"WASTE",6);panel(3,14,2,2,6);card(11,14,waste,7);
    number(3,14,17-stock_pos,2,6);hud("GOLF");text(0,1,"LEFT",6);number(5,1,remaining(),2,0);text(10,1,"DRAW",6);number(15,1,17-stock_pos,2,4);
    x=cursor*3*8;y=(counts[cursor]?3+(counts[cursor]-1)*2:11)*8;
    for(i=0;i<4;i++)object(i,T_BIGCURSOR+i,x+(i&1)*8,y+(i>>1)*8,4);
}
void golf_solitaire(void) BANKED {
    uint8_t deck[52],i,j,t,redraw,won;
    do {
        score=0;cursor=0;stock_pos=1;
        for(i=0;i<52;i++)deck[i]=i;
        for(i=51;i;i--){j=random8(i+1);t=deck[i];deck[i]=deck[j];deck[j]=t;}
        memcpy(stack_cards,deck,35);memcpy(stock,deck+35,17);memset(counts,5,7);waste=stock[0];
        clear_screen(0);hide_objects();render();flush();
        while(remaining()&&(stock_pos<17||moves_left())) {
            if(!game_frame())return;redraw=0;
            if(pressed&J_LEFT){cursor=cursor?cursor-1:6;redraw=1;}
            if(pressed&J_RIGHT){cursor=(cursor+1)%7;redraw=1;}
            if((pressed&J_A)&&counts[cursor]&&playable(stack_cards[cursor*5+counts[cursor]-1])) {counts[cursor]--;waste=stack_cards[cursor*5+counts[cursor]];add_score(10);sfx(1);redraw=1;}
            if((pressed&J_B)&&stock_pos<17){waste=stock[stock_pos++];redraw=1;sfx(0);}
            if(redraw)render();flush();
        }
        won=!remaining();if(won)add_score(500);
    }while(result(won?"EVERY STACK CLEAR!":"END OF THE DECK",won?"A BEAUTIFUL ROUND":"ANOTHER DEAL AWAITS"));
}
