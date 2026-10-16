#pragma bank 14
#include "arcade.h"
static uint8_t board[36],cursor,show_hints;
static uint8_t flips(uint8_t pos,uint8_t who,uint8_t apply) {
    int8_t dx,dy,x,y,px=pos%6,py=pos/6;uint8_t n,total=0,other=3-who,i;
    if(board[pos])return 0;
    for(dy=-1;dy<=1;dy++)for(dx=-1;dx<=1;dx++)if(dx||dy) {
        x=px+dx;y=py+dy;n=0;
        while(x>=0&&x<6&&y>=0&&y<6&&board[(uint8_t)y*6+x]==other){n++;x+=dx;y+=dy;}
        if(n&&x>=0&&x<6&&y>=0&&y<6&&board[(uint8_t)y*6+x]==who) {
            total+=n;
            if(apply){x=px+dx;y=py+dy;for(i=0;i<n;i++){board[(uint8_t)y*6+x]=who;x+=dx;y+=dy;}}
        }
    }
    if(total&&apply)board[pos]=who;
    return total;
}
static uint8_t legal_moves(uint8_t who){uint8_t i,n=0;for(i=0;i<36;i++)if(flips(i,who,0))n++;return n;}
static uint8_t count(uint8_t who){uint8_t i,n=0;for(i=0;i<36;i++)if(board[i]==who)n++;return n;}
static void render(void) {
    uint8_t i,j,x,y,p,v;
    for(i=0;i<36;i++) {
        x=4+(i%6)*2;y=3+(i/6)*2;v=board[i];p=v==1?1:v==2?2:6;
        for(j=0;j<4;j++)cell(x+(j&1),y+(j>>1),v?T_DISC+j:32,p);
        if(!v&&show_hints&&flips(i,1,0))cell(x,y,T_DOT,1);
    }
    hud("DISCS");text(0,1,"YOU",1);number(4,1,count(1),2,1);text(10,1,"CPU",2);number(14,1,count(2),2,2);
    text(1,16,"A PLACE   B HINTS",6);
}
static void cpu_move(void) {
    uint8_t i,n,best_pos=255,x,y;int16_t value,best_value=-1000;
    for(i=0;i<36;i++) {
        n=flips(i,2,0);if(!n)continue;x=i%6;y=i/6;value=n*2;
        if((x==0||x==5)&&(y==0||y==5))value+=100;
        else if(x==0||x==5||y==0||y==5)value+=10;
        if(x<=1&&y<=1&&board[0]!=2)value-=25;
        if(x>=4&&y<=1&&board[5]!=2)value-=25;
        if(x<=1&&y>=4&&board[30]!=2)value-=25;
        if(x>=4&&y>=4&&board[35]!=2)value-=25;
        if(value>best_value){best_value=value;best_pos=i;}
    }
    if(best_pos!=255)flips(best_pos,2,1);
}
void orbit_discs(void) BANKED {
    uint8_t turn,one=0,two=0,ended,i,redraw,green,red,board_changed;
    do {
        memset(board,0,36);board[14]=board[21]=2;board[15]=board[20]=1;turn=1;cursor=8;show_hints=1;score=0;ended=0;
        clear_screen(0);hide_objects();render();flush();board_changed=1;
        while(!ended) {
            if(!game_frame())return;redraw=0;
            if(board_changed){one=legal_moves(1);two=legal_moves(2);board_changed=0;}
            if(!one&&!two){ended=1;continue;}
            if(turn==1&&!one)turn=2;if(turn==2&&!two)turn=1;
            if(turn==2){cpu_move();turn=1;redraw=board_changed=1;sfx(0);}
            else {
                if((pressed&J_LEFT)&&cursor%6)cursor--;if((pressed&J_RIGHT)&&cursor%6<5)cursor++;
                if((pressed&J_UP)&&cursor>=6)cursor-=6;if((pressed&J_DOWN)&&cursor<30)cursor+=6;
                if(pressed&J_B){show_hints^=1;redraw=1;}
                if((pressed&J_A)&&flips(cursor,1,1)){turn=2;redraw=board_changed=1;sfx(1);}
            }
            if(redraw)render();
            for(i=0;i<4;i++)object(i,T_BIGCURSOR+i,(4+(cursor%6)*2+(i&1))*8,(3+(cursor/6)*2+(i>>1))*8,4);flush();
        }
        green=count(1);red=count(2);score=green*10;if(green>red)add_score(500);
    }while(result(green>red?"YOU TAKE THE ORBIT":green==red?"AN EVEN ORBIT":"CPU TAKES ORBIT","A CLEVER FINISH."));
}
