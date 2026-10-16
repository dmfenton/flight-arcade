from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib
import json

ROOT=Path(__file__).resolve().parent
output=ROOT/'FlightArcade.zip'
rom=(ROOT/'build/FlightArcade.gbc').read_bytes()
sha=hashlib.sha256(rom).hexdigest()
games=json.loads((ROOT/'src/games.json').read_text())
checks=[('smoke-results.json',len(games)),('playtest-results.json',8),('expanded-playtest-results.json',8),('navigation-results.json',len(games))]
for filename,count in checks:
    path=ROOT/'verification'/filename
    evidence=json.loads(path.read_text())
    assert evidence['rom_sha256']==sha,f'{path}: result does not match current ROM'
    assert len(evidence['results'])==count,f'{path}: incomplete verification'
    if filename=='navigation-results.json':assert evidence['passed'],f'{path}: navigation audit failed'
audio_path=ROOT/'verification/audio-results.json'
audio=json.loads(audio_path.read_text())
assert audio['rom_sha256']==sha and audio['sfx_peak']>0 and audio['muted_peak']==0,f'{audio_path}: audio verification failed or stale'
header=0
for value in rom[0x134:0x14d]:header=(header-value-1)&255
assert header==rom[0x14d],f'{ROOT}/build/FlightArcade.gbc: invalid header checksum'
assert (sum(rom)-rom[0x14e]-rom[0x14f])&65535==int.from_bytes(rom[0x14e:0x150],'big'),f'{ROOT}/build/FlightArcade.gbc: invalid global checksum'
assert len(rom)==524288 and rom[0x143]==0xc0 and rom[0x147:0x14a]==bytes([0x1b,4,3]),f'{ROOT}/build/FlightArcade.gbc: unexpected cartridge layout'
manifest={'rom':'build/FlightArcade.gbc','bytes':len(rom),'sha256':sha,'title':rom[0x134:0x143].split(b'\0')[0].decode(),'cgb_flag':hex(rom[0x143]),'platform':'Game Boy Color','cartridge_type':hex(rom[0x147]),'rom_size_code':hex(rom[0x148]),'ram_size_code':hex(rom[0x149]),'header_checksum_valid':True,'global_checksum_valid':True,'sdk_version':'GBDK 2020 4.5.0','games':len(games)}
(ROOT/'verification/rom-manifest.json').write_text(json.dumps(manifest,indent=2))
files=[ROOT/name for name in ['.gitignore','README.md','LICENSE','THIRD_PARTY_NOTICES.md','CONTRIBUTING.md','RELEASE_NOTES.md','requirements-dev.txt','build.sh','verify.py','verify_navigation.py','playtest.py','playtest_more.py','package.py','build/FlightArcade.gbc']]
files+=[path for path in (ROOT/'src').iterdir()if path.suffix in ('.c','.h','.py','.json')]
files+=[ROOT/'verification'/name for name in ['puzzle-solutions.json','rom-manifest.json','smoke-results.json','playtest-results.json','expanded-playtest-results.json','audio-results.json','navigation-results.json']]
files+=list((ROOT/'.github/workflows').glob('*.yml'))
files+=list((ROOT/'verification').glob('games-*.png'))
files+=list((ROOT/'verification').glob('instructions-*.png'))
files+=list((ROOT/'verification').glob('navigation-results-*.png'))
files+=[ROOT/'verification'/name for name in ['menu.png','menu-2.png']]
with ZipFile(output,'w',ZIP_DEFLATED) as archive:
    for path in files:
        if path.is_file():archive.write(path,Path('FlightArcade')/path.relative_to(ROOT))
print(output)
