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
for game in games:
    assert len(game['name'])<=17
    assert len(game['description'])<=19
    assert len(game['help'])==6 and all(len(line)<=18 for line in game['help'])
write('catalog.h',f'#ifndef CATALOG_H\n#define CATALOG_H\n#define GAME_COUNT {len(games)}\n#endif\n')
catalog='#include "arcade.h"\nconst char * const game_names[GAME_COUNT] = {\n'+',\n'.join('    '+c(g['name'])for g in games)+'\n};\n'
catalog+='const char * const game_descriptions[GAME_COUNT] = {\n'+',\n'.join('    '+c(g['description'])for g in games)+'\n};\n'
write('catalog.c',catalog)
instructions='#pragma bank 17\n#include "arcade.h"\nstatic const char * const help[GAME_COUNT][6] = {\n'
instructions+=',\n'.join('    {'+','.join(c(line)for line in game['help'])+'}'for game in games)+'\n};\n'
instructions+='''uint8_t intro(uint8_t game) BANKED {
    uint8_t i;
    hide_objects();clear_screen(0);text(2,2,game_names[game],4);
    for(i=0;i<6;i++)text(2,i==0?5:i==1?7:i==2?9:8+i,help[game][i],i<3?0:6);
    text(1,16,"A PLAY     B BACK",0);flush();
    while(1){read_frame();if(pressed&J_A)return 1;if(pressed&J_B)return 0;}
}
'''
write('instructions.c',instructions)
