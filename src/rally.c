#pragma bank 7
#include "arcade.h"
void orbit_rally(void) BANKED {
    int16_t py,ay,fx,fy,bx,by;
    int8_t dx,dy;
    uint8_t player,opponent,serve,i;
    uint16_t frame;
    do {
        score=0;player=opponent=0;py=ay=64;fx=76*16;fy=72*16;dx=24;dy=13;serve=1;frame=0;
        clear_screen(0);hide_objects();for(i=3;i<16;i+=2)cell(10,i,T_DOT,6);
        while(player<7 && opponent<7) {
            if(!game_frame())return;frame++;
            if((keys&J_UP)&&py>24)py-=2;if((keys&J_DOWN)&&py<104)py+=2;
            if(serve){fx=20*16;fy=(py+8)*16;if(pressed&J_A){serve=0;dx=24;dy=random8(2)?15:-15;}}
            else {
                fx+=dx;fy+=dy;bx=fx/16;by=fy/16;
                if(by<20){fy=20*16;dy=-dy;}if(by>124){fy=124*16;dy=-dy;}
                if(dx<0 && bx<=16 && bx>=10 && by+6>=py && by<=py+24) {
                    fx=17*16;dx=dx>-42?(-dx)+1:43;dy=(by+3-py-12)*2;if(dy>-6&&dy<6)dy=6;add_score(5);sfx(0);
                }
                if(dx>0 && bx>=136 && bx<=144 && by+6>=ay && by<=ay+24){fx=135*16;dx=-dx;dy=(by+3-ay-12)*2;if(dy>-6&&dy<6)dy=-6;sfx(0);}
                if(bx<0){opponent++;serve=1;sfx(2);}if(bx>154){player++;add_score(100);serve=1;sfx(1);}
            }
            if((frame&3)!=0) {
                if(ay+12<fy/16+3-3 && ay<104)ay++;
                if(ay+12>fy/16+3+3 && ay>24)ay--;
            }
            for(i=0;i<3;i++){object(i,T_WALL,8,py+i*8,5);object(3+i,T_WALL,144,ay+i*8,2);}
            object(6,T_BALL,fx/16,fy/16,4);
            hud("RALLY");text(1,1,"YOU",5);number(5,1,player,1,5);text(12,1,"CPU",2);number(16,1,opponent,1,2);
            text(3,16,serve?"A SERVE         ":"FIRST TO 7 WINS",6);flush();
        }
        if(player==7)add_score(500);
    }while(result(player==7?"ORBIT CHAMPION!":"REMATCH?",player==7?"STELLAR PLAY!":"THE ORB RETURNS."));
}
