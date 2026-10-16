"""Play the eight added games through the actual ROM, using only joypad writes."""
from verify import Game,NAMES,ROOT,OUT,STATIC,SYMBOLS,ROM_SHA,progress,trace
from playtest import move_cursor
from collections import deque
from functools import lru_cache
import json
import math
import time

RESULTS=[]
SOLUTIONS=json.loads((OUT/'puzzle-solutions.json').read_text())
def signed(v):return v if v<128 else v-256
def dword(g,name):
    return sum(v<<(i*8)for i,v in enumerate(g.array(name,4)))
def done(g,index,**checks):
    g.shot(f'{index+1:02d}-result');checks.update(game=NAMES[index],score=g.word('_score'),emulated_frames=g.frames)
    RESULTS.append(checks);trace('expanded_game_verified',**checks);print('PASS',json.dumps(checks),flush=True)
    g.close();progress('expanded_playtests',8+len(RESULTS))

SHAPES=[(0x0f00,0x2222,0x00f0,0x4444),(0x6600,)*4,(0x4e00,0x4640,0x0e40,0x4c40),(0x6c00,0x4620,0x06c0,0x8c40),(0xc600,0x2640,0x0c60,0x4c80),(0x8e00,0x6440,0x0e20,0x44c0),(0x2e00,0x4460,0x0e80,0xc440)]
def block_drop():
    g=Game();g.enter(8);placed=0
    for _ in range(180):
        if 'STACK COMPLETE' in g.row(3):break
        board=g.array('drop._field',160);piece=g.byte('drop._piece');candidates=[]
        for rotation,shape in enumerate(SHAPES[piece]):
            cells=[(x,y)for y in range(4)for x in range(4)if shape&(0x8000>>(y*4+x))]
            def fits(x,y):return all(0<=x+dx<10 and 0<=y+dy<16 and not board[(y+dy)*10+x+dx]for dx,dy in cells)
            for x in range(-2,10):
                if not fits(x,0):continue
                y=0
                while fits(x,y+1):y+=1
                test=board[:]
                for dx,dy in cells:test[(y+dy)*10+x+dx]=1
                rows=[test[y*10:y*10+10]for y in range(16)];cleared=sum(all(row)for row in rows)
                rows=[[0]*10 for _ in range(cleared)]+[row for row in rows if not all(row)]
                heights=[];holes=0
                for column in range(10):
                    top=next((r for r in range(16)if rows[r][column]),16);heights.append(16-top)
                    holes+=sum(not rows[r][column]for r in range(top,16))
                value=cleared*1000-holes*65-sum(heights)*3-sum(abs(a-b)for a,b in zip(heights,heights[1:]))*3-max(heights)*7
                candidates.append((value,rotation,x))
        if not candidates:break
        _,rotation,x=max(candidates)
        current=g.byte('drop._rotation')
        for _ in range((rotation-current)%4):g.tap('a',1)
        for _ in range(12):
            actual=signed(g.byte('drop._piece_x'))
            if actual==x:break
            g.tap('right'if actual<x else 'left',1)
        g.tap('up',1);placed+=1
        if placed==10:g.shot('09-action')
        if g.word('drop._lines')>=8:break
    assert g.word('drop._lines')>=4,f'Block rows: {g.word("drop._lines")}, pieces={placed}, score={g.word("_score")}'
    g.shot('09-action');g.pause_test(8);g.exit_test()
    done(g,8,rotation_and_hard_drop=True,rows_cleared=g.word('drop._lines'),pieces_placed=placed,pause_and_menu=True)

def crate_escape():
    g=Game();g.enter(9);level0=SOLUTIONS['crate'][0]
    initial=g.array('crates._board',63);player=g.byte('crates._player')
    g.tap(level0['solution'][0]);g.tap('a')
    assert g.array('crates._board',63)==initial and g.byte('crates._player')==player and g.word('crates._moves')==0
    g.tap(level0['solution'][0]);g.tap('b')
    assert g.array('crates._board',63)==initial and g.byte('crates._player')==player
    for i,puzzle in enumerate(SOLUTIONS['crate']):
        assert g.array('crates._board',63)==puzzle['board']
        for direction in puzzle['solution']:g.tap(direction,1)
        g.wait(lambda:'CRATES DELIVERED' in g.row(3),f'crate puzzle {i+1} solved')
        if i in (0,12,23):g.shot(f'10-puzzle-{i+1:02d}-solved')
        if i<23:g.tap('a');g.wait(lambda:'START PAUSE' in g.row(17),'next crate puzzle')
    g.shot('10-result');g.tap('b');g.wait(lambda:'FLIGHT ARCADE' in g.row(1),'crate menu')
    high=g.array('_best',32);ram=g.close();h=Game(ram)
    assert h.array('_best',32)==high and h.word_at(STATIC['core._best']+9*2)>20000
    h.close();RESULTS.append({'game':NAMES[9],'all_24_puzzles_solved':True,'undo_and_restart':True,'expanded_scores_persist':True})
    print('PASS all 24 Crate Escape puzzles and durable new-game scores',flush=True)

def flip_pairs():
    g=Game();g.enter(10);cards=g.array('pairs._cards',24);cursor=[0,0]
    assert sorted(cards)==sorted(list(range(12))*2)
    g.tap('a');g.tap('b');assert g.byte('pairs._first')==255
    other=next(i for i,v in enumerate(cards)if v!=cards[0])
    g.tap('a');move_cursor(g,cursor,other%6,other//6);g.tap('a')
    assert g.byte('pairs._second')!=255
    g.shot('11-mismatch');g.wait(lambda:g.byte('pairs._first')==255,'mismatched cards turn back',180)
    assert sum(g.array('pairs._matched',24))==0
    for value in range(12):
        for i,v in enumerate(cards):
            if v==value:move_cursor(g,cursor,i%6,i//6);g.tap('a')
        if value==5:g.shot('11-action')
    g.wait(lambda:'ALL PAIRED UP!' in g.row(3),'all twelve pairs')
    assert sum(g.array('pairs._matched',24))==24 and g.word('_score')==844
    done(g,10,cancel=True,mismatch_hides=True,all_12_pairs=True,turns=g.word('pairs._turns'))

def path_to(board,start,goals):
    q=deque([(start,[])]);seen={start}
    while q:
        p,path=q.popleft()
        if p in goals:return path
        for d,key in [(-1,'left'),(1,'right'),(-15,'up'),(15,'down')]:
            n=p+d
            if 0<=n<195 and board[n]!=1 and n not in seen:seen.add(n);q.append((n,path+[key]))
    raise AssertionError('Maze target is unreachable')
def maze_hopper():
    g=Game();g.enter(11);board=g.array('maze._maze',195);seen=g.array('maze._seen',195)
    assert sum(v==2 for v in board)==5
    hidden=next(i for i,v in enumerate(board)if not seen[i] and v!=1)
    tile=SYMBOLS['_tiles']+(hidden//15+3)*20+hidden%15+2
    assert g.b.memory[tile]==32
    g.hold('a');g.tick(20);assert g.b.memory[tile]!=32 and g.array('maze._seen',195)==seen
    g.shot('12-map');g.hold();g.tick(20);assert g.b.memory[tile]==32
    while g.byte('maze._collected')<5:
        board=g.array('maze._maze',195);goals={i for i,v in enumerate(board)if v==2}
        path=path_to(board,g.byte('maze._player'),goals)
        for key in path:g.tap(key,1)
        if g.byte('maze._collected')==3:g.shot('12-action')
    path=path_to(g.array('maze._maze',195),g.byte('maze._player'),{178})
    for key in path:g.tap(key,1)
    g.wait(lambda:'THE GATE IS OPEN' in g.row(3),'maze gate win')
    assert g.word('_score')==300
    done(g,11,all_gems_reachable=True,fog_and_hold_map=True,five_gems_and_gate_win=True)

def peg_trails():
    g=Game();g.enter(12);cursor=[2,2]
    def jump(source,target):
        move_cursor(g,cursor,source%5,source//5);g.tap('a');move_cursor(g,cursor,target%5,target//5);g.tap('a')
    original=dword(g,'pegs._pegs');jump(*SOLUTIONS['peg'][0]['solution'][0]);g.tap('b')
    assert dword(g,'pegs._pegs')==original
    for level,puzzle in enumerate(SOLUTIONS['peg']):
        assert dword(g,'pegs._pegs')==puzzle['mask']
        for source,target in puzzle['solution']:jump(source,target)
        g.wait(lambda:'ONE PEG REMAINS!' in g.row(3),f'peg puzzle {level+1} solved')
        assert dword(g,'pegs._pegs')==1<<12
        if level in (0,19):g.shot(f'13-puzzle-{level+1:02d}-solved')
        if level<19:g.tap('a');g.wait(lambda:'START PAUSE' in g.row(17),'next peg puzzle');cursor=[2,2]
    done(g,12,all_20_puzzles_solved=True,undo=True,one_peg_finish=True)

def reversi_moves(board,who):
    result={}
    for p,v in enumerate(board):
        if v:continue
        captures=[]
        for dy in (-1,0,1):
            for dx in (-1,0,1):
                if not(dx or dy):continue
                x=p%6+dx;y=p//6+dy;line=[]
                while 0<=x<6 and 0<=y<6 and board[y*6+x]==3-who:line.append(y*6+x);x+=dx;y+=dy
                if line and 0<=x<6 and 0<=y<6 and board[y*6+x]==who:captures+=line
        if captures:
            after=board[:];after[p]=who
            for cell in captures:after[cell]=who
            result[p]=after
    return result
def reversi_value(board):
    value=(board.count(1)-board.count(2))*(8 if board.count(0)<8 else 1)
    for corner in (0,5,30,35):value+=100 if board[corner]==1 else -100 if board[corner]==2 else 0
    return value+3*(len(reversi_moves(board,1))-len(reversi_moves(board,2)))
def minimax(board,who,depth):
    moves=reversi_moves(board,who)
    if not depth:return reversi_value(board)
    if not moves:
        if not reversi_moves(board,3-who):return (board.count(1)-board.count(2))*1000
        return minimax(board,3-who,depth-1)
    values=[minimax(after,3-who,depth-1)for after in moves.values()]
    return max(values)if who==1 else min(values)
def orbit_discs():
    g=Game();g.enter(13);initial=g.array('discs._board',36);g.tap('b');assert not g.byte('discs._show_hints');g.tap('b');assert g.byte('discs._show_hints')
    turns=0
    for _ in range(50):
        if any(s in g.row(3)for s in ['TAKE THE ORBIT','TAKES ORBIT','AN EVEN ORBIT']):break
        board=g.array('discs._board',36);moves=reversi_moves(board,1)
        if not moves:g.tick(30);continue
        choice=max(moves,key=lambda p:minimax(moves[p],2,2))
        cursor=g.byte('discs._cursor');xy=[cursor%6,cursor//6];move_cursor(g,xy,choice%6,choice//6);g.tap('a');g.tick(20);turns+=1
        if turns==5:g.shot('14-action')
    board=g.array('discs._board',36)
    assert not reversi_moves(board,1)and not reversi_moves(board,2),'Both sides must finish legally'
    green=board.count(1);red=board.count(2)
    assert g.word('_score')==green*10+(500 if green>red else 0)
    done(g,13,hints_toggle=True,complete_legal_match=True,human_discs=green,cpu_discs=red,human_turns=turns)

def golf_solution(cards,stock,limit=400000):
    visited=set();calls=0
    def adjacent(a,b):return abs(a-b)==1 or {a,b}=={1,13}
    def visit(counts,draw,waste):
        nonlocal calls
        if not any(counts):return []
        state=counts+(draw,waste)
        if state in visited:return None
        visited.add(state);calls+=1
        if calls>limit:raise OverflowError
        options=[c for c,n in enumerate(counts)if n and adjacent(cards[c*5+n-1]%13+1,waste)]
        options.sort(key=lambda c:counts[c])
        for c in options:
            n=counts[c]-1;next_counts=counts[:c]+(n,)+counts[c+1:]
            path=visit(next_counts,draw,cards[c*5+n]%13+1)
            if path is not None:return [c]+path
        if draw<17:
            path=visit(counts,draw+1,stock[draw]%13+1)
            if path is not None:return ['draw']+path
        return None
    try:return visit((5,)*7,1,stock[0]%13+1)
    except OverflowError:return None
def golf_solitaire():
    solved=None
    for seed in range(16):
        progress('searching_a_solvable_golf_deal',14)
        g=Game();g.tick(seed*7);g.enter(14);cards=g.array('golf._stack_cards',35);stock=g.array('golf._stock',17)
        assert sorted(cards+stock)==list(range(52))
        solved=golf_solution(cards,stock)
        if solved is not None:break
        g.close()
    assert solved is not None,'Find an honestly dealt, solvable card game'
    draws=plays=0
    for action in solved:
        if action=='draw':
            before=g.byte('golf._stock_pos');g.tap('b');assert g.byte('golf._stock_pos')==before+1;draws+=1
        else:
            current=g.byte('golf._cursor')
            while current!=action:g.tap('right');current=(current+1)%7
            count=g.array('golf._counts',7)[action];g.tap('a');assert g.array('golf._counts',7)[action]==count-1;plays+=1
            if plays==12:g.shot('15-action')
    g.wait(lambda:'EVERY STACK CLEAR' in g.row(3),'all seven card stacks cleared')
    assert plays==35 and g.word('_score')==850
    done(g,14,standard_52_card_deck=True,all_35_tableau_cards_cleared=True,draws=draws,solvable_deal_attempt=seed+1)

def lunar_lander():
    g=Game();g.enter(15);landings=0
    for mission in range(2):
        for _ in range(1400):
            if 'TOUCHDOWN!' in g.row(3)or'A ROUGH LANDING'in g.row(3):break
            height=120-(g.word('lander._fy')/16+8);velocity=signed(g.byte('lander._vy'))
            target=max(3,min(14,int(height*0.35)))
            pad=g.byte('lander._pad');width=g.byte('lander._width');x=g.word('lander._fx')/16;vx=signed(g.byte('lander._vx'))
            center=pad*8+width*4-4;desired=max(-12,min(12,(center-x)*0.8))
            buttons=[]
            if velocity>target:buttons.append('a')
            if vx<desired-1:buttons.append('right')
            if vx>desired+1:buttons.append('left')
            g.hold(*buttons);g.tick()
        g.hold();g.shot(f'16-landing-{mission+1}')
        assert 'TOUCHDOWN!'in g.row(3),f'Lander result: {g.row(3)}, pos={g.word("lander._fx")/16,g.word("lander._fy")/16}, velocity={signed(g.byte("lander._vx")),signed(g.byte("lander._vy"))}'
        assert g.byte('lander._fuel')<100 and abs(signed(g.byte('lander._vy')))<=15
        landings+=1
        if mission==0:g.tap('a');g.wait(lambda:'START PAUSE'in g.row(17),'next landing')
    done(g,15,soft_landings=landings,fuel_consumption=True,second_pad=True)

def word_at(self,addr):return self.b.memory[addr]|self.b.memory[addr+1]<<8
Game.word_at=word_at

if __name__=='__main__':
    started=time.monotonic()
    try:
        for test in [block_drop,crate_escape,flip_pairs,maze_hopper,peg_trails,orbit_discs,golf_solitaire,lunar_lander]:test()
    except Exception:
        progress('expanded_playtests_failed',8+len(RESULTS),'failed','PLAYTEST_ASSERTION');raise
    finally:
        (OUT/'expanded-playtest-results.json').write_text(json.dumps({'rom_sha256':ROM_SHA,'results':RESULTS,'elapsed_seconds':round(time.monotonic()-started,2)},indent=2))
    progress('all_16_games_verified',16)
    print('All eight added games passed extended playtests.',flush=True)
