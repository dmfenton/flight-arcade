"""The catalog is the single source for menu labels, instructions and test names."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parent
games=json.loads((ROOT/'games.json').read_text())
def write(name,content):
    path=ROOT/name
    if not path.exists()or path.read_text()!=content:path.write_text(content)
    print(path)
def c(value):return json.dumps(value.upper())
catalog_path=ROOT/'games.json'
for game in games:
    for field,limit in [('name',17),('description',19)]:
        if len(game[field])>limit:raise ValueError(f'{catalog_path}: {game["name"]} {field} exceeds {limit} screen columns')
    for field in ('help','strategy'):
        if len(game[field])!=6 or any(len(line)>18 for line in game[field]):
            raise ValueError(f'{catalog_path}: {game["name"]} {field} requires six lines of at most 18 characters')
if len({g['save_slot']for g in games})!=len(games)or any(not 0<=g['save_slot']<16 for g in games):
    raise ValueError(f'{catalog_path}: save slots must be unique values from 0 through 15')
write('catalog.h',f'#ifndef CATALOG_H\n#define CATALOG_H\n#define GAME_COUNT {len(games)}\n#endif\n')
catalog='#include "arcade.h"\nconst char * const game_names[GAME_COUNT] = {\n'+',\n'.join('    '+c(g['name'])for g in games)+'\n};\n'
catalog+='const char * const game_descriptions[GAME_COUNT] = {\n'+',\n'.join('    '+c(g['description'])for g in games)+'\n};\n'
catalog+='const uint8_t game_save_slots[GAME_COUNT] = {'+','.join(str(g['save_slot'])for g in games)+'};\n'
write('catalog.c',catalog)
instructions='#pragma bank 9\n#include "arcade.h"\nstatic const char * const help[GAME_COUNT][2][6] = {\n'
instructions+=',\n'.join('    {{'+','.join(c(line)for line in game['help'])+'},{'+','.join(c(line)for line in game['strategy'])+'}}'for game in games)+'\n};\n'
instructions+='''uint8_t intro(uint8_t game) BANKED {
    uint8_t i,page=0,redraw=1;
    hide_objects();
    while(1) {
        if(redraw) {
            clear_screen(0);text(2,2,game_names[game],4);text(2,3,page?"TACTICS  2/2":"CONTROLS 1/2",6);
            for(i=0;i<6;i++)text(2,i==0?5:i==1?7:i==2?9:8+i,help[game][page][i],i<3?0:6);
            text(1,15,"L/R CHANGE PAGE",6);text(1,17,"A PLAY     B BACK",0);flush();redraw=0;
        }
        read_frame();if(pressed&(J_LEFT|J_RIGHT)){page^=1;redraw=1;}
        if(pressed&J_A)return 1;if(pressed&J_B)return 0;
    }
}
'''
write('instructions.c',instructions)
