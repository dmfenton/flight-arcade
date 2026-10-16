"""End-to-end playtests. All changes to game state go through the emulated joypad."""
from verify import Game, NAMES, ROOT, OUT, STATIC, SYMBOLS, ROM_SHA, progress, trace, atlas
from collections import deque
import json
import random
import time

RESULTS=[]
def finish(g,index,checks):
    g.shot(f'{index+1:02d}-action')
    checks.update(game=NAMES[index],emulated_frames=g.frames,score=g.word('_score'))
    trace('game_verified',**checks);RESULTS.append(checks)
    print('PASS',json.dumps(checks),flush=True)
    g.close();progress('extended_playtests',len(RESULTS))

def shooter():
    g=Game();g.enter(0)
    for step in range(3000):
        if 'MISSION' in g.row(3):break
        px=g.b.memory[0xFE01]-8
        enemies=[]
        for i in range(6):
            y=g.word_at(STATIC['star._ey']+i*2)
            if 18<=y<105:enemies.append((y,g.word_at(STATIC['star._ex']+i*2)))
        target=max(enemies,default=(0,76))[1]
        direction='right' if px<target-2 else 'left' if px>target+2 else None
        g.hold('a',*([direction] if direction else []));g.tick()
    assert g.word('_score')>=125,'Shooter must destroy enemies and award points'
    g.hold();g.shot('01-action')
    if 'MISSION' in g.row(3):g.tap('b');g.wait(lambda:'FLIGHT ARCADE' in g.row(1),'shooter result to menu')
    else:g.exit_test()
    high=g.array('_best',16);ram=g.close()
    h=Game(ram);assert h.array('_best',16)==high,'High scores must survive power off'
    assert h.word('_best')>=125
    h.tap('b');sound=h.byte('_sound_on');ram2=h.close()
    h=Game(ram2);assert h.byte('_sound_on')==sound,'Sound preference must persist';h.close()
    RESULTS.append({'game':NAMES[0],'shooting_and_enemy_hits':True,'saved_best_after_restart':True,'saved_sound_after_restart':True,'emulated_frames':g.frames})
    print('PASS Star Skimmer and durable saves',flush=True)

def snake():
    g=Game();g.enter(1)
    turns=0
    while g.word('_score')<60 and turns<250:
        length=g.byte('snake._length');xs=g.array('snake._xs',length);ys=g.array('snake._ys',length)
        head=(xs[0],ys[0]);goal=(g.byte('snake._fx'),g.byte('snake._fy'))
        blocked=set(zip(xs[:-1],ys[:-1]));q=deque([(head,[])]);seen={head};path=None
        while q:
            pos,steps=q.popleft()
            if pos==goal:path=steps;break
            for dx,dy,key in [(1,0,'right'),(0,1,'down'),(-1,0,'left'),(0,-1,'up')]:
                nxt=(pos[0]+dx,pos[1]+dy)
                if 1<=nxt[0]<=18 and 3<=nxt[1]<=15 and nxt not in blocked and nxt not in seen:
                    seen.add(nxt);q.append((nxt,steps+[key]))
        if not path:
            g.shot('02-bot-blocked')
            raise AssertionError(f'Snake bot route: head={head}, goal={goal}, body={list(zip(xs,ys))}, score={g.word("_score")}, screen={g.row(3)}')
        g.hold(path[0]);g.wait(lambda:(g.byte('snake._xs'),g.byte('snake._ys'))!=head,'snake advances')
        g.tick(1)
        turns+=1
    assert g.word('_score')>=60
    assert g.byte('snake._length')==10,'Each of six fruits grows the snake once'
    g.hold();g.shot('02-action')
    g.hold('right');g.wait(lambda:'THE GARDEN RESTS' in g.row(3),'snake wall collision',800)
    g.tap('b');g.wait(lambda:'FLIGHT ARCADE' in g.row(1),'snake game over to menu')
    finish(g,1,{'fruit_collection_and_growth':True,'wall_collision_and_game_over':True})

def bricks():
    g=Game();g.enter(2)
    stage2=False;previous_ball=None
    for frame in range(30000):
        if 'FLIGHT COMPLETE' in g.row(3):break
        px=g.b.memory[0xFE01]-8;bx=g.b.memory[0xFE0D]-8;by=g.b.memory[0xFE0C]-16
        remaining=[i for i,v in enumerate(g.array('bricks._bricks',45))if v]
        hit=remaining[(frame//240)%len(remaining)] if remaining else 0
        vx=bx-previous_ball[0]if previous_ball else 0;vy=by-previous_ball[1]if previous_ball else 0
        predicted=bx+vx*max(0,114-by)/vy if vy>0 else bx
        while predicted<2 or predicted>150:predicted=4-predicted if predicted<2 else 300-predicted
        desired_dx=max(-27,min(27,((hit%9)*16+12-predicted)*22/(114-(24+hit//9*8))))
        target=max(2,min(134,predicted-9-desired_dx/3))
        previous_ball=(bx,by)
        direction='right' if px<target-2 else 'left' if px>target+2 else None
        g.hold(*(['a'] if frame%30<15 else []),*([direction] if direction else []));g.tick()
        if '02' in g.row(1)[:9]:stage2=True;break
    if not stage2:
        g.shot('03-bot-failure')
        raise AssertionError(f'Bricks stage 2: score={g.word("_score")}, remaining={sum(g.array("bricks._bricks",45))}, rows={[g.row(i)for i in (0,1,3,5,14)]}, OAM={list(g.b.memory[0xFE00:0xFE10])}')
    assert g.word('_score')>=550
    finish(g,2,{'paddle_ball_and_brick_collisions':True,'cleared_first_wall':True,'stage_2':True})

def merge():
    g=Game();g.enter(3)
    start=g.array('merge._board',16);old_score=g.word('_score')
    for key in ['left','up','right','down']:
        g.tap(key)
        if g.array('merge._board',16)!=start:break
    assert g.array('merge._board',16)!=start
    g.tap('b');assert g.array('merge._board',16)==start and g.word('_score')==old_score,'Undo restores the board and score before spawning'
    rng=random.Random(72);moves=0;merges=0
    for _ in range(600):
        before=g.array('merge._board',16);before_score=g.word('_score')
        if 'FINISH' in g.row(1):break
        g.tap(rng.choice(['left','up','right','down']))
        after=g.array('merge._board',16);after_score=g.word('_score')
        if before!=after:
            before_sum=sum(1<<v for v in before if v);after_sum=sum(1<<v for v in after if v)
            assert after_sum-before_sum in [2,4],'A valid move adds exactly one new 2 or 4 tile'
            assert after_score>=before_score
            moves+=1;merges+=after_score>before_score
    assert merges>=10 and g.word('_score')>=100
    finish(g,3,{'undo_restores_board_and_score':True,'tile_value_conservation':True,'valid_moves':moves,'merges':merges})

def move_cursor(g,cursor,x,y):
    while cursor[0]!=x:
        delta=1 if cursor[0]<x else -1;g.tap('right' if delta==1 else 'left');cursor[0]+=delta
    while cursor[1]!=y:
        delta=1 if cursor[1]<y else -1;g.tap('down' if delta==1 else 'up');cursor[1]+=delta

def mines():
    g=Game();g.enter(4);cursor=[4,3]
    before=g.array('mines._ground',63)
    g.tap('start');g.wait(lambda:'PAUSED' in g.row(4),'mines pause')
    g.tap('a');g.wait(lambda:'START PAUSE' in g.row(17),'mines resume')
    assert g.array('mines._ground',63)==before,'Resume must not reveal or flag a cell'
    g.tap('a');ground=g.array('mines._ground',63)
    assert sum(bool(v&4)for v in ground)==10
    assert not any(ground[y*9+x]&4 for y in range(2,5)for x in range(3,6)),'First pick and its neighbors must be mine free'
    assert g.byte('mines._revealed')>1,'An empty first cell must flood reveal'
    mine=next(i for i,v in enumerate(ground)if v&4)
    move_cursor(g,cursor,mine%9,mine//9);g.tap('b');assert g.array('mines._ground',63)[mine]&2
    g.tap('b');assert not(g.array('mines._ground',63)[mine]&2)
    for i in range(63):
        value=g.array('mines._ground',63)[i]
        if not(value&5):move_cursor(g,cursor,i%9,i//9);g.tap('a')
    assert g.byte('mines._revealed')==53
    assert 'CLEAR!' in g.row(1)
    finish(g,4,{'safe_first_pick':True,'flood_reveal':True,'flag_and_unflag':True,'resume_has_no_action':True,'all_53_safe_cells_revealed':True,'win':True})

def solve_lights(state):
    rows=[]
    for target in range(25):
        mask=0
        for press in range(25):
            affected=[press]
            if press%5:affected.append(press-1)
            if press%5<4:affected.append(press+1)
            if press>=5:affected.append(press-5)
            if press<20:affected.append(press+5)
            if target in affected:mask|=1<<press
        rows.append(mask|(state[target]<<25))
    pivots=[];r=0
    for c in range(25):
        found=next((i for i in range(r,25)if rows[i]>>c&1),None)
        if found is None:continue
        rows[r],rows[found]=rows[found],rows[r]
        for i in range(25):
            if i!=r and rows[i]>>c&1:rows[i]^=rows[r]
        pivots.append(c);r+=1
    assert all(row&((1<<25)-1) or not(row>>25) for row in rows),'Generated puzzle must have a solution'
    return [c for i,c in enumerate(pivots)if rows[i]>>25&1]

def lights():
    g=Game();g.enter(5);cursor=[2,2]
    original=g.array('lights._lights',25);g.tap('a');assert g.array('lights._lights',25)!=original
    g.tap('b');assert g.array('lights._lights',25)==original
    for level in range(10):
        solution=solve_lights(g.array('lights._lights',25))
        for index in solution:move_cursor(g,cursor,index%5,index//5);g.tap('a')
        assert not any(g.array('lights._lights',25)) and 'SOLVED!' in g.row(1)
        if level==0:g.shot('06-solved')
        g.tap('a');assert g.word('_score')>=(level+1)*60
    finish(g,5,{'undo':True,'10_generated_puzzles_solved':True,'next_puzzle_and_score':True})

def rally():
    g=Game();g.enter(6)
    for frame in range(36000):
        if 'ORBIT CHAMPION' in g.row(3) or 'REMATCH?' in g.row(3):break
        py=g.b.memory[0xFE00]-16;by=g.b.memory[0xFE18]-16
        target=max(24,min(104,by-2))
        direction='down' if py<target-1 else 'up' if py>target+1 else None
        g.hold(*(['a']if frame%30<15 else []),*([direction]if direction else []));g.tick()
    if 'ORBIT CHAMPION' not in g.row(3):
        g.shot('07-bot-failure')
        raise AssertionError(f'Rally win: score={g.word("_score")}, rows={[g.row(i)for i in (1,3,5,16)]}')
    assert g.word('_score')>=1200
    finish(g,6,{'serve_and_both_paddles':True,'first_to_7_win':True})

def river():
    attempts=[]
    # The heuristic pilot can lose a random river. Retry normally dealt rivers,
    # varying only the user's time on the menu; never alter emulated game state.
    for delay in (0,7,19,31,63,91):
        g=Game();g.tick(delay);g.enter(7)
        for frame in range(16000):
            if 'RIVER COMPLETE' in g.row(3):break
            grid=g.array('river._river',252);px=g.b.memory[0xFE01]-8;current=(px-8+4)//8
            candidates=[]
            for x in range(18):
                upcoming=[grid[y*18+x]for y in range(8,14)]
                if all(v not in (106,115)for v in upcoming):candidates.append(x)
            if candidates:
                target=min(candidates,key=lambda x:abs(x-current))*8+8
                direction='right'if px<target-1 else 'left'if px>target+1 else None
                g.hold(*(['a']if frame<150 else []),*([direction]if direction else []))
            else:g.hold()
            g.tick()
            if g.word('_score')>=750:break
        attempts.append({'menu_delay_frames':delay,'score':g.word('_score'),'game_over':'RIVER COMPLETE' in g.row(3)})
        trace('river_attempt',**attempts[-1])
        if g.word('_score')>=750:break
        g.close()
    assert g.word('_score')>=750,'Steering, gems, boost and narrowing channel must remain playable'
    assert g.byte('river._channel_width')<12,'The river must actually narrow'
    finish(g,7,{'scrolling_and_steering':True,'boost_then_cruise':True,'gems_and_distance_score':True,'narrow_channel_survival':True,'channel_width':g.byte('river._channel_width'),'attempts':attempts})

def word_at(self,addr):
    value=self.b.memory[addr]|self.b.memory[addr+1]<<8
    return value if value<32768 else value-65536
Game.word_at=word_at

if __name__=='__main__':
    start=time.monotonic()
    try:
        for test in [shooter,snake,bricks,merge,mines,lights,rally,river]:test()
    except Exception:
        progress('extended_playtests_failed',len(RESULTS),'failed','PLAYTEST_ASSERTION')
        raise
    finally:
        (OUT/'playtest-results.json').write_text(json.dumps({'rom_sha256':ROM_SHA,'results':RESULTS,'elapsed_seconds':round(time.monotonic()-start,2)},indent=2))
    progress('extended_playtests_passed',8)
    print('All eight extended playtests passed.',flush=True)
