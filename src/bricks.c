#pragma bank 3
#include "arcade.h"
static uint8_t bricks[24],kind[24],lives,stage,remaining,launched,charges,energy,combo,caught;
static int16_t px,fx,fy;
static int8_t dx,dy,gravity;
static uint16_t frame;
static void paint(uint8_t i){uint8_t x=1+(i%6)*3,y=4+i/6,p=kind[i]==3?2:bricks[i]>1?3:5;cell(x,y,bricks[i]?T_BRICK:32,p);cell(x+1,y,bricks[i]?T_BRICK:32,p);}
static void wall(void){uint8_t i;clear_screen(0);hide_objects();remaining=24;for(i=0;i<24;i++){kind[i]=(i+stage)%7==0?3:(i+stage)%3==0?2:1;bricks[i]=kind[i]==2?2:1;paint(i);}launched=caught=0;gravity=1;combo=0;}
static void hit(uint8_t start){
    uint8_t queue[24],head=0,tail=1,i,j,r,c;queue[0]=start;
    if(bricks[start]>1){bricks[start]--;paint(start);sfx(0);return;}
    /* Mark on enqueue: each brick enters the explosion queue at most once. */
    bricks[start]=0;
    while(head<tail){
        i=queue[head++];remaining--;if(combo<20)combo++;add_score(10*combo);energy=energy<88?energy+12:100;
        if(kind[i]==3){
            if(charges<4)charges++;r=i/6;c=i%6;
            for(j=0;j<24;j++)if(bricks[j]&&((j/6==r&&(j%6+1==c||j%6==c+1))||(j%6==c&&(j/6+1==r||j/6==r+1)))){bricks[j]=0;queue[tail++]=j;}
        }
        paint(i);
    }
    sfx(1);
}
void brick_flight(void) BANKED {
    int16_t bx,by,oldy;uint8_t i,idx,won;
    do {
        score=0;lives=3;stage=1;charges=2;energy=100;px=68;frame=0;won=0;wall();
        while(lives&&!won){
            if(!game_frame())return;frame++;
            if(keys&J_LEFT&&px>3)px-=3;if(keys&J_RIGHT&&px<132)px+=3;
            if(!launched){fx=(px+8)*16;fy=112*16;if(pressed&J_A){launched=1;caught=0;dx=10;dy=-30;sfx(0);}}
            else {
                if(pressed&J_A&&charges){charges--;gravity=-gravity;sfx(3);}
                oldy=fy/16;
                if(!(frame&3)){dy+=gravity;if(dy>42)dy=42;if(dy< -42)dy=-42;}
                fx+=dx;fy+=dy;bx=fx/16;by=fy/16;
                if(bx<2){fx=32;dx=-dx;}if(bx>150){fx=2400;dx=-dx;}
                if(by<24){fy=24*16;dy=dy<0?-dy:dy;}
                if(dy>0&&oldy<=114&&by>=113&&by<124&&bx+6>=px&&bx<=px+23){
                    combo=0;fy=112*16;
                    if(keys&J_B&&energy>=12){energy-=12;launched=0;caught=1;}else{dy=-32;dx=(bx+3-px-12)*3;if(dx>-5&&dx<5)dx=dx<0?-5:5;}
                    sfx(0);
                }
                if(by+3>=32&&by+3<64&&bx+3>=8&&bx+3<152){
                    idx=((by+3-32)/8)*6+(bx+3-8)/24;
                    if(idx<24&&bricks[idx]&&(bx+3-8)%24<16){hit(idx);dy=-dy;fy=(oldy<by?((idx/6)*8+24):((idx/6)*8+40))*16;}
                }
                if(by>136){lives--;launched=0;combo=0;sfx(2);}
            }
            if(!remaining){add_score(300);if(stage==3)won=1;else{stage++;charges=charges<3?charges+1:4;wall();}}
            for(i=0;i<3;i++)object(i,T_PADDLE,px+i*8,120,keys&J_B&&energy>=12?3:5);
            object(3,T_BALL,fx/16,fy/16,gravity>0?4:3);
            hud("RICOCHET");text(0,1,"BALL",6);number(4,1,lives,1,2);text(7,1,"ROOM",6);number(12,1,stage,1,4);text(15,1,gravity>0?"DOWN":"UP  ",3);
            text(0,2,"FLIP",6);number(5,2,charges,1,3);text(8,2,"MAG",6);number(12,2,energy,3,5);text(16,2,"X",6);number(18,2,combo,2,4);
            text(1,16,!launched?(caught?"CAUGHT! A LAUNCH   ":"A LAUNCH          "):"A FLIP B MAGNET   ",6);flush();
        }
    }while(result(won?"REACTORS CLEARED!":"OUT OF BALLS",won?"A PERFECT REACTION":"BEND THE NEXT SHOT"));
}
