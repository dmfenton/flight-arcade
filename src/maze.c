#pragma bank 12
#include "arcade.h"
static uint8_t maze[195],seen[195],visited[42],stack[42],player,collected;
static void generate(void) {
    uint8_t top=1,current,cx,cy,options[4],n,chosen,nx,ny,index,i;
    memset(maze,1,195);memset(seen,0,195);memset(visited,0,42);stack[0]=0;visited[0]=1;maze[16]=0;
    while(top) {
        current=stack[top-1];cx=current%7;cy=current/7;n=0;
        if(cx&&!visited[current-1])options[n++]=current-1;
        if(cx<6&&!visited[current+1])options[n++]=current+1;
        if(cy&&!visited[current-7])options[n++]=current-7;
        if(cy<5&&!visited[current+7])options[n++]=current+7;
        if(!n){top--;continue;}
        chosen=options[random8(n)];nx=chosen%7;ny=chosen/7;
        maze[(cy+ny+1)*15+cx+nx+1]=0;maze[(ny*2+1)*15+nx*2+1]=0;
        visited[chosen]=1;stack[top++]=chosen;
    }
    maze[178]=3;player=16;collected=0;
    for(i=0;i<5;i++){do{index=random8(195);}while(maze[index]||index==player);maze[index]=2;}
}
static void discover(void) {
    int8_t x,y,cx=player%15,cy=player/15;
    for(y=cy-2;y<=cy+2;y++)for(x=cx-2;x<=cx+2;x++)if(x>=0&&x<15&&y>=0&&y<13)seen[(uint8_t)y*15+x]=1;
}
static void render(uint8_t map,uint16_t level) {
    uint8_t i,x,y,t,p;uint16_t position;
    for(y=0;y<13;y++)for(x=0;x<15;x++) {
        i=y*15+x;t=32;p=0;
        if(map||seen[i]) {
            t=maze[i]==1?T_WALL:maze[i]==2?T_GEM:maze[i]==3?T_GATE:T_DOT;
            p=maze[i]==1?3:maze[i]==2?4:maze[i]==3?(collected==5?1:6):6;
        }
        position=(uint16_t)(y+3)*20+x+2;
        if(tiles[position]!=t||attrs[position]!=p)cell(x+2,y+3,t,p);
    }
    object(0,T_PERSON,(2+player%15)*8,(3+player/15)*8,1);
    hud("MAZE");text(0,1,"GEMS",6);number(5,1,collected,1,4);text(6,1,"/5",6);text(10,1,"ROOM",6);number(15,1,level,3,0);
}
void maze_hopper(void) BANKED {
    uint8_t peek=0,previous_peek=0,redraw,repeat;int16_t next;int8_t d;uint16_t level=1,frame;
    score=0;
    while(1) {
        generate();frame=0;repeat=0;clear_screen(0);hide_objects();discover();render(0,level);flush();
        while(collected<5||player!=178) {
            if(!game_frame())return;frame++;d=0;redraw=0;
            if(keys&15){if(repeat<240)repeat++;}else repeat=0;
            if((pressed&J_LEFT)||((keys&J_LEFT)&&repeat>=12&&repeat%8==0))d=-1;
            if((pressed&J_RIGHT)||((keys&J_RIGHT)&&repeat>=12&&repeat%8==0))d=1;
            if((pressed&J_UP)||((keys&J_UP)&&repeat>=12&&repeat%8==0))d=-15;
            if((pressed&J_DOWN)||((keys&J_DOWN)&&repeat>=12&&repeat%8==0))d=15;
            next=(int16_t)player+d;
            if(d&&next>=0&&next<195&&maze[next]!=1) {
                player=next;
                if(maze[player]==2){maze[player]=0;collected++;add_score(20);sfx(1);}
                discover();redraw=1;
            }
            peek=(keys&J_A)!=0;if(peek!=previous_peek)redraw=1;previous_peek=peek;
            if(redraw)render(peek,level);flush();
        }
        add_score(200);
        if(!result_action("THE GATE IS OPEN!","ANOTHER WORLD AHEAD","A NEW MAZE"))return;
        level++;
    }
}
