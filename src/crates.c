#pragma bank 10
#include "arcade.h"
extern const uint8_t crate_levels[24][63],crate_starts[24];
static uint8_t board[63],undo_board[63],player,undo_player,undo_valid,level;
static uint16_t moves;
static void render(void) {
    uint8_t i,x,y,j,v,base,p;
    for(i=0;i<63;i++) {
        x=1+(i%9)*2;y=3+(i/9)*2;v=board[i];p=v&1?6:v&4?(v&2?1:4):0;base=v&1?T_WALL:v&4?T_CRATE:32;
        for(j=0;j<4;j++)cell(x+(j&1),y+(j>>1),base==T_CRATE?base+j:base,p);
        if((v&2)&&!(v&4))cell(x,y,T_GEM,1);
    }
    object(0,T_PERSON,(1+(player%9)*2)*8+4,(3+(player/9)*2)*8+4,5);
    hud("CRATES");text(0,1,"NO.",6);number(4,1,level+1,2,4);text(8,1,"MOVES",6);number(14,1,moves,3,0);
}
static uint8_t solved(void) {uint8_t i;for(i=0;i<63;i++)if((board[i]&6)==2)return 0;return 1;}
void crate_escape(void) BANKED {
    uint8_t won=0,replay=1,redraw;int8_t d;int16_t target,beyond;
    level=choose_puzzle(9,level,24);if(level==255){level=0;return;}score=0;
    while(replay) {
        memcpy(board,crate_levels[level],63);player=crate_starts[level];undo_valid=0;moves=0;won=0;
        clear_screen(0);hide_objects();render();flush();
        while(!won) {
            if(!game_frame())return;d=0;redraw=0;
            if(pressed&J_LEFT)d=-1;if(pressed&J_RIGHT)d=1;if(pressed&J_UP)d=-9;if(pressed&J_DOWN)d=9;
            if((pressed&J_A)&&undo_valid){memcpy(board,undo_board,63);player=undo_player;undo_valid=0;if(moves)moves--;redraw=1;sfx(0);}
            if(pressed&J_B){memcpy(board,crate_levels[level],63);player=crate_starts[level];undo_valid=0;moves=0;redraw=1;}
            if(d) {
                target=(int16_t)player+d;beyond=target+d;
                if(target>=0&&target<63&&!(board[target]&1)&&(!(board[target]&4)||(beyond>=0&&beyond<63&&!(board[beyond]&5)))) {
                    memcpy(undo_board,board,63);undo_player=player;undo_valid=1;
                    if(board[target]&4){board[target]&=~4;board[beyond]|=4;sfx(1);}
                    player=target;if(moves<999)moves++;redraw=1;
                }
            }
            if(redraw)render();flush();won=solved();
        }
        add_score(moves>=800?200:1000-moves);render();flush();
        replay=result_action("CRATES DELIVERED!","NEXT PUZZLE AWAITS","A NEXT PUZZLE");
        if(replay)level=(level+1)%24;
    }
}
