"""Input-only campaigns and mechanic checks against the compiled CGB ROM.

Bots read live state to choose joypad input. They never write game RAM.
"""
from collections import deque
import json,sys
from verify import Game,ROOT,OUT,ROM_SHA,NAMES,progress,trace,atlas,PRIVATE,STATIC
from verify_navigation import audit_choice,audit_result,fork

def signed(v):return v-256 if v>127 else v
def sw(g,n):
    v=g.word(n);return v-65536 if v>32767 else v
def words(g,n,count):
    a=g.array(n,count*2);return [signed_word(a[i]|a[i+1]<<8)for i in range(0,len(a),2)]
def signed_word(v):return v-65536 if v>32767 else v
def result(g):return 'B ARCADE MENU' in g.row(16)
def choice(g):return 'A PICK' in g.row(16)
def move_toward(value,target,negative,positive,margin=2):
    return [negative]if value>target+margin else [positive]if value<target-margin else []
def step(g,keys=(),n=1):g.hold(*keys);g.tick(n)
def check(g,predicate,label):
    if not predicate:
        idx=g.byte('_selected_game');prefix=['star','drop','bricks','rally','river','snake','orbit','lander'][idx]+'.'
        (PRIVATE/f'failure-{idx}.json').write_text(json.dumps(dict(label=label,frames=g.frames,scalars={n:[g.byte(n),g.word(n)]for n in STATIC if n.startswith(prefix)},rows=[g.row(y)for y in range(18)]),indent=2))
        with (PRIVATE/f'failure-{idx}.state').open('wb')as f:g.b.save_state(f)
    assert predicate,f'{ROOT}/build/FlightArcade.gbc: {label}; rows={[g.row(y)for y in range(18)]}'
def finish(g,index,details,won=None):
    audit_result(g,index);details['navigation']=g.navigation
    g.hold();g.shot(f'{index+1:02d}-depth');details['score']=g.word('_score');details['frames']=g.frames
    if won is not None:details['campaign_won']=won
    trace('depth_pass',game=NAMES[index],**details)
    print('PASS',NAMES[index],details,flush=True)
    return details

def star(g):
    shield_checked=False;resting=False
    maxheat=0;overheat=False;cool=False;refits=[];previous_sector=1
    for tick in range(18000):
        if result(g):break
        if choice(g):
            audit_choice(g,0,'refit');oldgun=g.byte('star._gun');oldh=g.byte('star._hull')
            pick=0 if len(refits)==0 else 1
            if pick:g.tap('down',1)
            g.tap('a',1);refits.append(pick)
            if pick==0:check(g,g.byte('star._gun')==oldgun+1,'gun upgrade applies')
            else:check(g,g.byte('star._hull')==min(5,oldh+2),'repair applies')
            continue
        px=sw(g,'star._px');py=sw(g,'star._py');fr=g.word('star._frame');hp=g.byte('star._boss_hp')
        ex=words(g,'star._ex',6);ey=words(g,'star._ey',6)
        if hp:
            ahead=(py-45)//4;target=32+((fr+ahead)//2)%96+4
        else:
            candidates=[i for i in range(6)if ey[i]>=0]
            target=ex[max(candidates,key=lambda i:ey[i])]if candidates else 76
        # Maintain a firing lane while avoiding incoming bullets close to the hull.
        sx=words(g,'star._sx',8);sy=words(g,'star._sy',8)
        danger=[x for x,y in zip(sx,sy)if py-18<y<py+8 and abs(x-px)<11]
        danger += [x for x,y in zip(ex,ey)if py-22<y<py+8 and abs(x-px)<14]
        if danger and not g.byte('star._invuln'):target=px+22 if px<128 else px-22
        ly=words(g,'star._ly',4);lx=words(g,'star._lx',4)
        loot=[i for i in range(4)if 80<ly[i]<122 and abs(lx[i]-px)<35]
        if loot and not hp:target=lx[loot[0]]
        if cool:
            if g.byte('star._heat')>75:resting=True
            if g.byte('star._heat')<20:resting=False
        fire=[]if resting or g.byte('star._overheated')else ['a']
        keys=fire+move_toward(px,target,'left','right',1)+move_toward(py,90 if hp else 112,'up','down',1)
        if g.byte('star._heat')>90:maxheat=max(maxheat,g.byte('star._heat'))
        if g.byte('star._overheated'):overheat=True
        if overheat and not g.byte('star._overheated'):cool=True
        if (danger or g.byte('star._hull')<=2) and g.byte('star._salvage')>=2 and not g.byte('star._invuln')and tick%2==0:
            old=g.byte('star._salvage');g.hold(*keys);g.tap('b',1)
            check(g,g.byte('star._salvage')==old-2,'shield consumes two salvage');shield_checked=True;continue
        step(g,keys)
    check(g,result(g)and 'THREE SECTORS SAFE' in g.row(3),'three bosses won by joypad')
    check(g,shield_checked and overheat and cool and refits==[0,1],'heat recovery and both refit outcomes')
    return finish(g,0,dict(shield_cost=2,overheated=overheat,recovered=cool,refits=refits,sectors=3),True)

SHAPES=[(0x0F00,0x2222,0x00F0,0x4444),(0x6600,)*4,(0x4E00,0x4640,0x0E40,0x4C40),(0x6C00,0x4620,0x06C0,0x8C40),(0xC600,0x2640,0x0C60,0x4C80),(0x8E00,0x6440,0x0E20,0x44C0),(0x2E00,0x4460,0x0E80,0xC440)]
def cells(piece,rot):return [(x,y)for y in range(4)for x in range(4)if SHAPES[piece][rot]&(0x8000>>(y*4+x))]
def placement(board,shape,x,y):return all(0<=x+a<10 and 0<=y+b<16 and not board[(y+b)*10+x+a]for a,b in shape)
def best_drop(board,piece):
    best=None
    for rot in range(4):
        shape=cells(piece,rot)
        for x in range(-3,10):
            if not placement(board,shape,x,0):continue
            y=0
            while placement(board,shape,x,y+1):y+=1
            new=board[:]
            for a,b in shape:new[(y+b)*10+x+a]=1
            rows=[new[i:i+10]for i in range(0,160,10)];cleared=sum(all(r)for r in rows)
            rows=[[0]*10 for _ in range(cleared)]+[r for r in rows if not all(r)]
            heights=[];holes=0
            for col in range(10):
                seen=False;h=0
                for row in range(16):
                    if rows[row][col]:
                        if not seen:h=16-row
                        seen=True
                    elif seen:holes+=1
                heights.append(h)
            value=cleared*6-holes*8-sum(heights)*.7-sum(abs(a-b)for a,b in zip(heights,heights[1:]))*.4-max(heights)*1.5
            if best is None or value>best[0]:best=(value,rot,x)
    return best

def drop(g):
    garbage_seen=vents=multi=0;pieces=0;previous_lines=0
    for _ in range(240):
        if result(g):break
        board=g.array('drop._field',160)
        if 8 in board:garbage_seen+=1
        plan=best_drop(board,g.byte('drop._piece'));check(g,plan is not None,'reachable drop placement')
        _,rot,x=plan
        for _ in range(rot):g.tap('a',1)
        for _ in range(12):
            px=signed(g.byte('drop._piece_x'))
            if px==x:break
            g.tap('left'if px>x else 'right',1)
        if 8 in board and g.byte('drop._coolant')>=3:
            old=g.byte('drop._coolant');oldlines=g.word('drop._lines');g.tap('b',1);vents+=1
            check(g,8 not in g.array('drop._field',160),'vent drops and removes garbage rows')
            check(g,g.byte('drop._coolant')>=old-3,'vent charge accounted with row rewards')
        else:g.tap('up',1)
        pieces+=1
        current=g.word('drop._lines');multi+=current-previous_lines>=2;previous_lines=current
    check(g,result(g)and 'REACTOR STABLE!'in g.row(3),'24 rows and all three zones cooled')
    check(g,garbage_seen and vents and multi,'pressure, vent, and multi-row rewards exercised')
    return finish(g,1,dict(lines=g.word('drop._lines'),pieces=pieces,vents=vents,multi_clears=multi),True)

def bricks(g):
    flips=catches=0;previous_caught=0;maxcombo=0;launch_wait=0
    for tick in range(26000):
        if result(g):break
        bx=sw(g,'bricks._fx')/16;px=sw(g,'bricks._px');launched=g.byte('bricks._launched')
        maxcombo=max(maxcombo,g.byte('bricks._combo'))
        if not launched:
            remaining=g.array('bricks._bricks',24);target=8+(next((i for i,v in enumerate(remaining)if v),0)%6)*24
            # Launch from varied lanes; avoid a vertical repeating orbit.
            target=max(3,min(132,target-4))
            keys=move_toward(px,target,'left','right',2)
            if not keys:keys=['a']if tick%2==0 else []
        else:
            keys=move_toward(px,max(3,min(132,bx-(4 if tick//240%2 else 17))),'left','right',1)
            if g.byte('bricks._energy')>=12:keys+=['b']
            if tick>80 and tick%600==0 and g.byte('bricks._charges'):
                old=signed(g.byte('bricks._gravity'));g.hold();g.tap('a',1);flips+=1
                check(g,signed(g.byte('bricks._gravity'))==-old,'gravity direction reverses');continue
        caught=g.byte('bricks._caught')
        if caught and not previous_caught:catches+=1
        previous_caught=caught
        step(g,keys)
    check(g,result(g)and 'REACTORS CLEARED!'in g.row(3),'all three ricochet rooms cleared')
    check(g,flips>=2 and catches and maxcombo>=2,'gravity, magnetic catch, and chain reaction used')
    return finish(g,2,dict(rooms=3,gravity_flips=flips,magnetic_catches=catches,max_chain=maxcombo),True)

def rally(g):
    g.tap('b',1);check(g,g.byte('rally._flux')==2 and g.byte('rally._reversed')>0,'reverse field spends flux')
    spin_seen=False;dash_seen=False;wells=set()
    for tick in range(18000):
        if result(g):break
        py=sw(g,'rally._py');by=sw(g,'rally._fy')/16;bx=sw(g,'rally._fx')/16
        if g.byte('rally._serve'):
            keys=['a']if tick%2==0 else []
        else:
            # Strike with the edge of a moving paddle for deliberate spin.
            target=max(24,min(104,by-(4 if by<76 else 20)))
            keys=move_toward(py,target,'up','down',0)
            if signed(g.byte('rally._dx'))<0 and bx<50 and not g.byte('rally._cooldown'):keys.append('a')
        step(g,keys);spin_seen|=signed(g.byte('rally._spin'))!=0;dash_seen|=g.byte('rally._dash')>0;wells.add(g.byte('rally._well'))
    check(g,result(g)and g.byte('rally._player')==7,'first-to-seven rally won')
    check(g,spin_seen and dash_seen and wells=={0,1},'spin, dash, and alternating wells')
    return finish(g,3,dict(player=7,opponent=g.byte('rally._opponent'),spin=spin_seen,dash=dash_seen,wells=2),True)

def river(g):
    step(g,(),24);v=signed(g.byte('river._vx'));check(g,v!=0,'current produces drift without input')
    oldpower=g.byte('river._power');step(g,['b'],8)
    check(g,abs(signed(g.byte('river._vx')))<abs(v)and g.byte('river._power')<oldpower,'drag reduces drift and spends power')
    forks=[];boost=False
    for tick in range(8000):
        if result(g):break
        if choice(g):
            audit_choice(g,4,'fork');pick=len(forks)%2
            if pick:g.tap('down',1)
            g.tap('a',1);forks.append(pick);continue
        data=g.array('river._river',252);fr=g.word('river._frame');dist=g.word('river._distance');start_fx=sw(g,'river._fx');start_v=signed(g.byte('river._vx'));timer=g.byte('river._flow_timer');cur=signed(g.byte('river._current'));power=g.byte('river._power');route=g.byte('river._route')
        actions=[(),('left',),('right',),('b',),('left','b'),('right','b')]
        def forecast(action,state,steps):
            fx,v,t,offset,p,c,dt,cost=state
            for _ in range(steps):
                dt+=1;f=fr+dt
                if 'left'in action:v-=2
                if 'right'in action:v+=2
                if f%4==0:
                    if v:v+=-1 if v>0 else 1
                    v+=c
                if 'b'in action and p:
                    v=int(v/2)
                    if f%4==0:p-=1
                elif f%4==0 and p<100:p+=1
                v=max(-28,min(28,v));fx=max(8*16,min(144*16,fx+v));interval=12 if 'b'in action and p else 4 if route else 7
                t+=1
                if t>=interval:
                    t=0;offset+=1
                    if (dist+offset)%20==0:c=-c
                col=max(0,min(17,int((fx/16-8+4)/8)))
                for row in (12-offset,13-offset):
                    if 0<=row<14:
                        tile=data[row*18+col]
                        if tile in (106,115):cost+=10000
                        if tile==108:cost-=2
                cost+=abs(fx/16-80)*.002
            return fx,v,t,offset,p,c,dt,cost
        initial=(start_fx,start_v,timer,0,power,cur,0,0)
        plans=[]
        for action in actions:
            one=forecast(action,initial,8)
            for second in actions:
                end=forecast(second,one,12)
                plans.append((end[-1]-end[3]*.2+(100-end[4])*.002,action))
        keys=list(min(plans,key=lambda plan:plan[0])[1])
        if tick<12:keys=['a'];boost=True
        step(g,keys)
    check(g,result(g)and 'ESTUARY REACHED!'in g.row(3),'three river reaches completed')
    check(g,forks==[0,1]and boost,'both branch decisions and rowing used')
    return finish(g,4,dict(reaches=3,forks=forks,rescues=g.byte('river._rescues'),current_drift=True,drag=True),True)

DIRS=[(1,0,'right'),(0,1,'down'),(-1,0,'left'),(0,-1,'up')]
def serpent(g):
    # Prove the timed phase cost, then let it expire through normal moves.
    g.hold('right');g.wait(lambda:g.byte('snake._xs')==17,'approach phase wall',100);g.hold();
    old=g.byte('snake._power');g.tap('b',1);check(g,g.byte('snake._power')==old-3 and g.byte('snake._phase')>0,'four-step phase purchase')
    g.hold('right');g.wait(lambda:g.byte('snake._xs')==1,'phase through wall',50);g.hold();
    check(g,g.byte('snake._phase')>0,'wall crossing preserves live phase')
    picks=[];gates=set();last_turn=-1
    for tick in range(15000):
        if result(g):break
        if choice(g):
            audit_choice(g,5,'coil-choice');pick=len(picks)%2
            if pick:g.tap('down',1)
            g.tap('a',1);picks.append(pick);last_turn=-1;continue
        turn=g.word('snake._turn');gates.add(g.byte('snake._gate_on'))
        if turn==last_turn:g.tick();continue
        last_turn=turn;length=g.byte('snake._length');xs=g.array('snake._xs',length);ys=g.array('snake._ys',length)
        head=(xs[0],ys[0]);target=(g.byte('snake._fx'),g.byte('snake._fy'));direction=g.byte('snake._dir');occupied=set(zip(xs[:-1],ys[:-1]));phase=g.byte('snake._phase')
        gate=g.byte('snake._gate_on')and g.byte('snake._arena')>1
        q=deque([(head,direction,[])]);seen={(head,direction)};found=None
        while q:
            pos,d,path=q.popleft()
            if pos==target:found=path;break
            for nd,(dx,dy,key)in enumerate(DIRS):
                if (nd-d)%4==2:continue
                x,y=pos[0]+dx,pos[1]+dy
                if not(1<=x<=18 and 3<=y<=15):continue
                if (x,y)in occupied:continue
                if gate and x in (5,14)and 7<=y<=11:continue
                state=((x,y),nd)
                if state not in seen:seen.add(state);q.append(((x,y),nd,path+[key]))
        if found:g.hold(found[0])
        else:
            # Large open arenas leave a safe local move if a tail temporarily blocks the fruit.
            moves=[k for nd,(dx,dy,k)in enumerate(DIRS)if (nd-direction)%4!=2 and 1<=head[0]+dx<=18 and 3<=head[1]+dy<=15 and (head[0]+dx,head[1]+dy)not in occupied]
            check(g,bool(moves),'serpent escape route');g.hold(moves[0])
        g.tick()
    check(g,result(g)and 'PULSE MASTER!'in g.row(3),'three serpent arenas harvested')
    check(g,picks==[0,1]and gates=={0,1},'both coil choices and pulsing gates')
    return finish(g,5,dict(arenas=3,phase_cost=3,phase_wall_crossing=True,choices=picks,pulsing_gates=True),True)

def orbit(g):
    tethered=False;momentum=False;refits=[];previous=(-1,-1);waypoint=None
    for tick in range(22000):
        if result(g):break
        if choice(g):
            audit_choice(g,6,'refit');pick=len(refits)%2
            if pick:g.tap('down',1)
            g.tap('a',1);refits.append(pick);continue
        x=sw(g,'orbit._fx')/16;y=sw(g,'orbit._fy')/16;vx=signed(g.byte('orbit._vx'));vy=signed(g.byte('orbit._vy'))
        cx=words(g,'orbit._cx',3);cy=words(g,'orbit._cy',3);active=g.array('orbit._active',3);t=signed(g.byte('orbit._tether'))
        if t>=0:
            tethered=True
            # Route cargo above the core before bringing it down into the dock.
            if cx[t]/16>52:target=(34,42)if cy[t]/16<76 else (34,116)
            else:target=(22,76)
        else:
            if 8<=x<=30 and 55<=y<=91 and g.byte('orbit._fuel')<190:
                step(g,['b']if abs(vx)+abs(vy)>3 else []);continue
            if not any(active):step(g,());continue
            i=min((i for i in range(3)if active[i]),key=lambda i:abs(cx[i]/16-x)+abs(cy[i]/16-y))
            target=(cx[i]/16,cy[i]/16)
            if abs(target[0]-x)+abs(target[1]-y)<34:
                g.tap('a',1);continue
            if min(x,target[0])<77<max(x,target[0])and min(y,target[1])<90 and max(y,target[1])>60:target=(target[0],42 if y<77 else 116)
        ex=target[0]-x;ey=target[1]-y;dvx=max(-15,min(15,ex*1.5));dvy=max(-15,min(15,ey*1.5))
        keys=[]
        if vx<dvx-1:keys+=['right']
        elif vx>dvx+1:keys+=['left']
        if vy<dvy-1:keys+=['down']
        elif vy>dvy+1:keys+=['up']
        if abs(ex)+abs(ey)<16 and abs(vx)+abs(vy)>14:keys+=['b']
        momentum|=abs(vx)+abs(vy)>0;step(g,keys)
    check(g,result(g)and 'SALVAGE MASTER!'in g.row(3),'nine pods delivered through spring physics')
    check(g,tethered and momentum and refits==[0,1],'tether, inertia, and both refits')
    return finish(g,6,dict(pods=g.byte('orbit._total'),refits=refits,tether=True,momentum=True),True)

def lander(g):
    # First launch carries ore. Jettison proves the mass/reward tradeoff.
    g.tap('a',1);oldmass=g.byte('lander._mass')
    heavy=fork(g);light=fork(g)
    try:
        light.tap('b',1);end=light.word('lander._frame')+16
        light.wait(lambda:light.word('lander._frame')>=end,'empty coast')
        heavy.wait(lambda:heavy.word('lander._frame')>=end,'heavy coast')
        check(g,signed(heavy.byte('lander._vy'))>signed(light.byte('lander._vy')),'heavy cargo physically accelerates faster')
    finally:heavy.close();light.close()
    g.tap('b',1)
    check(g,oldmass==3 and g.byte('lander._mass')==0 and not g.byte('lander._intact'),'jettison removes mass and cargo reward')
    jobs=0;fuel_at=[];mass_seen={0,3};wind_seen=set();previous_delivery=0
    for tick in range(14000):
        if result(g):break
        if choice(g):
            audit_choice(g,7,'contract');fuel_at.append(g.byte('lander._fuel'));pick=jobs%2
            if pick:g.tap('down',1)
            g.tap('a',1);jobs+=1;mass_seen.add(g.byte('lander._mass'));g.shot('08-contract-play');continue
        if g.byte('lander._docked'):g.tap('a',1);continue
        x=sw(g,'lander._fx')/16;y=sw(g,'lander._fy')/16;vx=signed(g.byte('lander._vx'));vy=signed(g.byte('lander._vy'));target=g.byte('lander._target');tx=[16,72,136][target]
        ex=tx-x;desired_vx=max(-20,min(20,ex*1.5));keys=[]
        if vx<desired_vx-1:keys+=['right']
        elif vx>desired_vx+1:keys+=['left']
        ty=60 if abs(ex)>9 else 113
        desired_vy=max(-18,min(9,(ty-y)*1.5))
        if vy>desired_vy:keys+=['up']
        wind_seen.add(signed(g.byte('lander._wind')));step(g,keys)
    check(g,result(g)and 'ROUTE COMPLETE!'in g.row(3),'five contracts delivered with real landing physics')
    check(g,{0,1,3}.issubset(mass_seen),'empty, fragile, and heavy cargo flown')
    check(g,any(f<180 for f in fuel_at),'fuel persists across jobs')
    return finish(g,7,dict(deliveries=g.byte('lander._deliveries'),mass_states=sorted(mass_seen),mass_changes_acceleration=True,jettison=True,fuel_between_jobs=fuel_at,wind=sorted(wind_seen)),True)

TESTS=[star,drop,bricks,rally,river,serpent,orbit,lander]
def run(indices=range(8)):
    results=[]
    for i in indices:
        progress('depth_playtest',len(results));g=Game()
        try:g.enter(i);details=TESTS[i](g);results.append(dict(game=NAMES[i],**details))
        finally:ram=g.close()
        saved=Game(ram)
        try:
            slot=json.loads((ROOT/'src/games.json').read_text())[i]['save_slot'];raw=saved.array('_best',32)
            assert raw[slot*2]|raw[slot*2+1]<<8==details['score'],f'{ROOT}/build/FlightArcade.gbc: completed score did not persist'
            results[-1]['score_persisted']=True
        finally:saved.close()
    if list(indices)==list(range(8)):
        (OUT/'playtest-results.json').write_text(json.dumps(dict(rom_sha256=ROM_SHA,results=results,input_only=True),indent=2))
        for first in (0,4):atlas(range(first,first+4),f'depth-{first+1}-{first+4}.png','depth')
        progress('depth_playtest_passed',8)
    return results
if __name__=='__main__':run([int(v)for v in sys.argv[1:]]if len(sys.argv)>1 else range(8))
