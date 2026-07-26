#!/usr/bin/env bash
#
# encode.sh — turn a directory of frame_%04d.png captures into the demo assets.
#
# Produces an animated WebP (primary, referenced by the README) and an animated
# GIF (fallback for anything that doesn't render WebP). Requires ffmpeg.
#
# Usage: encode.sh <frames_dir> <out_dir> [fps]
set -euo pipefail

FRAMES="${1:?usage: encode.sh <frames_dir> <out_dir> [fps]}"
OUT="${2:?usage: encode.sh <frames_dir> <out_dir> [fps]}"
FPS="${3:-30}"

mkdir -p "$OUT"

echo ">> Encoding animated WebP..."
ffmpeg -y -hide_banner -loglevel error \
  -framerate "$FPS" -i "$FRAMES/frame_%04d.png" \
  -c:v libwebp -lossless 0 -quality 80 -preset drawing \
  -loop 0 -an -fps_mode passthrough \
  "$OUT/ui-demo.webp"

echo ">> Encoding animated GIF (fallback)..."
PAL="$(mktemp --suffix=.png)"
ffmpeg -y -hide_banner -loglevel error \
  -framerate "$FPS" -i "$FRAMES/frame_%04d.png" \
  -vf "fps=$FPS,palettegen=stats_mode=diff" "$PAL"
ffmpeg -y -hide_banner -loglevel error \
  -framerate "$FPS" -i "$FRAMES/frame_%04d.png" -i "$PAL" \
  -lavfi "fps=$FPS,paletteuse=dither=bayer:bayer_scale=3" \
  -loop 0 "$OUT/ui-demo.gif"
rm -f "$PAL"

# A still poster (first frame) for embeds that want a static image.
cp "$FRAMES/frame_0000.png" "$OUT/ui-poster.png"

echo ">> Done:"
ls -lh "$OUT"/ui-demo.webp "$OUT"/ui-demo.gif "$OUT"/ui-poster.png
