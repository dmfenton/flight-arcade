"""Package only public source and evidence verified against this exact ROM."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json
ROOT=Path(__file__).resolve().parent
output=ROOT/'FlightArcade.zip';rom_path=ROOT/'build/FlightArcade.gbc';rom=rom_path.read_bytes();sha=hashlib.sha256(rom).hexdigest()
games=json.loads((ROOT/'src/games.json').read_text())
for filename in ('smoke-results.json','playtest-results.json','navigation-results.json'):
    path=ROOT/'verification'/filename;evidence=json.loads(path.read_text())
    assert evidence['rom_sha256']==sha,f'{path}: result does not match current ROM'
    assert len(evidence['results'])==len(games)==8,f'{path}: incomplete verification'
    if filename=='navigation-results.json':assert evidence['passed'],f'{path}: navigation audit failed'
    if filename=='playtest-results.json':assert all(v['campaign_won']and v['score_persisted']for v in evidence['results']),f'{path}: campaigns or score persistence incomplete'
for filename in ('audio-results.json','save-results.json'):
    path=ROOT/'verification'/filename;data=json.loads(path.read_text());assert data['rom_sha256']==sha,f'{path}: stale verification'
    if filename=='audio-results.json':assert data['sfx_peak']>0 and data['muted_peak']==0,f'{path}: sound/mute failed'
    else:assert data['passed']and data['legacy_slots_preserved']==16,f'{path}: save preservation failed'
header=0
for value in rom[0x134:0x14d]:header=(header-value-1)&255
assert header==rom[0x14d],f'{rom_path}: invalid header checksum'
assert (sum(rom)-rom[0x14e]-rom[0x14f])&65535==int.from_bytes(rom[0x14e:0x150],'big'),f'{rom_path}: invalid global checksum'
assert len(rom)==262144 and rom[0x143]==0xc0 and rom[0x147:0x14a]==bytes([0x1b,3,3]),f'{rom_path}: unexpected cartridge layout'
manifest=dict(rom='build/FlightArcade.gbc',bytes=len(rom),sha256=sha,title=rom[0x134:0x143].split(b'\0')[0].decode(),cgb_flag=hex(rom[0x143]),platform='Game Boy Color',cartridge_type=hex(rom[0x147]),rom_size_code=hex(rom[0x148]),ram_size_code=hex(rom[0x149]),header_checksum_valid=True,global_checksum_valid=True,sdk_version='GBDK 2020 4.5.0',games=len(games),release_status='development candidate; not physically installed')
(ROOT/'verification/rom-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
files=[ROOT/name for name in ['.gitignore','README.md','DESIGN.md','LICENSE','THIRD_PARTY_NOTICES.md','CONTRIBUTING.md','RELEASE_NOTES.md','requirements-dev.txt','build.sh','verify.py','verify_navigation.py','playtest.py','package.py','build/FlightArcade.gbc']]
files+=sorted(p for p in (ROOT/'src').iterdir()if p.suffix in ('.c','.h','.py','.json'))
files+=[ROOT/'verification'/name for name in ['rom-manifest.json','smoke-results.json','playtest-results.json','audio-results.json','save-results.json','navigation-results.json','menu.png','games-1-4.png','games-5-8.png','instructions-1.png','instructions-2.png','depth-1-4.png','depth-5-8.png','navigation-results-1-4.png','navigation-results-5-8.png']]
files+=sorted((ROOT/'.github/workflows').glob('*.yml'))
with ZipFile(output,'w',ZIP_DEFLATED)as archive:
    for path in files:
        assert path.is_file(),f'{path}: required public artifact not found'
        archive.write(path,Path('FlightArcade')/path.relative_to(ROOT))
with ZipFile(output)as archive:
    assert archive.testzip()is None,f'{output}: archive failed CRC check'
    assert archive.read('FlightArcade/build/FlightArcade.gbc')==rom,f'{output}: archived ROM differs'
    assert not any(part in ('private','tools','.git','..')for name in archive.namelist()for part in Path(name).parts),f'{output}: private path in archive'
print(output)
