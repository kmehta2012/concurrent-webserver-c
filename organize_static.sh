#!/bin/bash

# Set the target static directory
STATIC_DIR="public/static"

# Create subdirectories
mkdir -p "$STATIC_DIR/css"
mkdir -p "$STATIC_DIR/js"
mkdir -p "$STATIC_DIR/html"
mkdir -p "$STATIC_DIR/text"
mkdir -p "$STATIC_DIR/media"
mkdir -p "$STATIC_DIR/misc"

# Loop through files in static/
for file in "$STATIC_DIR"/*; do
  [ -f "$file" ] || continue  # Skip if not a regular file

  ext="${file##*.}"
  base="${file##*/}"

  case "$ext" in
    html)
      mv "$file" "$STATIC_DIR/html/$base"
      ;;
    css)
      mv "$file" "$STATIC_DIR/css/$base"
      ;;
    js)
      mv "$file" "$STATIC_DIR/js/$base"
      ;;
    txt|json|xml|log|md)
      mv "$file" "$STATIC_DIR/text/$base"
      ;;
    jpg|jpeg|png|gif|svg|webm|mp4|mp3|wav)
      mv "$file" "$STATIC_DIR/media/$base"
      ;;
    ps|zip|woff|woff2|gz|tar)
      mv "$file" "$STATIC_DIR/misc/$base"
      ;;
    *)  # Files with no extension or unknown types
      mv "$file" "$STATIC_DIR/misc/$base"
      ;;
  esac
done

echo "✅ Files in $STATIC_DIR have been organized into type-based subdirectories."