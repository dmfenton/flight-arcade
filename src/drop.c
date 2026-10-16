#pragma bank 9
#include "arcade.h"
static const uint16_t shapes[7][4]={
    {0x0F00,0x2222,0x00F0,0x4444},{0x6600,0x6600,0x6600,0x6600},
    {0x4E00,0x4640,0x0E40,0x4C40},{0x6C00,0x4620,0x06C0,0x8C40},
    {0xC600,0x2640,0x0C60,0x4C80},{0x8E00,0x6440,0x0E20,0x44C0},
    {0x2E00,0x4460,0x0E80,0xC440}
};
static uint8_t field[160],bag[7],bag_index,piece,next_piece,rotation;
static int8_t piece_x,piece_y;
static uint16_t lines;
static uint8_t draw_bag(void) {
    uint8_t i,j,t;
    if(bag_index>=7){for(i=0;i<7;i++)bag[i]=i;for(i=6;i;i--){j=random8(i+1);t=bag[i];bag[i]=bag[j];bag[j]=t;}bag_index=0;}
    return bag[bag_index++];
}
static uint8_t fits(int8_t px,int8_t py,uint8_t r) {
    uint8_t x,y;int8_t xx,yy;uint16_t mask=0x8000,shape=shapes[piece][r];
    for(y=0;y<4;y++)for(x=0;x<4;x++,mask>>=1)if(shape&mask){xx=px+x;yy=py+y;if(xx<0||xx>=10||yy<0||yy>=16||field[(uint8_t)yy*10+xx])return 0;}
    return 1;
}
static void draw_piece(int8_t px,int8_t py,uint8_t tile,uint8_t palette) {
    uint8_t x,y;uint16_t mask=0x8000,shape=shapes[piece][rotation];
    for(y=0;y<4;y++)for(x=0;x<4;x++,mask>>=1)if(shape&mask)cell(1+px+x,1+py+y,tile,palette);
}
static void render(void) {
    uint8_t x,y,v;uint16_t pos=21,mask=0x8000;int8_t ghost=piece_y;
    for(y=0;y<16;y++,pos+=20)for(x=0;x<10;x++) {
        v=field[y*10+x];
        if(tiles[pos+x]!=(v?T_WALL:32)||attrs[pos+x]!=(v?1+(v-1)%5:0))cell(1+x,1+y,v?T_WALL:32,v?1+(v-1)%5:0);
    }
    while(fits(piece_x,ghost+1,rotation))ghost++;
    draw_piece(piece_x,ghost,T_DOT,6);draw_piece(piece_x,piece_y,T_WALL,1+piece%5);
    box(13,4,4,4,0);
    for(y=0;y<4;y++)for(x=0;x<4;x++,mask>>=1)if(shapes[next_piece][0]&mask)cell(13+x,4+y,T_WALL,1+next_piece%5);
    hud("DROP");text(13,2,"NEXT",6);text(13,9,"LINES",6);number(13,10,lines,4,0);
    text(13,12,"LEVEL",6);number(14,13,1+lines/5,2,4);text(13,15,"UP",6);text(12,16,"HARDROP",6);
}
static uint8_t lock_piece(void) {
    uint8_t x,y,full,cleared=0,yy;uint16_t mask=0x8000,shape=shapes[piece][rotation],reward;
    for(y=0;y<4;y++)for(x=0;x<4;x++,mask>>=1)if(shape&mask)field[(piece_y+y)*10+piece_x+x]=piece+1;
    y=16;
    while(y) {
        y--;full=1;for(x=0;x<10;x++)if(!field[y*10+x])full=0;
        if(full) {
            for(yy=y;yy;yy--)memcpy(field+yy*10,field+(yy-1)*10,10);
            memset(field,0,10);cleared++;y++;
        }
    }
    if(cleared){reward=cleared==1?100:cleared==2?300:cleared==3?500:800;add_score(reward);lines+=cleared;sfx(3);}else sfx(0);
    piece=next_piece;next_piece=draw_bag();rotation=0;piece_x=3;piece_y=0;
    return fits(piece_x,piece_y,rotation);
}
void block_drop(void) BANKED {
    uint8_t timer,interval,r,alive,redraw,repeat;uint16_t frame;
    do {
        score=lines=0;bag_index=7;piece=draw_bag();next_piece=draw_bag();rotation=0;piece_x=3;piece_y=0;timer=0;frame=0;alive=1;
        repeat=0;memset(field,0,160);clear_screen(0);hide_objects();for(r=1;r<17;r++){cell(0,r,T_LINE,6);cell(11,r,T_LINE,6);}render();flush();
        while(alive) {
            if(!game_frame())return;frame++;redraw=0;
            if(keys&(J_LEFT|J_RIGHT)){if(repeat<240)repeat++;}else repeat=0;
            if((pressed&J_LEFT)||((keys&J_LEFT)&&repeat>=12&&repeat%4==0)){if(fits(piece_x-1,piece_y,rotation)){piece_x--;redraw=1;}}
            if((pressed&J_RIGHT)||((keys&J_RIGHT)&&repeat>=12&&repeat%4==0)){if(fits(piece_x+1,piece_y,rotation)){piece_x++;redraw=1;}}
            if(pressed&(J_A|J_B)) {
                r=(rotation+(pressed&J_A?1:3))&3;
                if(fits(piece_x,piece_y,r)){rotation=r;redraw=1;}
                else if(fits(piece_x-1,piece_y,r)){piece_x--;rotation=r;redraw=1;}
                else if(fits(piece_x+1,piece_y,r)){piece_x++;rotation=r;redraw=1;}
            }
            if(pressed&J_UP){while(fits(piece_x,piece_y+1,rotation)){piece_y++;add_score(2);}alive=lock_piece();timer=0;redraw=1;}
            else {
                interval=keys&J_DOWN?3:lines>=28?8:36-lines;
                if(++timer>=interval){timer=0;if(fits(piece_x,piece_y+1,rotation)){piece_y++;if(keys&J_DOWN)add_score(1);}else alive=lock_piece();redraw=1;}
            }
            if(redraw)render();flush();
        }
    }while(result("STACK COMPLETE","MAKE ANOTHER ROW."));
}
