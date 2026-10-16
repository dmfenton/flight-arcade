# Third-party notices

Flight Arcade's game code, tile/font artwork, sound-effect definitions, puzzle generators, tests, and documentation were created for this project with Codex and are released under the [MIT License](LICENSE). It contains no third-party game ROMs or extracted game artwork or audio.

## GBDK 2020 and SDCC

The ROM is built with [GBDK 2020 4.5.0](https://github.com/gbdk-2020/gbdk-2020/releases/tag/4.5.0), which uses SDCC and includes its runtime libraries. GBDK credits Pascal Felber, Lars Malmborg, Michael Hope, David Galloway, John Fuge, and other contributors. The linked runtime components retain their upstream licenses; this project's MIT license does not relicense those components.

GBDK's [licensing FAQ](https://gbdk.org/docs/api/docs_faq.html) distinguishes distributing a compiled game ROM from redistributing the development environment. Its [license overview at version 4.5.0](https://github.com/gbdk-2020/gbdk-2020/blob/4.5.0/LICENSE) documents the library's GPLv2 with linking exception and the licenses of the tools. The unmodified SDK is downloaded separately and is excluded from this repository and release archive.

## Development-only dependencies

- [PyBoy](https://github.com/Baekalfen/PyBoy) runs automated emulator playtests. It is a separate development tool and is not embedded in the ROM or redistributed here.
- [Pillow](https://python-pillow.github.io/) assembles verification screenshots. It is a separate development dependency and is not embedded in the ROM or redistributed here.
- ModRetro's official firmware updater and `@modretro/chromatic-cli` can install the ROM on supported hardware. Neither is included in this repository or release archive. Their own terms apply separately.

Game Boy and Game Boy Color are Nintendo trademarks. ModRetro and Chromatic are ModRetro trademarks. These names identify hardware compatibility; this project is independent of those companies.
