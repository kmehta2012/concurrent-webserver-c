#!/bin/bash

# Large File Generator for HTTP Server Testing
# Creates files of various sizes to test server performance and reliability

LARGE_DIR="public/static/large"
mkdir -p "$LARGE_DIR"

echo "📁 Creating large test files in: $LARGE_DIR"
echo "⚠️  Warning: This will create large files! Make sure you have enough disk space."
echo ""

# Function to create a file with specific content pattern
create_patterned_file() {
    local filename="$1"
    local size_mb="$2"
    local pattern="$3"
    
    echo "📝 Creating $filename (${size_mb}MB)..."
    
    # Calculate how many times to repeat the pattern
    local pattern_size=${#pattern}
    local target_bytes=$((size_mb * 1024 * 1024))
    local repetitions=$((target_bytes / pattern_size))
    
    # Create the file
    {
        for ((i=0; i<repetitions; i++)); do
            echo -n "$pattern"
        done
        # Add final newline
        echo ""
    } > "$LARGE_DIR/$filename"
    
    local actual_size=$(ls -lh "$LARGE_DIR/$filename" | awk '{print $5}')
    echo "✅ Created: $filename ($actual_size)"
}

# Function to create binary-like file (faster method)
create_large_file_fast() {
    local filename="$1"
    local size_mb="$2"
    
    echo "⚡ Creating $filename (${size_mb}MB) using fast method..."
    
    # Use dd for speed (creates file filled with zeros or random data)
    if [ "$3" = "random" ]; then
        # Random data (slower but more realistic)
        dd if=/dev/urandom of="$LARGE_DIR/$filename" bs=1M count="$size_mb" 2>/dev/null
    else
        # Zero-filled (very fast)
        dd if=/dev/zero of="$LARGE_DIR/$filename" bs=1M count="$size_mb" 2>/dev/null
    fi
    
    local actual_size=$(ls -lh "$LARGE_DIR/$filename" | awk '{print $5}')
    echo "✅ Created: $filename ($actual_size)"
}

# Ask user what sizes they want
echo "What file sizes would you like to create?"
echo "1) Small test files (1MB, 10MB, 50MB)"
echo "2) Medium test files (100MB, 250MB, 500MB)"  
echo "3) Large test files (1GB, 2GB)"
echo "4) All of the above"
echo "5) Custom sizes"
echo ""
read -p "Choose option (1-5): " choice

case $choice in
    1)
        sizes=(1 10 50)
        ;;
    2)
        sizes=(100 250 500)
        ;;
    3)
        sizes=(1024 2048)  # 1GB, 2GB
        ;;
    4)
        sizes=(1 10 50 100 250 500 1024)
        ;;
    5)
        echo "Enter sizes in MB separated by spaces (e.g., 100 500 1000):"
        read -p "Sizes: " custom_sizes
        sizes=($custom_sizes)
        ;;
    *)
        echo "Invalid choice, using small test files"
        sizes=(1 10 50)
        ;;
esac

echo ""
echo "📊 Will create files of sizes: ${sizes[@]} MB"
echo ""

# Create different types of large files
for size in "${sizes[@]}"; do
    echo "--- Creating ${size}MB files ---"
    
    # 1. Text file with repeating pattern
    create_patterned_file "text_${size}MB.txt" "$size" "This is line $(date +%s) with some text content to make a large file for testing the HTTP server robustness and performance under load. "
    
    # 2. "Binary" file (fast creation with zeros)
    create_large_file_fast "binary_${size}MB.bin" "$size" "zero"
    
    # 3. JSON-like structured data
    if [ "$size" -le 100 ]; then  # Only for smaller files (JSON generation is slow)
        echo "📄 Creating structured_${size}MB.json..."
        {
            echo '{"data": ['
            local records=$((size * 1024 * 10))  # Approximate number of records
            for ((i=1; i<=records; i++)); do
                echo "  {\"id\": $i, \"value\": \"data_item_$i\", \"timestamp\": $(date +%s)},"
            done | head -n -1  # Remove last comma
            echo ""
            echo "]}"
        } > "$LARGE_DIR/structured_${size}MB.json"
        local actual_size=$(ls -lh "$LARGE_DIR/structured_${size}MB.json" | awk '{print $5}')
        echo "✅ Created: structured_${size}MB.json ($actual_size)"
    fi
    
    echo ""
done

# Create one random data file for ultimate testing
if [ "$choice" != "1" ]; then
    echo "🎲 Creating random data file (this may take a while)..."
    create_large_file_fast "random_100MB.bin" "100" "random"
    echo ""
fi

echo "🎯 Large file generation complete!"
echo ""
echo "📁 Files created in: $LARGE_DIR"
ls -lh "$LARGE_DIR/"

echo ""
echo "🌐 Test URLs:"
for file in "$LARGE_DIR"/*; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        echo "   http://localhost:8080/static/large/$filename"
    fi
done

echo ""
echo "⚡ Performance Testing Tips:"
echo "1. Use 'time curl' to measure download speed:"
echo "   time curl -o /dev/null http://localhost:8080/static/large/text_50MB.txt"
echo ""
echo "2. Test with wget to see progress:"
echo "   wget http://localhost:8080/static/large/binary_100MB.bin"
echo ""
echo "3. Monitor server with 'top' or 'htop' during transfer"
echo ""
echo "4. Test browser behavior (may prompt for download on large files)"
echo ""
echo "5. Check server logs for any issues with large file serving"

echo ""
echo "🗑️  To clean up large files later:"
echo "   rm -rf $LARGE_DIR"

# Add to .gitignore if it exists
if [ -f ".gitignore" ]; then
    if ! grep -q "public/static/large/" .gitignore; then
        echo "" >> .gitignore
        echo "# Large test files" >> .gitignore
        echo "public/static/large/" >> .gitignore
        echo "📝 Added large files directory to .gitignore"
    fi
fi