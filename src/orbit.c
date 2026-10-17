#pragma bank 7
#include "arcade.h"
static int16_t fx,fy,cx[3],cy[3];
static int8_t vx,vy,cvx[3],cvy[3],tether;
static uint8_t hull,fuel,sector,delivered,total,active[3],engine,line_strength,invuln;
static uint16_t frame;
static int16_t absolute(int16_t v){return v<0?-v:v;}
static int8_t limited(int16_t v,int8_t cap){return v>cap?cap:v< -cap?-cap:v;}
static uint8_t dock(int16_t x,int16_t y){return x>=8&&x<=30&&y>=55&&y<=91;}
static uint8_t core(int16_t x,int16_t y){return absolute(x-77)<11&&absolute(y-77)<11;}
static void field(void){
    uint8_t x,y,i;clear_screen(0);hide_objects();
    for(y=4;y<16;y++)for(x=0;x<20;x++)if(random8(20)==0)cell(x,y,T_DOT,6);
    panel(1,7,4,5,5);text(1,12,"DOCK",5);box(9,9,3,3,2);cell(10,10,T_STAR,2);
    for(i=0;i<3;i++){cx[i]=(i==0?48:i==1?124:108)*16;cy[i]=(i==0?72:i==1?48:112)*16;cvx[i]=cvy[i]=0;active[i]=1;}
    fx=20*16;fy=76*16;vx=vy=0;tether=-1;delivered=0;frame=0;invuln=60;
}
static void recover(uint8_t i){active[i]=0;delivered++;total++;tether=-1;add_score(200+fuel);fuel=fuel<170?fuel+30:200;sfx(3);}
void orbit_salvage(void) BANKED {
    uint8_t i,pick,thrust,won;int16_t x,y,tx,ty;int8_t pullx,pully;
    do{
        score=0;hull=4;fuel=200;sector=1;total=0;engine=line_strength=0;won=0;field();
        while(hull&&!won){
            if(!game_frame())return;frame++;if(invuln)invuln--;thrust=0;
            if(fuel){
                if(keys&J_LEFT){vx--;thrust=1;}if(keys&J_RIGHT){vx++;thrust=1;}
                if(keys&J_UP){vy--;thrust=1;}if(keys&J_DOWN){vy++;thrust=1;}
                if(thrust&&frame%(engine?10:6)==0)fuel--;
                if(keys&J_B){vx=vx*7/8;vy=vy*7/8;if(frame%8==0&&fuel)fuel--;}
            }
            x=fx/16;y=fy/16;
            if(pressed&J_A){
                if(tether>=0)tether=-1;
                else for(i=0;i<3;i++)if(active[i]&&absolute(cx[i]/16-x)+absolute(cy[i]/16-y)<38){tether=i;sfx(0);break;}
            }
            /* The core accelerates the ship. Cargo has inertia and spring tension,
               with equal opposite tug on the ship; heavier sectors pull harder. */
            if(frame%4==0&&!dock(x,y)){vx+=x<77?1:-1;vy+=y<77?1:-1;}
            if(tether>=0){
                i=tether;tx=cx[i]/16-x;ty=cy[i]/16-y;
                pullx=limited(tx/(line_strength?5:8),3);pully=limited(ty/(line_strength?5:8),3);
                cvx[i]-=pullx;cvy[i]-=pully;
                if(frame%(4-sector)==0){vx+=pullx;vy+=pully;}
            }
            vx=limited(vx,28);vy=limited(vy,28);fx+=vx;fy+=vy;
            if(fx<0){fx=0;vx=-vx/2;}if(fx>152*16){fx=152*16;vx=-vx/2;}
            if(fy<32*16){fy=32*16;vy=-vy/2;}if(fy>124*16){fy=124*16;vy=-vy/2;}
            for(i=0;i<3;i++)if(active[i]){
                /* Light passive damping keeps unattended cargo recoverable. */
                if(frame%6==0){if(cvx[i])cvx[i]+=cvx[i]>0?-1:1;if(cvy[i])cvy[i]+=cvy[i]>0?-1:1;}
                cvx[i]=limited(cvx[i],24);cvy[i]=limited(cvy[i],24);cx[i]+=cvx[i];cy[i]+=cvy[i];
                if(cx[i]<0){cx[i]=0;cvx[i]=-cvx[i];}if(cx[i]>152*16){cx[i]=152*16;cvx[i]=-cvx[i];}
                if(cy[i]<32*16){cy[i]=32*16;cvy[i]=-cvy[i];}if(cy[i]>124*16){cy[i]=124*16;cvy[i]=-cvy[i];}
                if(core(cx[i]/16,cy[i]/16)){cx[i]=(i==0?48:124)*16;cy[i]=(40+i*32)*16;cvx[i]=cvy[i]=0;if((uint8_t)tether==i){tether=-1;if(hull)hull--;sfx(2);}}
                if((uint8_t)tether==i&&dock(cx[i]/16,cy[i]/16)&&absolute(cvx[i])+absolute(cvy[i])<=24)recover(i);
            }
            x=fx/16;y=fy/16;
            if(core(x,y)&&!invuln){hull--;invuln=90;fx=20*16;fy=76*16;vx=vy=0;tether=-1;sfx(2);}
            if(dock(x,y)&&!thrust&&fuel<200&&frame%4==0)fuel++;
            if(!fuel&&!dock(x,y)){hull=0;}
            if(delivered==3){
                if(sector==3){won=1;add_score(500);}
                else{pick=choice("SALVAGE SECURED","HEAVIER PODS AHEAD","ECONOMY THRUST","STRONGER TETHER");if(pick==255)return;
                    if(pick==0)engine=1;else line_strength=1;sector++;field();}
            }
            object(0,T_SHIP,fx/16,invuln&&(frame&4)?160:fy/16,5);
            for(i=0;i<3;i++)object(1+i,T_GEM,cx[i]/16,active[i]?cy[i]/16:160,(uint8_t)tether==i?4:6);
            for(i=0;i<3;i++)object(4+i,T_DOT,tether>=0?(fx+(cx[tether]-fx)*(i+1)/4)/16:0,tether>=0?(fy+(cy[tether]-fy)*(i+1)/4)/16:160,4);
            hud("SALVAGE");text(0,1,"H",6);number(1,1,hull,1,1);text(3,1,"FUEL",6);number(7,1,fuel,3,5);text(12,1,"PODS",6);number(17,1,total,1,4);text(18,1,"/9",6);
            text(0,2,"SECTOR",6);number(7,2,sector,1,3);text(10,2,tether>=0?"TOWING  ":"RELEASED",4);text(0,16,"A TETHER   B BRAKE",6);flush();
        }
    }while(result(won?"SALVAGE MASTER!":"ORBIT LOST",won?"NINE PODS HOME":"EASE THE TETHER"));
}
