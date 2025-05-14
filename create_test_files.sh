#!/bin/bash

# Script to create dummy test files for MIME type testing
# Run from project root directory

# Define base directory
BASE_DIR="public/static"

# Create directories if they don't exist
mkdir -p "$BASE_DIR/media"
mkdir -p "$BASE_DIR/misc"

# ----- Create dummy image files -----
echo "Creating dummy image files..."

# PNG file
cat > "$BASE_DIR/media/dummy.png" << 'EOF'
This is a dummy PNG file for testing image/png MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# JPEG file
cat > "$BASE_DIR/media/dummy.jpg" << 'EOF'
This is a dummy JPEG file for testing image/jpeg MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# GIF file
cat > "$BASE_DIR/media/dummy.gif" << 'EOF'
This is a dummy GIF file for testing image/gif MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# SVG file
cat > "$BASE_DIR/media/dummy.svg" << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">
 <rect width="100" height="100" fill="blue"/>
 <text x="10" y="50" fill="white">SVG Test</text>
</svg>
EOF

# ----- Create dummy audio files -----
echo "Creating dummy audio files..."

# MP3 file
cat > "$BASE_DIR/media/dummy.mp3" << 'EOF'
This is a dummy MP3 file for testing audio/mpeg MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# WAV file
cat > "$BASE_DIR/media/dummy.wav" << 'EOF'
This is a dummy WAV file for testing audio/wav MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# ----- Create dummy video files -----
echo "Creating dummy video files..."

# MP4 file
cat > "$BASE_DIR/media/dummy.mp4" << 'EOF'
This is a dummy MP4 file for testing video/mp4 MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# WebM file
cat > "$BASE_DIR/media/dummy.webm" << 'EOF'
This is a dummy WebM file for testing video/webm MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# ----- Create dummy document files -----
echo "Creating dummy document files..."

# PDF file
cat > "$BASE_DIR/misc/dummy.pdf" << 'EOF'
This is a dummy PDF file for testing application/pdf MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# ZIP file
cat > "$BASE_DIR/misc/dummy.zip" << 'EOF'
This is a dummy ZIP file for testing application/zip MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# ----- Create dummy font files -----
echo "Creating dummy font files..."

# WOFF file
cat > "$BASE_DIR/misc/dummy.woff" << 'EOF'
This is a dummy WOFF font file for testing font/woff MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# WOFF2 file
cat > "$BASE_DIR/misc/dummy.woff2" << 'EOF'
This is a dummy WOFF2 font file for testing font/woff2 MIME type detection.
The actual content doesn't matter since we're only testing the extension-to-MIME mapping.
EOF

# ----- Create special case files -----
echo "Creating special case files..."

# PostScript file
cat > "$BASE_DIR/misc/dummy.ps" << 'EOF'
%!PS-Adobe-3.0
% This is a dummy PostScript file for testing application/postscript MIME type detection.
/Helvetica findfont 12 scalefont setfont
72 72 moveto
(Test PostScript Document) show
showpage
EOF

# File with multiple extensions
cat > "$BASE_DIR/misc/dummy.tar.gz" << 'EOF'
This is a dummy compressed tarball file.
Testing multiple extensions and how they're handled by the MIME type detection.
EOF

# File with no extension
cat > "$BASE_DIR/misc/noextension" << 'EOF'
This file has no extension.
It should default to text/plain MIME type.
EOF

echo "Done! Created all dummy test files."
echo "Files are organized in:"
echo "- $BASE_DIR/media/ (images, audio, video)"
echo "- $BASE_DIR/misc/ (documents, fonts, special cases)"