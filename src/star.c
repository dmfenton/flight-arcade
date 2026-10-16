#pragma bank 1
#include "arcade.h"
static int16_t ex[6], ey[6], bx[5], by[5], sx[4], sy[4];
static int8_t drift[6];
static void field(void) {
    uint8_t x,y;
    clear_screen(0);
    for(y=2;y<16;y++)for(x=0;x<20;x++)if(random8(16)==0)cell(x,y,T_DOT,6);
}
void star_skimmer(void) BANKED {
    uint8_t i,j,hull,invuln,cooldown,spawn,k;
    uint16_t frame;
    int16_t px,py;
    do {
        score=0;hull=3;invuln=0;cooldown=0;spawn=35;frame=0;px=76;py=112;
        for(i=0;i<6;i++)ey[i]=-20;
        for(i=0;i<5;i++)by[i]=-20;
        for(i=0;i<4;i++)sy[i]=160;
        hide_objects();field();
        while(hull) {
            if(!game_frame())return;
            frame++;if(invuln)invuln--;
            if(keys&J_LEFT && px>4)px-=2;if(keys&J_RIGHT && px<148)px+=2;
            if(keys&J_UP && py>32)py-=2;if(keys&J_DOWN && py<120)py+=2;
            if(cooldown)cooldown--;
            if((keys&J_A) && !cooldown)for(i=0;i<5;i++)if(by[i]<0){bx[i]=px+2;by[i]=py-5;cooldown=10;sfx(0);break;}
            if(spawn)spawn--;
            else {
                for(i=0;i<6;i++)if(ey[i]<0){ex[i]=8+random8(136);ey[i]=18;drift[i]=random8(2)?1:-1;break;}
                spawn=score>1200?18:score>500?26:38;
            }
            for(i=0;i<5;i++)if(by[i]>=0) {
                by[i]-=3;
                for(j=0;j<6;j++)if(ey[j]>=0 && bx[i]+3>=ex[j] && bx[i]<ex[j]+8 && by[i]<ey[j]+8 && by[i]+6>=ey[j]) {
                    ey[j]=-20;by[i]=-20;add_score(25);sfx(1);break;
                }
            }
            for(i=0;i<6;i++)if(ey[i]>=0) {
                if(!(frame&1))ey[i]++;
                if(!(frame&3)){ex[i]+=drift[i];if(ex[i]<4 || ex[i]>148)drift[i]=-drift[i];}
                if((frame+i*13)%100==0 && ey[i]<96)for(k=0;k<4;k++)if(sy[k]>144){sx[k]=ex[i]+2;sy[k]=ey[i]+8;break;}
                if(!invuln && ex[i]<px+7 && ex[i]+7>px && ey[i]<py+7 && ey[i]+7>py) {hull--;invuln=90;ey[i]=-20;sfx(2);}
                if(ey[i]>128)ey[i]=-20;
            }
            for(i=0;i<4;i++)if(sy[i]<145) {
                sy[i]+=2;
                if(!invuln && sx[i]<px+7 && sx[i]+4>px && sy[i]<py+7 && sy[i]+6>py){hull--;invuln=90;sy[i]=160;sfx(2);}
            }
            object(0,T_SHIP,px,invuln && (frame&4)?160:py,5);
            for(i=0;i<6;i++)object(1+i,T_ALIEN,ex[i],ey[i],2);
            for(i=0;i<5;i++)object(7+i,T_BULLET,bx[i],by[i],4);
            for(i=0;i<4;i++)object(12+i,T_BULLET,sx[i],sy[i],2);
            hud("SKIMMER");text(0,1,"HULL",6);number(5,1,hull,1,2);flush();
        }
    } while(result("MISSION COMPLETE","THE STARS REMEMBER"));
}
