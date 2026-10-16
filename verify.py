"""Play the actual ROM with joypad input and inspect its rendered screens and state."""
from pathlib import Path
from datetime import datetime, timezone
import io
import hashlib
import json
import os
import re
import subprocess
import time
from pyboy import PyBoy
from PIL import Image, ImageDraw

ROOT=Path(__file__).resolve().parent
OUT=ROOT/'verification'
def private_directory():
    if (ROOT/'.git').exists():
        common=Path(subprocess.check_output(['git','-C',str(ROOT),'rev-parse','--path-format=absolute','--git-common-dir'],text=True).strip())
        return common.parent/'private'
    return ROOT/'private'

PRIVATE=private_directory()
OUT.mkdir(exist_ok=True)
PRIVATE.mkdir(mode=0o700,exist_ok=True)
PRIVATE.chmod(0o700)
ROM_SHA=hashlib.sha256((ROOT/'build/FlightArcade.gbc').read_bytes()).hexdigest()
NAMES=[game['name']for game in json.loads((ROOT/'src/games.json').read_text())]
SYMBOLS={name:int(addr,16) for name,addr in re.findall(r'^DEF (\S+) 0x([0-9A-Fa-f]+)',(ROOT/'build/FlightArcade.noi').read_text(),re.M)}
STATIC={}
base=SYMBOLS['s__DATA']
for obj in sorted((ROOT/'build').glob('*.o')):
    sym=obj.with_suffix('.sym').read_text()
    data_area=re.search(r'^\s+(\S+) _DATA\s+size',sym,re.M)[1]
    for name,offset in re.findall(r'^\s+'+data_area+r' (\S+)\s+([0-9A-F]{8}) [GR]+',sym,re.M):
        STATIC[obj.stem+'.'+name]=base+int(offset,16)
    base+=int(re.search(r'A _DATA size ([0-9A-F]+)',obj.read_text())[1],16)
# The GBDK startup object contributes its own data before the project objects.
startup_data=SYMBOLS['_score']-STATIC['core._score']
STATIC={name:address+startup_data for name,address in STATIC.items()}
assert STATIC['core._best']==SYMBOLS['_best']
assert STATIC['core._tiles']==SYMBOLS['_tiles']

def progress(phase,count=0,outcome='running',blocker=None):
    p=PRIVATE/'progress.json'; tmp=p.with_suffix('.tmp')
    tmp.write_text(json.dumps({'phase':phase,'updated_at':datetime.now(timezone.utc).isoformat(),'games_implemented':len(NAMES),'games_verified':count,'blocker':blocker,'outcome':outcome},indent=2))
    tmp.chmod(0o600);os.replace(tmp,p)

TRACE=(PRIVATE/'playtest.jsonl').open('a',buffering=1)
os.chmod(PRIVATE/'playtest.jsonl',0o600)
def trace(event,**data):
    TRACE.write(json.dumps({'at':datetime.now(timezone.utc).isoformat(),'event':event,**data})+'\n')

class Game:
    def __init__(self,ram=None):
        self.ram=io.BytesIO(ram or bytes(32768))
        self.b=PyBoy(str(ROOT/'build/FlightArcade.gbc'),window='null',cgb=True,sound_emulated=True,ram_file=self.ram)
        self.b.set_emulation_speed(0)
        self.held=set();self.frames=0
        self.tick(180)
        self.wait(lambda:'FLIGHT ARCADE' in self.row(1),'boot to menu')
    def tick(self,n=1):
        self.b.tick(n);self.frames+=n
    def byte(self,name):return self.b.memory[STATIC[name] if '.' in name else SYMBOLS[name]]
    def word(self,name):
        addr=STATIC[name] if '.' in name else SYMBOLS[name]
        return self.b.memory[addr] | self.b.memory[addr+1]<<8
    def array(self,name,n):
        addr=STATIC[name] if '.' in name else SYMBOLS[name]
        return list(self.b.memory[addr:addr+n])
    def row(self,y):
        p=SYMBOLS['_tiles']+y*20
        return ''.join(chr(c) if 32<=c<96 else '?' for c in self.b.memory[p:p+20])
    def wait(self,predicate,label,limit=360):
        for _ in range(limit):
            if predicate():return
            self.tick()
        self.shot('failure')
        raise AssertionError(f'{label}: {[self.row(y) for y in range(18)]}')
    def hold(self,*keys):
        new=set(keys)
        for key in self.held-new:self.b.button_release(key)
        for key in new-self.held:self.b.button_press(key)
        self.held=new
        trace('joypad',keys=sorted(new),frame=self.frames)
    def tap(self,key,n=4):
        mask={'right':1,'left':2,'up':4,'down':8,'a':16,'b':32,'select':64,'start':128}[key]
        self.hold();self.wait(lambda:not(self.byte('_keys')&mask),'ready for '+key)
        self.hold(key);self.wait(lambda:bool(self.byte('_keys')&mask),'read '+key)
        self.tick(n);self.hold();self.wait(lambda:not(self.byte('_keys')&mask),'release '+key)
        self.tick(n)
    def shot(self,name):
        # Render consecutive frames: PyBoy's batched tick skips intermediate
        # rendering and can retain partial pixels in double-speed CGB mode.
        for _ in range(3):self.tick()
        path=OUT/(name+'.png')
        self.b.screen.image.resize((480,432),Image.Resampling.NEAREST).save(path)
        trace('screenshot',path=str(path),frame=self.frames)
    def enter(self,index):
        if index>=8:self.tap('right')
        for _ in range(index%8):self.tap('down')
        assert self.byte('_selected_game')==index
        self.tap('a');self.wait(lambda:NAMES[index].upper() in self.row(2),'instructions')
        self.shot(f'{index+1:02d}-intro')
        self.tap('a')
        if index in (9,12):
            self.wait(lambda:'CHOOSE PUZZLE' in self.row(4),'puzzle picker');self.shot(f'{index+1:02d}-picker');self.tap('a')
        self.wait(lambda:'START PAUSE' in self.row(17),'gameplay')
        self.shot(f'{index+1:02d}-play')
        trace('enter',game=NAMES[index])
    def pause_test(self,index):
        self.hold();self.tick(10)
        self.tap('start');self.wait(lambda:'PAUSED' in self.row(4),'pause')
        before=self.word('_score');self.tick(90);assert self.word('_score')==before
        self.shot(f'{index+1:02d}-pause')
        self.tap('a');self.wait(lambda:'START PAUSE' in self.row(17),'resume')
    def exit_test(self):
        self.hold();self.tap('start');self.wait(lambda:'PAUSED' in self.row(4),'pause to exit')
        self.tap('b');self.wait(lambda:'FLIGHT ARCADE' in self.row(1),'return to menu')
    def close(self):
        self.hold();destination=io.BytesIO();self.b.stop(ram_file=destination);return destination.getvalue()

def atlas(indices,name,kind='play'):
    canvas=Image.new('RGB',(960,920),'#10182e');d=ImageDraw.Draw(canvas)
    for pos,i in enumerate(indices):
        x=(pos%2)*480;y=(pos//2)*460
        d.text((x+12,y+8),NAMES[i],fill='#d6f7e9')
        canvas.paste(Image.open(OUT/f'{i+1:02d}-{kind}.png'),(x,y+28))
    canvas.save(OUT/name)

def smoke():
    results=[]
    g=Game();g.shot('menu');g.tap('right');g.shot('menu-2');g.close()
    for i,name in enumerate(NAMES):
        progress('emulator_smoke',i)
        g=Game();g.enter(i);g.pause_test(i);g.exit_test();g.close()
        results.append({'game':name,'boot':True,'instructions':True,'play_screen':True,'pause_freezes_score':True,'resume':True,'return_to_menu':True})
        print('PASS',name,flush=True)
    for first in range(0,len(NAMES),4):
        indices=range(first,min(first+4,len(NAMES)))
        atlas(indices,f'games-{first+1}-{min(first+4,len(NAMES))}.png')
        atlas(indices,f'instructions-{first//4+1}.png','intro')
    (OUT/'smoke-results.json').write_text(json.dumps({'rom_sha256':ROM_SHA,'results':results},indent=2))
    audio_test()
    progress('emulator_smoke_passed',len(NAMES))
    return results

def audio_test():
    g=Game();g.enter(0);g.hold('a');loud=0
    for _ in range(40):
        g.tick();loud=max(loud,int(abs(g.b.sound.ndarray.astype('int16')).max()))
    g.hold();g.tap('select');g.tick(40);g.hold('a');muted=0
    for _ in range(40):
        g.tick();muted=max(muted,int(abs(g.b.sound.ndarray.astype('int16')).max()))
    g.close()
    assert loud>0 and muted==0,f'{ROOT}/build/FlightArcade.gbc: sound or mute failed ({loud}, {muted})'
    (OUT/'audio-results.json').write_text(json.dumps({'rom_sha256':ROM_SHA,'sfx_peak':loud,'muted_peak':muted},indent=2))
    print('PASS sound and mute',flush=True)

if __name__=='__main__':
    smoke()
