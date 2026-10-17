#pragma bank 4
#include "arcade.h"
static int16_t py,ay,fx,fy;
static int8_t dx,dy,spin,polarity;
static uint8_t player,opponent,serve,flux,dash,cooldown,reversed,well;
static uint16_t frame;
static int8_t clamp(int16_t v,int8_t limit){return v>limit?limit:v< -limit?-limit:v;}
void orbit_rally(void) BANKED {
    int16_t bx,by,oldpy;uint8_t i;
    do {
        score=0;player=opponent=0;py=ay=64;fx=76*16;fy=72*16;dx=24;dy=13;serve=1;frame=0;flux=3;dash=cooldown=reversed=0;spin=0;
        clear_screen(0);hide_objects();for(i=4;i<16;i+=2)cell(10,i,T_DOT,6);
        while(player<7&&opponent<7){
            if(!game_frame())return;frame++;oldpy=py;
            if(cooldown)cooldown--;if(dash)dash--;if(reversed)reversed--;
            if(pressed&J_B&&flux){flux--;reversed=90;sfx(3);}
            if(!serve&&(pressed&J_A)&&!cooldown){dash=15;cooldown=90;}
            if(keys&J_UP&&py>24)py-=dash?4:2;if(keys&J_DOWN&&py<104)py+=dash?4:2;if(py<24)py=24;if(py>104)py=104;
            well=(frame/180)&1;polarity=reversed?-1:1;
            if(serve){fx=20*16;fy=(py+8)*16;if(pressed&J_A){serve=0;dx=24;dy=random8(2)?15:-15;spin=0;}}
            else{
                bx=fx/16;by=fy/16;
                if(bx>44&&bx<108&&!(frame&1)){dy+=((by<(well?100:48))?polarity:-polarity);dy=clamp(dy,42);}
                if(!(frame&7)&&spin){dy=clamp(dy+spin,42);spin+=spin<0?1:-1;}
                fx+=dx;fy+=dy;bx=fx/16;by=fy/16;
                if(by<24){fy=24*16;dy=-dy;}if(by>124){fy=124*16;dy=-dy;}
                if(dx<0&&bx<=16&&bx>=10&&by+6>=py&&by<=py+24){
                    fx=17*16;dx=dash?48:dx>-38?(-dx)+2:40;spin=(py-oldpy)*2;dy=clamp((by+3-py-12)*2+spin,40);if(dy>-6&&dy<6)dy=6;add_score(dash?25:5);sfx(0);
                }
                if(dx>0&&bx>=136&&bx<=144&&by+6>=ay&&by<=ay+24){fx=135*16;dx=-dx;spin=0;dy=clamp((by+3-ay-12)*2,36);if(dy>-6&&dy<6)dy=-6;sfx(0);}
                if(bx<0||bx>154){if(bx<0){opponent++;sfx(2);}else{player++;add_score(100);sfx(1);}serve=1;if(flux<3)flux++;reversed=0;}
            }
            /* Delayed, limited opponent tracking leaves room for curved and dash shots. */
            if(frame%3){if(ay+12<fy/16&&ay<104)ay++;if(ay+12>fy/16+6&&ay>24)ay--;}
            for(i=0;i<3;i++){object(i,T_WALL,8,py+i*8,dash?3:5);object(3+i,T_WALL,144,ay+i*8,2);}
            object(6,T_BALL,fx/16,fy/16,4);object(7,T_GEM,76,well?100:48,reversed?2:3);
            hud("RALLY");text(0,1,"YOU",5);number(4,1,player,1,5);text(7,1,"CPU",2);number(11,1,opponent,1,2);text(14,1,"FLX",6);number(18,1,flux,1,3);
            text(1,2,cooldown?"DASH COOLING":"DASH READY  ",6);text(1,16,serve?"A SERVE           ":reversed?"FIELD REVERSED    ":"A DASH  B REVERSE ",6);flush();
        }
        if(player==7)add_score(500);
    }while(result(player==7?"GRAVITY CHAMPION!":"REMATCH?",player==7?"SPIN WELL SPENT":"CURVE YOUR RETURN"));
}
