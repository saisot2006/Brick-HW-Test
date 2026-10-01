#!/bin/sh
PAK_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$PAK_DIR" || exit 1

# Keep the same direct-ELF launch model as the known-working MusicPlayer.pak.
# Never call show.elf/say.elf.
{
  echo "== Brick HW Test =="
  echo "date: $(date)"
  echo "uname: $(uname -a)"
  echo "model:"
  cat /proc/device-tree/model 2>/dev/null | tr -d '\000'; echo
  echo "== audio env =="
  env | grep -i -E 'SDL|ALSA|AUDIO|PLATFORM|DEVICE' 2>/dev/null
  echo "== aplay -l =="
  command -v aplay >/dev/null 2>&1 && aplay -l 2>&1 || echo "aplay: unavailable"
  echo "== aplay hw params (default) =="
  command -v aplay >/dev/null 2>&1 && aplay -D default --dump-hw-params /dev/zero 2>&1 | head -160 || true
  echo "== /proc/asound =="
  cat /proc/asound/cards 2>/dev/null
  cat /proc/asound/pcm 2>/dev/null
} > "$LOGS_PATH/Brick_HW_Test_raw.txt" 2>&1

./bin/hwtest.elf > "$LOGS_PATH/Brick_HW_Test.txt" 2>&1
exit $?
