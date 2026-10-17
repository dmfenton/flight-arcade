#pragma bank 1
#include "arcade.h"
static int16_t ex[6],ey[6],bx[8],by[8],sx[8],sy[8],lx[4],ly[4];
static int8_t drift[6],shot_dx[8];
static int16_t px,py,boss_x;
static uint8_t hull,sector,gun,heat,salvage,boss_hp,overheated,kills,invuln,cooldown,spawn;
static uint16_t frame;
static void field(void) {
    uint8_t i,x,y;
    hide_objects();clear_screen(0);
    for(y=4;y<16;y++)for(x=0;x<20;x++)if(random8(18)==0)cell(x,y,T_DOT,6);
    for(i=0;i<6;i++)ey[i]=-20;
    for(i=0;i<8;i++){by[i]=-20;sy[i]=160;}
    for(i=0;i<4;i++)ly[i]=160;
    boss_hp=kills=heat=overheated=cooldown=0;spawn=20;boss_x=72;frame=0;
}
static void enemy_shot(int16_t x,int16_t y,int8_t dx) {
    uint8_t i;for(i=0;i<8;i++)if(sy[i]>144){sx[i]=x;sy[i]=y;shot_dx[i]=dx;break;}
}
static void hurt(void){if(!invuln){hull--;invuln=90;sfx(2);}}
static uint8_t near(int16_t a,int16_t b){return a<b+7&&a+7>b;}
static void render(void) {
    uint8_t i;
    object(0,T_SHIP,px,invuln&&(frame&4)?160:py,5);
    for(i=0;i<6;i++)object(1+i,T_ALIEN,ex[i],ey[i],2);
    for(i=0;i<8;i++){object(7+i,T_BULLET,bx[i],by[i],4);object(15+i,T_BULLET,sx[i],sy[i],2);}
    for(i=0;i<4;i++)object(23+i,T_GEM,lx[i],ly[i],4);
    for(i=0;i<4;i++)object(27+i,T_ALIEN,boss_x+(i&1)*8,boss_hp?34+(i/2)*8:160,3);
    hud("SKIMMER");text(0,1,"H",6);number(1,1,hull,1,1);text(4,1,"SEC",6);number(7,1,sector,1,4);
    text(10,1,"GUN",6);number(13,1,gun,1,5);text(15,1,"SCR",4);number(18,1,salvage,2,4);
    text(0,2,overheated?"OVERHEAT":"HEAT    ",overheated?2:6);
    for(i=0;i<10;i++)cell(9+i,2,heat>i*10?T_FILL:T_DOT,overheated?2:4);
    if(boss_hp){text(0,3,"BOSS    ",3);number(9,3,boss_hp,2,3);}else {text(0,3,"HOSTILES",6);number(9,3,8+sector*2-kills,2,6);}
    flush();
}
void star_skimmer(void) BANKED {
    uint8_t i,j,k,pick,won;
    do {
        score=0;hull=5;sector=gun=1;salvage=4;invuln=0;px=76;py=112;won=0;field();
        while(hull&&!won) {
            if(!game_frame())return;frame++;if(invuln)invuln--;if(cooldown)cooldown--;
            if(keys&J_LEFT&&px>4)px-=2;if(keys&J_RIGHT&&px<148)px+=2;
            if(keys&J_UP&&py>50)py-=2;if(keys&J_DOWN&&py<120)py+=2;
            if(pressed&J_B&&salvage>=2){salvage-=2;invuln=90;sfx(3);}
            if(heat){if(!(keys&J_A)||overheated)heat=heat>2?heat-2:0;else if(!(frame&1))heat--;}
            if(overheated&&heat<=30)overheated=0;
            if(keys&J_A&&!cooldown&&!overheated)for(i=0;i<8;i++)if(by[i]<0){
                bx[i]=px+2;by[i]=py-5;cooldown=14-gun*2;heat+=12;if(heat>=100){heat=100;overheated=1;}sfx(0);break;
            }
            if(!boss_hp&&kills<8+sector*2){
                if(spawn)spawn--;else {for(i=0;i<6;i++)if(ey[i]<0){ex[i]=8+random8(136);ey[i]=30;drift[i]=random8(2)?1:-1;break;}spawn=40-sector*4;}
            }
            for(i=0;i<8;i++)if(by[i]>=0){
                by[i]-=4;
                if(boss_hp&&bx[i]+4>boss_x&&bx[i]<boss_x+16&&by[i]<50){
                    by[i]=-20;boss_hp=boss_hp>gun?boss_hp-gun:0;add_score(15);
                    if(!boss_hp){
                        add_score(250*sector);sfx(3);
                        if(sector==3){won=1;break;}
                        pick=choice("SECTOR CLEAR","CHOOSE YOUR REFIT","FASTER GUN","REPAIR +2 HULL");if(pick==255)return;
                        if(pick==0){if(gun<4)gun++;}else hull=hull<4?hull+2:5;
                        sector++;px=76;py=112;invuln=90;field();break;
                    }
                }else for(j=0;j<6;j++)if(ey[j]>=0&&near(bx[i],ex[j])&&near(by[i],ey[j])){
                    for(k=0;k<4;k++)if(ly[k]>144){lx[k]=ex[j];ly[k]=ey[j];break;}
                    ey[j]=by[i]=-20;kills++;add_score(25);sfx(1);break;
                }
            }
            if(won)break;
            if(!boss_hp&&kills>=8+sector*2){boss_hp=10+sector*4;for(i=0;i<6;i++)ey[i]=-20;}
            if(boss_hp){boss_x=32+(frame/2)%96;if(frame%36==0){enemy_shot(boss_x,49,-1);enemy_shot(boss_x+8,49,1);}}
            for(i=0;i<6;i++)if(ey[i]>=0){
                if(!(frame&1))ey[i]++;if(!(frame&3)){ex[i]+=drift[i];if(ex[i]<4||ex[i]>148)drift[i]=-drift[i];}
                if((frame+i*13)%95==0)enemy_shot(ex[i],ey[i]+8,0);
                if(near(ex[i],px)&&near(ey[i],py))hurt();if(ey[i]>128)ey[i]=-20;
            }
            for(i=0;i<8;i++)if(sy[i]<=144){sy[i]+=2;sx[i]+=shot_dx[i];if(near(sx[i],px)&&near(sy[i],py)){hurt();sy[i]=160;}}
            for(i=0;i<4;i++)if(ly[i]<=144){if(!(frame&1))ly[i]++;if(near(lx[i],px)&&near(ly[i],py)){ly[i]=160;if(salvage<99)salvage++;add_score(10);sfx(1);}}
            render();
        }
    }while(result(won?"THREE SECTORS SAFE":"SHIP LOST",won?"SALVAGE WELL SPENT":"COOL. LOOT. FLY."));
}
