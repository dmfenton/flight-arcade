"""Navigation checks for real-input checkpoints plus the eight-game shell.

Checkpoint branches copy a whole emulator state reached by joypad play. No test
patches game RAM or invents a terminal/intermission state.
"""
import io,json
from verify import Game,ROOT,OUT,ROM_SHA,NAMES,STATIC,SYMBOLS,atlas,trace,progress

MODULES=['star','drop','bricks','rally','river','snake','orbit','lander']
def state(g,index):
    names=[n for n in STATIC if n.startswith(MODULES[index]+'.')]
    addresses=sorted(STATIC[n]for n in names)
    # The next object's first static variable bounds this module. Symbols are
    # backed by the compiler's actual DATA contribution, not hand-coded offsets.
    starts=sorted(set(v for k,v in STATIC.items()if k.split('.')[0]!=MODULES[index]))
    start=min(addresses);end=next((v for v in starts if v>max(addresses)),max(addresses)+8)
    return bytes(g.b.memory[start:end]),g.word('_score')
def fork(g):
    checkpoint=io.BytesIO();g.b.save_state(checkpoint);checkpoint.seek(0)
    other=Game();other.b.load_state(checkpoint)
    other.held=set(g.held)
    # Release the checkpoint's live joypad buttons normally.
    other.hold();other.tick(3)
    return other

def audit_choice(g,index,label):
    g.hold();g.tick(2);g.shot(f'{index+1:02d}-{label}')
    for route in ('b','pause-b'):
        other=fork(g)
        try:
            if route=='pause-b':
                other.tap('start',1);other.wait(lambda:'PAUSED'in other.row(4),'choice pause')
                before=state(other,index);other.tick(60);assert state(other,index)==before,'choice simulation advanced while paused'
            other.tap('b',1);other.wait(lambda:'FLIGHT ARCADE'in other.row(1),'choice exit')
        finally:other.close()
    g.navigation=getattr(g,'navigation',[])+[dict(screen=label,back=True,pause_menu=True,pause_freezes=True)]

def audit_result(g,index,label='campaign_result'):
    # A held firing/steering button must not replay a freshly reached result.
    g.tick(30);assert 'B ARCADE MENU'in g.row(16),'held input escaped result'
    g.hold();g.tick(2);g.shot(f'{index+1:02d}-result')
    for route in ('a','b'):
        other=fork(g)
        try:
            other.tap(route,1)
            if route=='b':other.wait(lambda:'FLIGHT ARCADE'in other.row(1),'result back')
            elif index==7:
                other.wait(lambda:'CARGO CONTRACT'in other.row(3),'replay initial contract')
                assert other.word('_score')==0 and other.byte('lander._deliveries')==0 and other.byte('lander._fuel')==180
                other.tap('b',1);other.wait(lambda:'FLIGHT ARCADE'in other.row(1),'replay contract back')
            else:
                other.wait(lambda:'START PAUSE'in other.row(17),'result replay')
                assert other.word('_score')==0,'replay did not reset score'
                other.exit_test()
        finally:other.close()
    g.navigation=getattr(g,'navigation',[])+[dict(screen=label,held_input=True,replay=True,back=True)]

def loss_result(index):
    g=Game()
    try:
        g.enter(index)
        for tick in range(12000):
            if 'B ARCADE MENU'in g.row(16):break
            if index==1:g.tap('up',1)
            elif index==2:
                if not g.byte('bricks._launched'):g.tap('a',1)
                g.hold('left');g.tick()
            elif index==3:
                if g.byte('rally._serve'):g.tap('a',1)
                g.hold('up');g.tick()
            elif index==4:g.hold('left');g.tick()
            elif index==5:g.hold('right');g.tick()
            elif index==6:g.hold('right');g.tick()
            elif index==7:
                if g.byte('lander._docked'):g.tap('a',1)
                g.hold('right');g.tick()
            else:g.hold();g.tick()
        assert 'B ARCADE MENU'in g.row(16),f'{NAMES[index]}: natural terminal state not reached'
        if index==3:assert g.byte('rally._opponent')==7,'opponent win not exercised'
        audit_result(g,index,'loss_result')
        return g.navigation[-1]
    finally:g.close()

def run():
    details=json.loads((OUT/'playtest-results.json').read_text())
    assert details['rom_sha256']==ROM_SHA
    results=[]
    for index,name in enumerate(NAMES):
        g=Game()
        try:
            for _ in range(index):g.tap('down',1)
            for page_key in (None,'right'):
                g.tap('a',1);g.wait(lambda:'CONTROLS'in g.row(3),'controls page')
                if page_key:g.tap(page_key,1);g.wait(lambda:'TACTICS'in g.row(3),'tactics page')
                g.tap('b',1);g.wait(lambda:'FLIGHT ARCADE'in g.row(1),'instructions back')
            g.tap('a',1);g.tap('right',1);g.tap('a',1)
            if index==7:
                audit_choice(g,index,'initial-contract');g.tap('a',1)
            g.wait(lambda:'START PAUSE'in g.row(17),'tactics starts gameplay')
            g.tap('start',1);g.wait(lambda:'PAUSED'in g.row(4),'pause')
            frozen=state(g,index);g.tick(90);assert state(g,index)==frozen,f'{name}: pause advanced gameplay data'
            g.tap('start',1);g.wait(lambda:'START PAUSE'in g.row(17),'Start resume')
            g.exit_test()
        finally:g.close()
        loss=loss_result(index)
        campaign=details['results'][index]
        assert campaign['campaign_won']and campaign['navigation']
        results.append(dict(game=name,controls_back=True,tactics_back=True,tactics_play=True,game_pause=True,pause_freezes_simulation=True,start_resume=True,game_menu=True,checkpoints=campaign['navigation']+[loss]))
        print('PASS navigation',name,flush=True)
    data=dict(rom_sha256=ROM_SHA,passed=True,results=results,checkpoint_method='whole emulator snapshots reached through joypad input; no RAM patching')
    (OUT/'navigation-results.json').write_text(json.dumps(data,indent=2))
    for first in (0,4):atlas(range(first,first+4),f'navigation-results-{first+1}-{first+4}.png','result')
    progress('navigation_passed',8)
if __name__=='__main__':run()
