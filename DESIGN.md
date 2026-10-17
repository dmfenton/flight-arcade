# Flight Arcade: eight deeper games

This development branch replaces the sixteen short games with eight action games.
The direction is arcade play with unusual physics, scarce resources, and complete
runs. The published v0.1.0 and the installed cartridge remain separate artifacts.
A new release or cartridge write requires a request from the owner.

## Game decisions

| Game | Distinct mechanic | Decisions and progression |
|---|---|---|
| Star Skimmer | A gun that overheats; salvage buys invulnerability | Leave a safe firing lane to collect salvage; choose a gun upgrade or hull repair after each boss; clear three sectors. |
| Reactor Drop | Piece pressure raises garbage; clearing rows earns coolant | Save for a multi-row clear or spend three coolant to drop the active piece and remove garbage rows; stabilize three eight-line zones. Venting operates only on the locked board, so it lowers the stack without overlapping a falling piece. |
| Ricochet Works | Reversible gravity, magnetic catches, explosive neighbor chains | Spend scarce gravity flips or catch and re-aim using magnet energy; clear three rooms with three balls. |
| Gravity Rally | Moving paddle hits add spin; alternating wells curve the ball | Time a cooldown-limited dash for a fast return or spend flux to reverse the well; first to seven. |
| Tide Rider | Continuous boat momentum and reversing lateral current | Row or drag at a power cost; coast to recover; rescue survivors; choose a fast scoring reach or hull repair at two forks. |
| Pulse Serpent | Four-step phasing through tail, walls, and pulsing gates | Spend fruit power on boost or save three for a phase escape; choose a longer scoring coil or short charged coil; harvest three arenas. |
| Orbit Salvage | A spring tether transfers momentum between ship and cargo | Route pods around the core, brake and dock slowly, refuel at home; choose economy thrust or stronger tether; recover nine pods across three sectors. |
| Lunar Courier | Cargo mass, fragile landing limits, wind, and fuel across flights | Choose heavy ore or fragile glass contracts; sacrifice cargo to lighten the ship; stop at other ports or return home for fuel; complete five jobs. |

Every control and tactic page is owned by `src/games.json`. Runtime instructions,
menu labels, save-slot mapping, and test names are generated from it. Game logic
is owned by one C module per game. Shared rendering, input, pause, score, and save
behavior lives in `src/core.c`. Game-specific physics stay with their game.

## Save ownership

The battery RAM layout remains version 2: sixteen stable score slots plus sound
settings. The smaller catalog selects slots 0, 8, 2, 10, 6, 1, 13, and 15.
Unused legacy scores remain in their original slots; there is no migration or
erasure. The mapped slot is resolved only by `best_score` and `record_score`.
Some slots now describe redesigned successors, so existing high scores may be
hard to compare with the new scoring rules.

## Navigation guarantees

- Both instruction pages accept A to play and B to return to the menu.
- Every live game, docking screen, and upgrade/fork choice accepts Start to pause,
  then B to return to the arcade menu. Pause freezes simulation state.
- Choices also accept B directly. A result always offers replay and B offers menu.
- Replay resets campaign resources. Intermissions preserve the current run.
- Held buttons do not auto-select the next screen.

## Completion evidence

Build with pinned GBDK 4.5.0; play all eight in PyBoy using joypad input, without
writes to game RAM. Complete campaigns and exercise the distinctive mechanic,
resource cost, and intermission choices. Audit controls/tactics, back, pause,
resume, replay, terminal outcomes, and menu exits. Inspect screenshots of play,
choices, and results. Verify score/sound persistence and that all sixteen old RAM
slots survive a save. Package only a ROM and public source/evidence with matching
SHA-256. Emulator evidence does not establish physical Chromatic performance.
