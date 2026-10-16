"""Exercise real ROM navigation using joypad input, including natural end states."""
import json
import random
from verify import Game,NAMES,OUT,ROM_SHA,progress,trace,atlas
from playtest import move_cursor,solve_lights
from playtest_more import SOLUTIONS,path_to,reversi_moves

RESULTS=[]

def menu(g):
    g.wait(lambda:'FLIGHT ARCADE' in g.row(1),'arcade menu')

def playing(g):
    g.wait(lambda:'START PAUSE' in g.row(17),'play screen')

def result_screen(g):
    return 'B ARCADE MENU' in g.row(16)

def choose(g,index,held=False):
    menu(g)
    while g.byte('_selected_game')!=index:g.tap('down')
    if held:g.hold('a')
    else:g.tap('a')
    g.wait(lambda:NAMES[index].upper() in g.row(2),'instructions')

def pause_exit(g,index):
    g.hold();g.tap('start');g.wait(lambda:'PAUSED' in g.row(4),'pause exit route')
    g.tap('b');menu(g)
    assert g.byte('_selected_game')==index

def menu_routes():
    g=Game();g.tap('up');assert g.byte('_selected_game')==15
    g.tap('down');assert g.byte('_selected_game')==0
    g.tap('left');assert g.byte('_selected_game')==8
    g.tap('right');assert g.byte('_selected_game')==0
    for _ in range(16):g.tap('down')
    assert g.byte('_selected_game')==0
    g.close();print('PASS menu wrap and both page directions',flush=True)

def basic_routes(index):
    g=Game();choose(g,index,held=True)
    # Keeping A down must not skip the instruction screen.
    g.tick(45)
    assert NAMES[index].upper() in g.row(2)
    g.hold();g.tick(4);g.tap('b');menu(g)
    assert g.byte('_selected_game')==index
    g.tap('start');g.wait(lambda:NAMES[index].upper() in g.row(2),'Start opens instructions')
    g.tap('a')
    picker=index in (9,12)
    if picker:
        count=24 if index==9 else 20
        g.wait(lambda:'CHOOSE PUZZLE' in g.row(4),'puzzle selector')
        g.tap('left');assert int(g.row(7)[6:8])==count
        g.tap('right');assert int(g.row(7)[6:8])==1
        g.tap('b');menu(g)
        choose(g,index);g.tap('a');g.wait(lambda:'CHOOSE PUZZLE' in g.row(4),'reopen puzzle selector')
        g.tap('left');g.tap('a');playing(g)
        assert g.byte('crates._level' if index==9 else 'pegs._level')==count-1
    else:playing(g)
    # A held Start pauses once; a fresh Start resumes without reopening pause.
    g.hold('start');g.wait(lambda:'PAUSED' in g.row(4),'held Start pause');g.tick(45)
    assert 'PAUSED' in g.row(4)
    g.hold();g.tick(4);g.tap('start');playing(g)
    g.tap('start');g.wait(lambda:'PAUSED' in g.row(4),'pause for sound toggle')
    before=g.byte('_sound_on');g.tap('select');assert g.byte('_sound_on')!=before
    g.tap('a');playing(g)
    g.tap('start');g.wait(lambda:'PAUSED' in g.row(4),'pause for held B exit')
    sound=g.byte('_sound_on');g.hold('b');menu(g);g.tick(45)
    assert 'FLIGHT ARCADE' in g.row(1) and g.byte('_sound_on')==sound
    assert g.byte('_selected_game')==index
    g.hold();g.close()
    return {'game':NAMES[index],'instructions_back':True,'held_buttons_do_not_skip_screens':True,'pause_resume_a_and_start':True,'pause_sound_toggle':True,'pause_to_menu':True,'selection_retained':True,'puzzle_picker_cancel_wrap_and_last_level':picker}

def arrive_at_end(g,index):
    if index==0:
        g.hold();g.wait(lambda:result_screen(g),'shooter natural game over',12000)
    elif index==1:
        g.hold('right');g.wait(lambda:result_screen(g),'snake wall collision',1200)
    elif index==2:
        for frame in range(12000):
            if result_screen(g):break
            paddle=g.b.memory[0xFE01]-8;ball=g.b.memory[0xFE0D]-8
            target=4 if ball>76 else 132
            direction='left' if paddle>target+2 else 'right' if paddle<target-2 else None
            g.hold(*(['a']if frame%20<10 else []),*([direction]if direction else []));g.tick()
    elif index==3:
        rng=random.Random(72)
        for _ in range(1600):
            if 'A FINISH' in g.row(1):break
            g.tap(rng.choice(['left','up','right','down']))
        assert 'A FINISH' in g.row(1),'Reach a naturally full merge board'
        undo=g.array('merge._undo_board',16);g.tap('b')
        assert g.array('merge._board',16)==undo and 'A FINISH' not in g.row(1)
        for _ in range(1600):
            if 'A FINISH' in g.row(1):break
            g.tap(rng.choice(['left','up','right','down']))
        assert 'A FINISH' in g.row(1)
        g.tap('a')
    elif index==4:
        g.tap('a');mine=next(i for i,v in enumerate(g.array('mines._ground',63))if v&4)
        move_cursor(g,[4,3],mine%9,mine//9);g.tap('a')
        assert 'MINE!' in g.row(1)
        g.tap('a')
    elif index==6:
        for frame in range(24000):
            if result_screen(g):break
            paddle=g.b.memory[0xFE00]-16;ball=g.b.memory[0xFE18]-16
            target=104 if ball<72 else 24
            direction='down' if paddle<target-1 else 'up' if paddle>target+1 else None
            g.hold(*(['a']if frame%20<10 else []),*([direction]if direction else []));g.tick()
    elif index==7:
        g.hold('left','a');g.wait(lambda:result_screen(g),'river bank game over',2400)
    elif index==8:
        for _ in range(100):
            if result_screen(g):break
            g.tap('up')
    elif index==9:
        for direction in SOLUTIONS['crate'][0]['solution']:g.tap(direction,1)
    elif index==10:
        cards=g.array('pairs._cards',24);cursor=[0,0]
        for value in range(12):
            for i,v in enumerate(cards):
                if v==value:move_cursor(g,cursor,i%6,i//6);g.tap('a')
    elif index==11:
        while g.byte('maze._collected')<5:
            board=g.array('maze._maze',195)
            for key in path_to(board,g.byte('maze._player'),{i for i,v in enumerate(board)if v==2}):g.tap(key,1)
        for key in path_to(g.array('maze._maze',195),g.byte('maze._player'),{178}):g.tap(key,1)
    elif index==12:
        cursor=[2,2]
        for source,target in SOLUTIONS['peg'][0]['solution']:
            move_cursor(g,cursor,source%5,source//5);g.tap('a')
            move_cursor(g,cursor,target%5,target//5);g.tap('a')
    elif index==13:
        for _ in range(100):
            if result_screen(g):break
            moves=reversi_moves(g.array('discs._board',36),1)
            if not moves:g.tick(30);continue
            choice=min(moves);cursor=g.byte('discs._cursor')
            move_cursor(g,[cursor%6,cursor//6],choice%6,choice//6);g.tap('a');g.tick(20)
    elif index==14:
        for _ in range(70):
            if result_screen(g):break
            if g.byte('golf._stock_pos')<17:g.tap('b');continue
            counts=g.array('golf._counts',7);cards=g.array('golf._stack_cards',35);waste=g.byte('golf._waste')%13+1
            choices=[i for i,n in enumerate(counts)if n and (abs(cards[i*5+n-1]%13+1-waste)==1 or {cards[i*5+n-1]%13+1,waste}=={1,13})]
            if not choices:g.tick(30);continue
            choice=choices[0]
            while g.byte('golf._cursor')!=choice:g.tap('right')
            g.tap('a')
    elif index==15:
        g.hold();g.wait(lambda:result_screen(g),'lander natural crash',3000)
    g.hold();g.wait(lambda:result_screen(g),NAMES[index]+' result screen')

def terminal_routes(index):
    if index==5:
        g=Game();g.enter(index);cursor=[2,2]
        for cell in solve_lights(g.array('lights._lights',25)):
            move_cursor(g,cursor,cell%5,cell//5);g.tap('a')
        assert 'SOLVED!' in g.row(1)
        g.shot('06-navigation-result')
        g.tap('a');assert any(g.array('lights._lights',25))
        pause_exit(g,index);g.close()
        return ['solved_to_next_puzzle','next_puzzle_to_menu']
    checked=[]
    for action in ('b','a'):
        g=Game();g.enter(index);arrive_at_end(g,index)
        if action=='b':g.shot(f'{index+1:02d}-navigation-result')
        g.tap(action)
        if action=='b':menu(g);assert g.byte('_selected_game')==index;checked.append('result_b_to_menu')
        else:playing(g);pause_exit(g,index);checked.append('result_a_to_play_then_menu')
        trace('terminal_navigation',game=NAMES[index],action=action,verified=True)
        g.close()
    return checked

if __name__=='__main__':
    passed=False
    try:
        menu_routes()
        for index in range(len(NAMES)):
            progress('navigation_audit',index)
            checks=basic_routes(index)
            checks['terminal_routes']=terminal_routes(index)
            RESULTS.append(checks)
            print('PASS navigation',NAMES[index],flush=True)
        for first in range(0,len(NAMES),4):
            atlas(range(first,first+4),f'navigation-results-{first+1}-{first+4}.png','navigation-result')
        passed=True;progress('all_navigation_routes_verified',len(NAMES))
    finally:
        path=OUT/'navigation-results.json'
        path.write_text(json.dumps({'rom_sha256':ROM_SHA,'passed':passed,'results':RESULTS},indent=2))
        if not passed:progress('navigation_audit_failed',len(RESULTS),'failed','NAVIGATION_ASSERTION')
