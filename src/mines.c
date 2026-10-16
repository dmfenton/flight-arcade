#pragma bank 5
#include "arcade.h"
#define REVEALED 1
#define FLAGGED 2
#define MINED 4
static uint8_t ground[63],queue[63],revealed;
static uint8_t neighbors(uint8_t index) {
    int8_t x,y,cx=index%9,cy=index/9;uint8_t n=0;
    for(y=cy-1;y<=cy+1;y++)for(x=cx-1;x<=cx+1;x++)if(x>=0&&x<9&&y>=0&&y<7 && (ground[(uint8_t)y*9+x]&MINED))n++;
    return n;
}
static void plant(uint8_t first) {
    uint8_t n=0,p;int8_t dx,dy;
    while(n<10){p=random8(63);dx=(p%9)-(first%9);dy=(p/9)-(first/9);if(!(ground[p]&MINED) && !(dx>=-1&&dx<=1&&dy>=-1&&dy<=1)){ground[p]|=MINED;n++;}}
}
static void uncover(uint8_t p) {
    uint8_t head=0,tail=0,index,q;int8_t x,y,cx,cy;
    ground[p]|=REVEALED;queue[tail++]=p;revealed++;
    while(head<tail) {
        index=queue[head++];if(neighbors(index))continue;
        cx=index%9;cy=index/9;
        for(y=cy-1;y<=cy+1;y++)for(x=cx-1;x<=cx+1;x++)if(x>=0&&x<9&&y>=0&&y<7) {
            q=(uint8_t)y*9+x;
            if(!(ground[q]&(REVEALED|FLAGGED|MINED))) {ground[q]|=REVEALED;queue[tail++]=q;revealed++;}
        }
    }
}
static void render(uint8_t ended) {
    uint8_t i,x,y,base,p,n,flags=0,j;
    for(i=0;i<63;i++) {
        x=1+(i%9)*2;y=3+(i/9)*2;p=6;base=T_COVER;
        if(ground[i]&FLAGGED){base=T_BIGFLAG;p=4;flags++;}
        if(ground[i]&REVEALED) {n=neighbors(i);base=T_NUMBER+n*4;p=n==1?5:n==2?1:n==3?2:3;}
        if(ended && (ground[i]&MINED)){base=T_BIGMINE;p=2;}
        for(j=0;j<4;j++)cell(x+(j&1),y+(j>>1),base+j,p);
    }
    hud("MINES");text(0,1,"FLAGS",6);number(6,1,flags,2,4);text(10,1,"SAFE",6);number(15,1,revealed,2,1);
}
void mine_garden(void) BANKED {
    uint8_t cx,cy,p,first,ended,i,win;uint16_t frames,seconds;
    do {
        score=0;cx=4;cy=3;first=1;ended=win=0;frames=seconds=0;revealed=0;memset(ground,0,63);clear_screen(0);hide_objects();render(0);
        while(1) {
            if(!game_frame())return;
            if(!first&&!ended && ++frames==60){frames=0;if(seconds<999)seconds++;}
            if((pressed&J_LEFT)&&cx)cx--;if((pressed&J_RIGHT)&&cx<8)cx++;
            if((pressed&J_UP)&&cy)cy--;if((pressed&J_DOWN)&&cy<6)cy++;
            p=cy*9+cx;
            if(!ended && (pressed&J_B) && !(ground[p]&REVEALED)){ground[p]^=FLAGGED;render(0);sfx(0);}
            if(ended && (pressed&J_A))break;
            if(!ended && (pressed&J_A) && !(ground[p]&(FLAGGED|REVEALED))) {
                if(first){plant(p);first=0;}
                if(ground[p]&MINED){ended=1;score=revealed;sfx(2);}
                else {uncover(p);score=revealed;sfx(0);if(revealed==53){ended=win=1;add_score(600-(seconds>500?500:seconds));}}
                render(ended);
            }
            if(ended)text(0,1,win?"CLEAR! A CONTINUE":"MINE!  A CONTINUE",win?1:2);
            for(i=0;i<4;i++)object(i,T_BIGCURSOR+i,(1+cx*2+(i&1))*8,(3+cy*2+(i>>1))*8,4);
            flush();
        }
    }while(result(win?"GARDEN CLEARED!":"A HIDDEN SURPRISE",win?"A SAFE DAY.":"TRY ANOTHER PATCH"));
}
