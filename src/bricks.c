#pragma bank 3
#include "arcade.h"
static uint8_t bricks[45];
static void wall(uint8_t stage) {
    uint8_t i;
    clear_screen(0);memset(bricks,1,45);
    for(i=0;i<45;i++){cell(1+(i%9)*2,3+i/9,T_BRICK,1+(i/9)%5);cell(2+(i%9)*2,3+i/9,T_BRICK,1+(i/9)%5);}
    text(0,1,"STAGE",6);number(6,1,stage,2,0);
}
void brick_flight(void) BANKED {
    uint8_t lives,stage,remaining,launched,i,r,c,idx,speed;
    int16_t px,fx,fy,bx,by,oldy;
    int8_t dx,dy;
    do {
        score=0;lives=3;stage=1;remaining=45;launched=0;px=68;fx=76*16;fy=112*16;dx=12;dy=-22;speed=22;
        hide_objects();wall(stage);
        while(lives) {
            if(!game_frame())return;
            if((keys&J_LEFT)&&px>4)px-=3;if((keys&J_RIGHT)&&px<132)px+=3;
            if(!launched){fx=(px+8)*16;fy=112*16;if(pressed&J_A){launched=1;dx=12;dy=-speed;sfx(0);}}
            else {
                oldy=fy/16;fx+=dx;fy+=dy;bx=fx/16;by=fy/16;
                if(bx<2){fx=2*16;dx=-dx;}if(bx>150){fx=150*16;dx=-dx;}
                if(by<16){fy=16*16;dy=-dy;}
                if(dy>0 && oldy<=116 && by>=114 && by<125 && bx+6>=px && bx<=px+23) {
                    fy=114*16;dy=-speed;dx=(bx+3-px-12)*3;if(dx>-5&&dx<5)dx=dx<0?-5:5;sfx(0);
                }
                if(by>=20 && by<64 && bx+3>=8 && bx+3<152) {
                    r=(by+3-24)/8;c=(bx+3-8)/16;
                    if(by+3>=24 && r<5 && c<9) {
                        idx=r*9+c;
                        if(bricks[idx]) {bricks[idx]=0;remaining--;dy=-dy;cell(1+c*2,3+r,32,0);cell(2+c*2,3+r,32,0);add_score(10);sfx(1);}
                    }
                }
                if(by>136){lives--;launched=0;sfx(2);}
            }
            if(!remaining){stage++;remaining=45;launched=0;add_score(100);if(speed<38)speed+=2;wall(stage);sfx(3);}
            for(i=0;i<3;i++)object(i,T_PADDLE,px+i*8,120,5);
            object(3,T_BALL,fx/16,fy/16,4);
            hud("BRICKS");text(11,1,"BALLS",6);number(17,1,lives,1,2);flush();
        }
    }while(result("FLIGHT COMPLETE","THE WALL CAN WAIT."));
}
