#!/bin/bash

# Script to generate small test media files for HTTP server testing
# Run from project root directory

MEDIA_DIR="public/static/media"
mkdir -p "$MEDIA_DIR"

echo "🎬 Generating test media files..."

# ===== IMAGE FILES =====

# Generate tiny PNG (1x1 pixel red square)
if command -v convert &> /dev/null || command -v magick &> /dev/null; then
    echo "📸 Creating tiny.png (1x1 red pixel)"
    if command -v magick &> /dev/null; then
        magick -size 1x1 xc:red "$MEDIA_DIR/tiny.png"
    else
        convert -size 1x1 xc:red "$MEDIA_DIR/tiny.png"
    fi
else
    echo "⚠️  ImageMagick not found, skipping PNG generation"
fi

# Generate small JPEG (10x10 blue square)
if command -v convert &> /dev/null || command -v magick &> /dev/null; then
    echo "📸 Creating small.jpg (10x10 blue square)"
    if command -v magick &> /dev/null; then
        magick -size 10x10 xc:blue "$MEDIA_DIR/small.jpg"
    else
        convert -size 10x10 xc:blue "$MEDIA_DIR/small.jpg"
    fi
else
    echo "⚠️  ImageMagick not found, skipping JPEG generation"
fi

# Create minimal SVG
echo "🖼️  Creating icon.svg"
cat > "$MEDIA_DIR/icon.svg" << 'EOF'
<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 16 16">
  <circle cx="8" cy="8" r="6" fill="green"/>
  <text x="8" y="12" text-anchor="middle" font-size="8" fill="white">✓</text>
</svg>
EOF

# Create animated GIF (requires ImageMagick)
if command -v convert &> /dev/null; then
    echo "🎨 Creating animated.gif (2-frame animation)"
    convert -size 20x20 xc:red -delay 50 -size 20x20 xc:blue -delay 50 -loop 0 "$MEDIA_DIR/animated.gif"
else
    echo "⚠️  ImageMagick not found, skipping GIF generation"
fi

# ===== AUDIO FILES =====

# Generate tiny WAV file (1 second of silence)
if command -v sox &> /dev/null; then
    echo "🎵 Creating silence.wav (1 second of silence)"
    sox -n -r 8000 -c 1 "$MEDIA_DIR/silence.wav" trim 0.0 1.0
else
    echo "⚠️  Sox not found, skipping WAV generation"
fi

# Generate MP3 from WAV if both tools available
if command -v sox &> /dev/null && command -v lame &> /dev/null; then
    echo "🎵 Creating silence.mp3"
    lame -b 32 "$MEDIA_DIR/silence.wav" "$MEDIA_DIR/silence.mp3" 2>/dev/null
elif command -v ffmpeg &> /dev/null; then
    echo "🎵 Creating silence.mp3 with ffmpeg"
    ffmpeg -f lavfi -i "sine=frequency=440:duration=1" -ac 1 -ar 8000 -b:a 32k "$MEDIA_DIR/silence.mp3" -y 2>/dev/null
else
    echo "⚠️  Neither lame nor ffmpeg found, skipping MP3 generation"
fi

# ===== VIDEO FILES =====

# Generate tiny MP4 video (1 second, 1 frame per second)
if command -v ffmpeg &> /dev/null; then
    echo "🎬 Creating tiny.mp4 (1 second red square)"
    ffmpeg -f lavfi -i "color=red:size=32x32:duration=1:rate=1" -c:v libx264 -pix_fmt yuv420p "$MEDIA_DIR/tiny.mp4" -y 2>/dev/null
else
    echo "⚠️  ffmpeg not found, skipping MP4 generation"
fi

# Generate WebM video 
if command -v ffmpeg &> /dev/null; then
    echo "🎬 Creating tiny.webm (1 second blue square)"
    ffmpeg -f lavfi -i "color=blue:size=32x32:duration=1:rate=1" -c:v libvpx -b:v 32k "$MEDIA_DIR/tiny.webm" -y 2>/dev/null
else
    echo "⚠️  ffmpeg not found, skipping WebM generation"
fi

# ===== FONT FILES =====

# Create minimal WOFF font (this is tricky, might need to download or skip)
echo "🔤 Font files require external tools - creating placeholder files for now"
echo "Placeholder WOFF font file for testing" > "$MEDIA_DIR/test.woff"
echo "Placeholder WOFF2 font file for testing" > "$MEDIA_DIR/test.woff2"

# ===== DOCUMENT FILES =====

# Create minimal PDF using ImageMagick
if command -v convert &> /dev/null || command -v magick &> /dev/null; then
    echo "📄 Creating test.pdf"
    if command -v magick &> /dev/null; then
        magick -size 100x100 xc:white -pointsize 12 -draw "text 10,50 'Test PDF'" "$MEDIA_DIR/test.pdf"
    else
        convert -size 100x100 xc:white -pointsize 12 -draw "text 10,50 'Test PDF'" "$MEDIA_DIR/test.pdf"
    fi
else
    echo "⚠️  ImageMagick not found, skipping PDF generation"
fi

echo ""
echo "✅ Media file generation complete!"
echo "📁 Files created in: $MEDIA_DIR"
echo ""
echo "📋 Generated files:"
ls -la "$MEDIA_DIR/" | grep -E '\.(png|jpg|svg|gif|wav|mp3|mp4|webm|woff|woff2|pdf)$' || echo "   (Check for any generation errors above)"

echo ""
echo "🧪 Test in browser:"
echo "   http://localhost:8080/static/media/tiny.png"
echo "   http://localhost:8080/static/media/small.jpg"
echo "   http://localhost:8080/static/media/icon.svg"
echo "   http://localhost:8080/static/media/tiny.mp4"

echo ""
echo "📦 Dependencies used:"
echo "   ImageMagick: $(command -v convert &> /dev/null && echo "✅ Found" || echo "❌ Not found")"
echo "   Sox: $(command -v sox &> /dev/null && echo "✅ Found" || echo "❌ Not found")"
echo "   FFmpeg: $(command -v ffmpeg &> /dev/null && echo "✅ Found" || echo "❌ Not found")"
echo "   Lame: $(command -v lame &> /dev/null && echo "✅ Found" || echo "❌ Not found")"