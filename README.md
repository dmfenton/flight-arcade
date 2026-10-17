# Flight Arcade

Eight original action games for Game Boy Color and ModRetro Chromatic, built with
GBDK 2020. This branch develops a smaller, deeper collection with momentum,
gravity, scarce resources, and complete campaigns.

**Published release:** [v0.1.0](https://github.com/dmfenton/flight-arcade/releases/tag/v0.1.0)
contains the earlier sixteen-game collection. The eight-game version on this
branch is a development candidate; it has not been published or flashed onto a
physical Chromatic.

## Play

Open `build/FlightArcade.gbc` in a Game Boy Color emulator. The ROM is CGB-only,
256 KiB, MBC5, with 32 KiB battery-backed RAM. A compatible writable cartridge is
required for hardware installation; USB connection alone does not turn an
ordinary retail cartridge into a flash cartridge.

Up/down selects a game. A opens instructions; left/right switches between
Controls and Tactics. A starts, B returns. Select toggles sound during play.
**Start pauses every live game or intermission; B from pause returns to the
arcade menu.** Results offer A replay and B menu. Choices accept B directly.

| Game | Play | Twist |
|---|---|---|
| Star Skimmer | D-pad move; A fire; B shield | Gun heat, falling salvage, paid shields, three bosses, gun-or-hull refits. |
| Reactor Drop | Left/right move; A rotate; up hard drop; down soft drop; B drop + vent | Garbage pressure, multi-row coolant rewards, three zones. |
| Ricochet Works | Left/right paddle; A launch/flip gravity; hold B magnet | Gravity charges, magnetic catches, explosive brick chains, three rooms. |
| Gravity Rally | Up/down paddle; A serve/dash; B reverse field | Paddle spin, two alternating wells, dash cooldown, limited flux; first to seven. |
| Tide Rider | Left/right paddle; A row; B drag | Current and momentum, shared power, rescues, fast-or-repair river forks. |
| Pulse Serpent | D-pad turn; A boost; B phase | Four-step wall/tail phasing, power economy, pulsing gates, coil choices. |
| Orbit Salvage | D-pad thrust; A tether/release; B brake | Spring tension, cargo inertia, core avoidance, slow docking, nine pods. |
| Lunar Courier | Left/right steer; A/up thrust; B jettison | Heavy ore or fragile glass, persistent fuel, wind, stopovers, five contracts. |

Scores and sound settings persist in battery RAM. The version-2 save layout
retains all sixteen old score slots; the new catalog maps to eight of them.
Older scores may not be comparable after a game's rules change. See
[DESIGN.md](DESIGN.md) for game decisions, code ownership, and verification
criteria.

## Build and verify

Install [GBDK 2020 4.5.0](https://github.com/gbdk-2020/gbdk-2020/releases/tag/4.5.0)
at `tools/gbdk`, or set `GBDK_ROOT` to its absolute directory. Install the pinned
Python development dependencies in your normal shared environment.

```sh
python3 -m pip install -r requirements-dev.txt
sh ./build.sh
python3 verify.py
python3 playtest.py
python3 verify_navigation.py
python3 package.py
```

The scripts resolve file paths from their own location. The build generates
artwork and instructions, then links only current C modules. Emulator tests
choose joypad input from live state; they never patch game RAM. To test exit
branches, they copy complete emulator checkpoints reached through normal play.
`verification/` holds public JSON and screenshot evidence tied to the ROM's
SHA-256. Private traces and atomic progress are in `private/` beside the main Git
common directory (or beside an exported source tree). They are excluded from Git
and the archive, as are SDKs, loader tools, and cartridge saves.

GitHub Actions independently builds and verifies the same flow on Linux.
Manual publication is a separate, explicit release action. An emulator pass
does not establish physical-device timing, controls, audio, or battery behavior.

## License

[MIT](LICENSE) for original game code, artwork, sound definitions, tests, and
documentation. No commercial ROMs or extracted game assets are included.
Toolchain/runtime components retain upstream licenses; see
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
