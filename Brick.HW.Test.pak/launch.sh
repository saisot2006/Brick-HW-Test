#!/bin/sh
PAK_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$PAK_DIR" || exit 1

LOG_DIR="${LOGS_PATH:-$PAK_DIR/logs}"
mkdir -p "$LOG_DIR" 2>/dev/null || LOG_DIR="$PAK_DIR/logs"
mkdir -p "$LOG_DIR" 2>/dev/null

# SAFE MODE: do not run aplay or any potentially blocking audio command here.
# Only read proc/sys information; the native UI handles exit immediately.
{
  echo "== Brick HW Test =="
  echo "date: $(date)"
  echo "uname: $(uname -a)"
  echo "model:"
  cat /proc/device-tree/model 2>/dev/null | tr -d '\000'; echo
  echo "== audio env =="
  env | grep -i -E 'SDL|ALSA|AUDIO|PLATFORM|DEVICE' 2>/dev/null || true
  echo "== /proc/asound/cards =="
  cat /proc/asound/cards 2>/dev/null || true
  echo "== /proc/asound/pcm =="
  cat /proc/asound/pcm 2>/dev/null || true
  echo "== /proc/asound/card0 (read-only) =="
  find /proc/asound/card0 -maxdepth 4 -type f -print 2>/dev/null | sort | while read f; do
    echo "--- $f"
    cat "$f" 2>/dev/null | head -80 || true
  done
  echo "== /sys/class/sound =="
  find /sys/class/sound -maxdepth 3 -type f -print 2>/dev/null | sort | while read f; do
    echo "--- $f"
    cat "$f" 2>/dev/null | head -80 || true
  done
  echo "== PCM sysfs =="
  for d in /sys/class/sound/pcmC0D0p /sys/class/sound/pcmC0D0c; do
    if [ -d "$d" ]; then
      echo "--- $d"
      readlink -f "$d" 2>/dev/null || true
      find "$d" -maxdepth 3 -type f -print 2>/dev/null | sort | while read f; do
        echo "--- $f"
        cat "$f" 2>/dev/null | head -80 || true
      done
    fi
  done
} > "$LOG_DIR/Brick_HW_Test_raw.txt" 2>&1

exec ./bin/hwtest.elf
