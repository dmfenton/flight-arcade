# Contributing

Keep the eight games distinct. A new mechanic should create a decision that
changes play, rather than only a new score label or faster repetition. Keep
physics and resources with their owning game module; see [DESIGN.md](DESIGN.md).

Use GBDK 2020 4.5.0 and the exact dependencies in `requirements-dev.txt`. Run the
build, smoke/audio checks, input-only campaigns, navigation audit, and package
command in the README before opening a pull request. Include the ROM hash and
meaningful behavior evidence. Do not substitute a RAM-patched terminal state for
playing the mechanic. Whole-emulator checkpoints reached through input may be
used to branch an exit/replay check.

Edit `src/games.json` for labels, controls, tactics, and stable save slots. Run
`src/generate_catalog.py`; edit `src/generate_assets.py` for generated tile art.
Do not change the persistent save layout or remap a slot silently. Remove
obsolete paths when replacing a game or test.

Keep `private/`, SDK binaries, personal cartridge backups, USB identifiers, and
loader/session state out of commits and archives. Packaging uses an explicit
public allowlist and requires matching test hashes. A release and a cartridge
write each require an explicit owner request.
