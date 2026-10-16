# Contributing

Use GBDK 2020 4.5.0 and the pinned Python dependencies in `requirements-dev.txt`. Follow the build and verification commands in [README.md](README.md).

Game rules and round state belong to each game's C file. Shared input, rendering, pause, sound, scores, and saves belong to `src/core.c`. Edit `src/games.json` for names and instructions, `src/generate_assets.py` for tile art, and `src/generate_puzzles.py` for generated puzzle content; rebuild to refresh their generated outputs.

Before proposing a change, rebuild, run the smoke suite, both playtest scripts, and `python3 verify_navigation.py`, inspect screenshots of changed screens, and run `python3 package.py`. Report the ROM hash and relevant test results. Label emulator checks and physical-device checks separately. Existing test pilots are heuristics; a lost random round is not by itself evidence of a game defect.

Keep original cartridge backups, activation codes, device logs, SDK downloads, and local runtime traces out of commits. Tests store private traces beside the main checkout, resolved through Git's common directory. An extracted source archive uses its own `private/` folder.

Only contribute code and assets you can release under this project's MIT license. Include the source of any third-party contribution and its license. No copied commercial game code, ROMs, artwork, characters, or audio.
