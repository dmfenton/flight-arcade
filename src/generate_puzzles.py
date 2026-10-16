"""Create original solvable crate and peg puzzles by reversing legal moves."""
from pathlib import Path
from collections import deque
import hashlib
import itertools
import json
import random

ROOT=Path(__file__).resolve().parent
VERSION=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
DATA=ROOT/'puzzle_data.c'
EVIDENCE=ROOT.parent/'verification/puzzle-solutions.json'
if DATA.exists() and EVIDENCE.exists() and DATA.read_text().startswith('// '+VERSION):
    print(DATA);print(EVIDENCE);raise SystemExit

DIRECTIONS=[(-1,0,'left'),(1,0,'right'),(0,-1,'up'),(0,1,'down')]
OPPOSITE={'left':'right','right':'left','up':'down','down':'up'}
templates=[
 ['#########','#       #','#   #   #','#       #','#   #   #','#       #','#########'],
 ['#########','#       #','# #   # #','#       #','# #   # #','#       #','#########'],
 ['#########','#       #','#  #    #','#       #','#    #  #','#       #','#########'],
 ['#########','#       #','# # # # #','#       #','#       #','#       #','#########'],
 ['#########','#       #','#       #','#   #   #','#       #','#       #','#########'],
 ['#########','#       #','#  # #  #','#       #','#  # #  #','#       #','#########'],
]

def crate_solve(floor,boxes,player,goals):
    """A push-space BFS gives an independently validated, short input solution."""
    def reachable(player,boxes):
        routes={player:[]};q=deque([player])
        while q:
            p=q.popleft()
            for dx,dy,key in DIRECTIONS:
                n=p+dy*9+dx
                if n in floor and n not in boxes and n not in routes:routes[n]=routes[p]+[key];q.append(n)
        return routes
    boxes=frozenset(boxes);q=deque([(boxes,player,[])]);seen=set()
    while q:
        boxes,player,path=q.popleft()
        if boxes==goals:return path
        routes=reachable(player,boxes);state=(boxes,min(routes))
        if state in seen:continue
        seen.add(state)
        for box in sorted(boxes):
            for dx,dy,key in DIRECTIONS:
                d=dy*9+dx;standing=box-d;destination=box+d
                if standing in routes and destination in floor and destination not in boxes:
                    # A box against two perpendicular walls cannot leave that corner.
                    if destination not in goals and any(destination+a not in floor and destination+b not in floor for a,b in [(1,9),(1,-9),(-1,9),(-1,-9)]):continue
                    q.append(((boxes-{box})|{destination},box,path+routes[standing]+[key]))
    return None

crates=[];unique=set()
for level in range(24):
    rng=random.Random(9043+level*131)
    template=templates[level%len(templates)]
    floor={y*9+x for y,row in enumerate(template)for x,c in enumerate(row)if c==' '}
    interior=[p for p in sorted(floor)if 2<=p%9<=6 and 2<=p//9<=4]
    while True:
        goals=frozenset(rng.sample(interior,2 if level<8 else 3));boxes=set(goals);player=rng.choice(sorted(floor-boxes));history=[]
        for _ in range(150):
            moves=[];pulls=[]
            for dx,dy,key in DIRECTIONS:
                d=dy*9+dx
                if player+d in floor and player+d not in boxes:
                    moves.append((d,key))
                    if player-d in boxes:pulls.append((d,key))
            if not moves:break
            d,key=rng.choice(pulls if pulls and rng.random()<0.7 else moves)
            if player-d in boxes:boxes.remove(player-d);boxes.add(player)
            player+=d;history.append(key)
        distance=min(sum(abs(a%9-b%9)+abs(a//9-b//9)for a,b in zip(sorted(boxes),perm))for perm in itertools.permutations(goals))
        signature=(tuple(sorted(boxes)),player,goals,level%6)
        if distance<4+level//8 or signature in unique:continue
        solution=crate_solve(floor,boxes,player,goals)
        if not solution or len(solution)<8:continue
        unique.add(signature)
        board=[(0 if i in floor else 1)|(2 if i in goals else 0)|(4 if i in boxes else 0)for i in range(63)]
        crates.append({'board':board,'player':player,'solution':solution})
        print(f'{DATA}: crate puzzle {level+1}/24, {len(solution)} input moves',flush=True);break

valid={y*5+x for y in range(5)for x in range(5)if not(x in (0,4)and y in (0,4))}
jumps=[]
for source in sorted(valid):
    for dx,dy,key in DIRECTIONS:
        x=source%5;y=source//5;mx=x+dx;my=y+dy;tx=x+2*dx;ty=y+2*dy
        middle=my*5+mx;target=ty*5+tx
        if 0<=tx<5 and 0<=ty<5 and middle in valid and target in valid:jumps.append((source,middle,target))
pegs=[];seen=set()
for level in range(20):
    rng=random.Random(4723+level*113);target_count=6+level%8
    while True:
        state={12};reverse=[]
        while len(state)<target_count:
            options=[j for j in jumps if j[2]in state and j[0]not in state and j[1]not in state]
            if not options:break
            source,middle,target=rng.choice(options);state.remove(target);state.update([source,middle]);reverse.append([source,target])
        mask=sum(1<<i for i in state)
        if len(state)==target_count and mask not in seen:
            seen.add(mask);pegs.append({'mask':mask,'solution':list(reversed(reverse))});break

content='// '+VERSION+'\n#pragma bank 10\n#include "arcade.h"\n'
content+='const uint8_t crate_levels[24][63]={\n'+',\n'.join('{'+','.join(map(str,g['board']))+'}'for g in crates)+'\n};\n'
content+='const uint8_t crate_starts[24]={'+','.join(str(g['player'])for g in crates)+'};\n'
# Peg data lives in bank 13; the runtime reads it without switching banks.
peg='// '+VERSION+'\n#pragma bank 13\n#include "arcade.h"\n'
peg+='const uint32_t peg_levels[20]={'+','.join(hex(g['mask'])+'UL'for g in pegs)+'};\n'
peg+='const uint32_t peg_valid='+hex(sum(1<<i for i in valid))+'UL;\n'
DATA.write_text(content);(ROOT/'peg_data.c').write_text(peg)
EVIDENCE.write_text(json.dumps({'generator_sha256':VERSION,'crate':crates,'peg':pegs},indent=2))
print(DATA);print(ROOT/'peg_data.c');print(EVIDENCE)
