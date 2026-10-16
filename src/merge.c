#pragma bank 4
#include "arcade.h"
static uint8_t board[16],undo_board[16],line_values[4];
static uint16_t undo_score;
static uint8_t undo_valid;
static void spawn_tile(void) {
    uint8_t free[16],count=0,i;
    for(i=0;i<16;i++)if(!board[i])free[count++]=i;
    if(count)board[free[random8(count)]]=random8(10)?1:2;
}
static uint8_t position(uint8_t dir,uint8_t line,uint8_t k) {
    if(dir==0)return line*4+k;
    if(dir==1)return line*4+3-k;
    if(dir==2)return k*4+line;
    return (3-k)*4+line;
}
static uint8_t slide(uint8_t dir) {
    uint8_t l,k,n,j,v,changed=0,index;
    for(l=0;l<4;l++) {
        n=0;memset(line_values,0,4);
        for(k=0;k<4;k++){v=board[position(dir,l,k)];if(v)line_values[n++]=v;}
        for(k=0;k+1<n;k++)if(line_values[k]==line_values[k+1] && line_values[k]<15) {
            line_values[k]++;add_score(1U<<line_values[k]);
            for(j=k+1;j+1<n;j++)line_values[j]=line_values[j+1];line_values[--n]=0;
        }
        for(k=0;k<4;k++){index=position(dir,l,k);if(board[index]!=line_values[k])changed=1;board[index]=line_values[k];}
    }
    return changed;
}
static uint8_t playable(void) {
    uint8_t i;
    for(i=0;i<16;i++)if(!board[i] || (board[i]<15 && ((i%4<3 && board[i]==board[i+1]) || (i<12 && board[i]==board[i+4]))))return 1;
    return 0;
}
static void render(void) {
    uint8_t i,x,y,p,width,largest=0;
    uint16_t value;
    for(i=0;i<16;i++) {
        x=2+(i%4)*4;y=3+(i/4)*3;
        p=board[i]?1+(board[i]%5):6;
        panel(x,y,4,3,p);
        if(board[i]) {
            value=1U<<board[i];width=value>=10000?5:value>=1000?4:value>=100?3:value>=10?2:1;
            if(width>4){text(x,y+1,value>=32768U?"32K":"16K",p);}else number(x+(4-width)/2,y+1,value,width,p);
        } else cell(x+1,y+1,T_DOT,6);
        if(board[i]>largest)largest=board[i];
    }
    hud("MERGE");text(0,1,largest>=11?"2048! KEEP GOING":"B UNDO LAST MOVE",largest>=11?4:6);
}
void merge_cabin(void) BANKED {
    uint8_t dir,stuck,replay=1,temp[16];uint16_t before;
    while(replay) {
        score=0;undo_valid=0;stuck=0;memset(board,0,16);spawn_tile();spawn_tile();clear_screen(0);hide_objects();render();flush();
        while(1) {
            if(!game_frame())return;
            if((pressed&J_B) && undo_valid){memcpy(board,undo_board,16);score=undo_score;undo_valid=0;stuck=0;render();sfx(0);}
            if(stuck && (pressed&J_A)){replay=result("CABIN IS FULL","MAKE A LITTLE ROOM");break;}
            dir=255;
            if(pressed&J_LEFT)dir=0;else if(pressed&J_RIGHT)dir=1;else if(pressed&J_UP)dir=2;else if(pressed&J_DOWN)dir=3;
            if(dir!=255 && !stuck) {
                memcpy(temp,board,16);before=score;
                if(slide(dir)) {
                    memcpy(undo_board,temp,16);undo_score=before;undo_valid=1;spawn_tile();sfx(1);
                    stuck=!playable();render();
                    if(stuck)text(0,1,"B UNDO / A FINISH",4);
                }
            }
            flush();
        }
    }
}
