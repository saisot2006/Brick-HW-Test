#!/bin/sh
PAK_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
LOG_DIR="${LOGS_PATH:-$PAK_DIR/logs}"
mkdir -p "$LOG_DIR"
cd "$LOG_DIR" || exit 1

exec "$PAK_DIR/hwtest.elf"
