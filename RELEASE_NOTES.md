Flight Arcade puts 16 original games in one offline Game Boy Color ROM: arcade games, solitaire, Reversi, falling blocks, memory, mazes, lunar landing, and 44 crate/peg puzzles. Includes saved best scores, pause, and sound controls.

Project source, tile/font artwork, sound-effect definitions, and puzzle generators are MIT licensed. See `THIRD_PARTY_NOTICES.md` for the separately licensed toolchain and development tools.

Downloads:

- `FlightArcade.gbc`: ready-to-play 512 KiB Game Boy Color ROM, MBC5 with 32 KiB save RAM.
- `FlightArcade.zip`: ROM, project source, license, build instructions, and public verification reports. SDK downloads, flashing tools, cartridge backups, and private traces are excluded.
- `rom-manifest.json`: ROM hash and verified header/checksum metadata.

All 16 games pass menu, instruction, pause/resume, and return checks. Extended joypad playtests cover all 44 puzzle solutions, a full solitaire clear, a complete Reversi match, two soft landings, sound/mute, and saved scores after restart. The navigation audit checks instruction back, held buttons, pause exits, puzzle selection, full merge boards, mine-loss prompts, solved lights, and replay/menu result paths. The river pilot may need several normally generated attempts; solitaire deals are not guaranteed solvable.

The ROM was written successfully to a writable DevDay Chromatic cartridge, its header was read back, and the handheld reset succeeded. The owner confirmed that it looks good on the physical Chromatic. Physical controls and audio have not been separately confirmed. Emulator checks do not establish those physical-device results.
