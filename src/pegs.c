#pragma bank 13
#include "arcade.h"
extern const uint32_t peg_levels[20],peg_valid;
static uint32_t pegs,undo_pegs;
static uint8_t cursor,selected,undo_valid,level;
static uint16_t moves;
static uint8_t count_pegs(void){uint8_t i,n=0;for(i=0;i<25;i++)if(pegs&(1UL<<i))n++;return n;}
static uint8_t legal(uint8_t from,uint8_t to) {
    int8_t dx=(to%5)-(from%5),dy=(to/5)-(from/5);uint8_t middle;
    if(!((dx==2||dx==-2)&&!dy)&&!((dy==2||dy==-2)&&!dx))return 0;
    middle=(from+to)/2;
    return (peg_valid&(1UL<<to))&&(pegs&(1UL<<from))&&(pegs&(1UL<<middle))&&!(pegs&(1UL<<to));
}
static uint8_t movable(void){uint8_t i;for(i=0;i<25;i++)if(pegs&(1UL<<i)){
    if(i%5>=2&&legal(i,i-2))return 1;if(i%5<3&&legal(i,i+2))return 1;
    if(i>=10&&legal(i,i-10))return 1;if(i<15&&legal(i,i+10))return 1;
}return 0;}
static void render(void) {
    uint8_t i,j,x,y,p;
    for(i=0;i<25;i++) {
        x=5+(i%5)*2;y=4+(i/5)*2;p=selected==i?4:(pegs&(1UL<<i))?1:6;
        for(j=0;j<4;j++)cell(x+(j&1),y+(j>>1),peg_valid&(1UL<<i)?T_DISC+j:32,p);
    }
    hud("PEGS");text(0,1,"LEFT",6);number(5,1,count_pegs(),2,0);text(10,1,"NO.",6);number(14,1,level+1,2,4);
    text(2,15,selected==255?"A PICK   B UNDO  ":"A JUMP   B CANCEL",6);
}
void peg_trails(void) BANKED {
    uint8_t i,n,ended,won,redraw,replay=1;
    level=choose_puzzle(12,level,20);if(level==255){level=0;return;}score=0;
    while(replay) {
        pegs=peg_levels[level];cursor=12;selected=255;undo_valid=0;moves=0;ended=won=0;
        clear_screen(0);hide_objects();render();flush();
        while(!ended) {
            if(!game_frame())return;redraw=0;
            if((pressed&J_LEFT)&&cursor%5)cursor--;if((pressed&J_RIGHT)&&cursor%5<4)cursor++;
            if((pressed&J_UP)&&cursor>=5)cursor-=5;if((pressed&J_DOWN)&&cursor<20)cursor+=5;
            if(pressed&J_B) {
                if(selected!=255)selected=255;
                else if(undo_valid){pegs=undo_pegs;undo_valid=0;if(moves)moves--;}
                redraw=1;
            }
            if((pressed&J_A)&&(peg_valid&(1UL<<cursor))) {
                if(selected==255){if(pegs&(1UL<<cursor))selected=cursor;}
                else if(legal(selected,cursor)) {
                    undo_pegs=pegs;undo_valid=1;pegs^=(1UL<<selected)|(1UL<<((selected+cursor)/2))|(1UL<<cursor);selected=255;moves++;sfx(1);
                } else if(pegs&(1UL<<cursor))selected=cursor;
                redraw=1;
            }
            if(redraw){render();n=count_pegs();won=n==1;ended=won||!movable();}
            for(i=0;i<4;i++)object(i,T_BIGCURSOR+i,(5+(cursor%5)*2+(i&1))*8,(4+(cursor/5)*2+(i>>1))*8,4);
            flush();
        }
        if(won)add_score(300+moves*20);
        replay=result_action(won?"ONE PEG REMAINS!":"NO JUMPS REMAIN",won?"A PERFECT TRAIL.":"TRY ANOTHER ROUTE",won?"A NEXT PUZZLE":"A TRY AGAIN");
        if(won&&replay)level=(level+1)%20;
    }
}
