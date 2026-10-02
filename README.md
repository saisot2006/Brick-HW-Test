# Brick HW Test

Native SDL2 hardware/ALSA diagnostic PAK for TrimUI Brick Pro / NextUI (tg5040).
Built using the same direct-ELF launch model as the supplied working MusicPlayer.pak.

## Important
This version does NOT call `show.elf` or `say.elf`. It runs `bin/hwtest.elf` directly.

## GitHub Actions
Run **Build Brick HW Test**. The workflow no longer requires the `zip` command; it creates the artifact with Python's built-in `zipfile` module.

Artifact: `Brick.HW.Test.pak.zip`

## Install
Extract the artifact so the PAK is under:
`Tools/tg5040/MusicPlayer.pak/`

The launcher writes diagnostics to NextUI's `LOGS_PATH` when available, otherwise to `MusicPlayer.pak/logs/`.
