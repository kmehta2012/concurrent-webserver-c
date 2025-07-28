#!/bin/bash

# Parallel Load Test Script for HTTP Server
# Tests multiple large files simultaneously to validate server behavior

SERVER_URL="http://localhost:8080"
LARGE_DIR="public/static/large"
RESULTS_DIR="test_results"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🚀 HTTP Server Parallel Load Test${NC}"
echo "Testing server at: $SERVER_URL"
echo "=================================================="

# Create results directory
mkdir -p "$RESULTS_DIR"
cd "$RESULTS_DIR" || exit 1

# Clean up any previous test files
echo -e "${YELLOW}🧹 Cleaning up previous test files...${NC}"
rm -f parallel_test_*.bin parallel_test_*.txt parallel_test_*.json
rm -f test_*.log

echo ""

# Function to run a parallel test
run_parallel_test() {
    local test_name="$1"
    local file_pattern="$2"
    local num_clients="$3"
    
    echo -e "${BLUE}📊 Test: $test_name${NC}"
    echo "Files matching: $file_pattern"
    echo "Concurrent clients: $num_clients"
    echo "Starting test..."
    
    # Find matching files
    local files=(../$LARGE_DIR/$file_pattern)
    local available_files=()
    
    # Check which files actually exist
    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            available_files+=("$file")
        fi
    done
    
    if [[ ${#available_files[@]} -eq 0 ]]; then
        echo -e "${RED}❌ No files found matching pattern: $file_pattern${NC}"
        echo ""
        return 1
    fi
    
    echo "Found ${#available_files[@]} files: ${available_files[*]##*/}"
    
    # Start parallel downloads
    local pids=()
    local start_time=$(date +%s.%N)
    
    echo "Starting $num_clients parallel downloads..."
    
    for ((i=1; i<=num_clients; i++)); do
        # Cycle through available files
        local file_index=$(((i-1) % ${#available_files[@]}))
        local source_file="${available_files[$file_index]}"
        local filename=$(basename "$source_file")
        local output_file="parallel_test_${i}_${filename}"
        
        # Start download in background with timing
        (
            echo "Client $i: Starting download of $filename at $(date +%T.%3N)" > "test_${i}.log"
            time curl -s -o "$output_file" "$SERVER_URL/static/large/$filename" 2>> "test_${i}.log"
            echo "Client $i: Finished download of $filename at $(date +%T.%3N)" >> "test_${i}.log"
        ) &
        
        pids+=($!)
        echo "  Started client $i (PID: $!) downloading: $filename"
    done
    
    echo "All clients started. Waiting for completion..."
    
    # Wait for all downloads to complete
    wait
    
    local end_time=$(date +%s.%N)
    local total_time=$(echo "$end_time - $start_time" | bc)
    
    echo ""
    echo -e "${GREEN}✅ All downloads completed!${NC}"
    echo "Total wall-clock time: ${total_time}s"
    
    # Analyze results
    echo ""
    echo "📋 Results Analysis:"
    echo "File                                    Status    Size      Expected"
    echo "---------------------------------------------------------------------"
    
    local success_count=0
    local total_downloaded=0
    
    for ((i=1; i<=num_clients; i++)); do
        local file_index=$(((i-1) % ${#available_files[@]}))
        local source_file="${available_files[$file_index]}"
        local filename=$(basename "$source_file")
        local output_file="parallel_test_${i}_${filename}"
        local expected_size=$(ls -l "$source_file" 2>/dev/null | awk '{print $5}')
        
        if [[ -f "$output_file" ]]; then
            local actual_size=$(ls -l "$output_file" | awk '{print $5}')
            local human_size=$(ls -lh "$output_file" | awk '{print $5}')
            
            if [[ "$actual_size" -eq "$expected_size" ]]; then
                echo -e "${output_file:0:39} ${GREEN}✅ OK${NC}     ${human_size}     ${human_size}"
                ((success_count++))
                total_downloaded=$((total_downloaded + actual_size))
            else
                echo -e "${output_file:0:39} ${RED}❌ SIZE${NC}   ${human_size}     $(ls -lh "$source_file" | awk '{print $5}')"
            fi
        else
            echo -e "${output_file:0:39} ${RED}❌ MISSING${NC} -         $(ls -lh "$source_file" | awk '{print $5}')"
        fi
    done
    
    echo "---------------------------------------------------------------------"
    echo "Success rate: $success_count/$num_clients ($(echo "scale=1; $success_count * 100 / $num_clients" | bc)%)"
    
    if [[ $total_downloaded -gt 0 ]]; then
        local total_mb=$(echo "scale=2; $total_downloaded / 1024 / 1024" | bc)
        local speed_mbps=$(echo "scale=2; $total_mb / $total_time" | bc)
        echo "Total data transferred: ${total_mb} MB"
        echo "Average throughput: ${speed_mbps} MB/s"
    fi
    
    # Show timing details
    echo ""
    echo "⏱️  Individual Client Timings:"
    for ((i=1; i<=num_clients; i++)); do
        if [[ -f "test_${i}.log" ]]; then
            echo "Client $i:"
            grep -E "(Starting|Finished|real|user|sys)" "test_${i}.log" | sed 's/^/  /'
        fi
    done
    
    echo ""
    echo "=================================================="
    echo ""
}

# Test 1: Small files (multiple clients per file)
echo -e "${YELLOW}🧪 Running Test Suite...${NC}"
echo ""

# Test with 1MB files (if available)
run_parallel_test "Small Files (1MB) - 3 Clients" "*1MB*" 3

# Test with 10MB files
run_parallel_test "Medium Files (10MB) - 5 Clients" "*10MB*" 5

# Test with 50MB files  
run_parallel_test "Large Files (50MB) - 3 Clients" "*50MB*" 3

# Test with 100MB files (if available)
run_parallel_test "Very Large Files (100MB) - 3 Clients" "*100MB*" 3

# NEW: Mixed workload test - all different files at once
echo -e "${YELLOW}🌈 Mixed Workload Test - All Different Files Simultaneously${NC}"
echo "This test downloads ALL available large files concurrently"
echo "Each client gets a different file to simulate real-world mixed traffic"
echo ""

# Find all available large files
mixed_files=(../$LARGE_DIR/*)
available_mixed=()

for file in "${mixed_files[@]}"; do
    if [[ -f "$file" && $(basename "$file") != "." && $(basename "$file") != ".." ]]; then
        available_mixed+=("$file")
    fi
done

if [[ ${#available_mixed[@]} -gt 0 ]]; then
    echo "Found ${#available_mixed[@]} files for mixed workload test:"
    for file in "${available_mixed[@]}"; do
        local size=$(ls -lh "$file" | awk '{print $5}')
        echo "  $(basename "$file") ($size)"
    done
    echo ""
    
    echo "Starting mixed workload test with ${#available_mixed[@]} concurrent downloads..."
    
    local pids=()
    local start_time=$(date +%s.%N)
    
    for ((i=0; i<${#available_mixed[@]}; i++)); do
        local source_file="${available_mixed[$i]}"
        local filename=$(basename "$source_file")
        local output_file="mixed_test_${filename}"
        local client_num=$((i+1))
        
        # Start download in background with timing
        (
            echo "Mixed Client $client_num: Starting download of $filename at $(date +%T.%3N)" > "mixed_test_${client_num}.log"
            time curl -s -o "$output_file" "$SERVER_URL/static/large/$filename" 2>> "mixed_test_${client_num}.log"
            echo "Mixed Client $client_num: Finished download of $filename at $(date +%T.%3N)" >> "mixed_test_${client_num}.log"
        ) &
        
        pids+=($!)
        echo "  Started mixed client $client_num (PID: $!) downloading: $filename"
    done
    
    echo "All mixed clients started. Waiting for completion..."
    wait
    
    local end_time=$(date +%s.%N)
    local total_time=$(echo "$end_time - $start_time" | bc)
    
    echo ""
    echo -e "${GREEN}✅ Mixed workload test completed!${NC}"
    echo "Total wall-clock time: ${total_time}s"
    
    # Analyze mixed results
    echo ""
    echo "📋 Mixed Workload Results:"
    echo "File                                    Status    Size      Client"
    echo "---------------------------------------------------------------------"
    
    local success_count=0
    local total_downloaded=0
    
    for ((i=0; i<${#available_mixed[@]}; i++)); do
        local source_file="${available_mixed[$i]}"
        local filename=$(basename "$source_file")
        local output_file="mixed_test_${filename}"
        local expected_size=$(ls -l "$source_file" 2>/dev/null | awk '{print $5}')
        local client_num=$((i+1))
        
        if [[ -f "$output_file" ]]; then
            local actual_size=$(ls -l "$output_file" | awk '{print $5}')
            local human_size=$(ls -lh "$output_file" | awk '{print $5}')
            
            if [[ "$actual_size" -eq "$expected_size" ]]; then
                echo -e "${filename:0:39} ${GREEN}✅ OK${NC}     ${human_size}     Client $client_num"
                ((success_count++))
                total_downloaded=$((total_downloaded + actual_size))
            else
                echo -e "${filename:0:39} ${RED}❌ SIZE${NC}   ${human_size}     Client $client_num"
            fi
        else
            echo -e "${filename:0:39} ${RED}❌ MISSING${NC} -         Client $client_num"
        fi
    done
    
    echo "---------------------------------------------------------------------"
    echo "Mixed workload success rate: $success_count/${#available_mixed[@]} ($(echo "scale=1; $success_count * 100 / ${#available_mixed[@]}" | bc)%)"
    
    if [[ $total_downloaded -gt 0 ]]; then
        local total_mb=$(echo "scale=2; $total_downloaded / 1024 / 1024" | bc)
        local speed_mbps=$(echo "scale=2; $total_mb / $total_time" | bc)
        echo "Total mixed data transferred: ${total_mb} MB"
        echo "Average mixed throughput: ${speed_mbps} MB/s"
    fi
    
    # Show mixed timing details
    echo ""
    echo "⏱️  Mixed Workload Client Timings:"
    for ((i=1; i<=${#available_mixed[@]}; i++)); do
        if [[ -f "mixed_test_${i}.log" ]]; then
            echo "Mixed Client $i:"
            grep -E "(Starting|Finished|real|user|sys)" "mixed_test_${i}.log" | sed 's/^/  /'
        fi
    done
    
    echo ""
    echo "🎯 For Stage 2 (sequential server), you should see:"
    echo "  📅 Different finish times (clients complete one after another)"
    echo "  🔄 Sequential processing order (first started = first finished for similar sizes)"
    echo "  ⚡ Individual clients get full speed when it's their turn"
    echo ""
    echo "=================================================="
    echo ""
else
    echo -e "${RED}❌ No files found for mixed workload test${NC}"
    echo ""
fi

# Stress test: Many small concurrent requests
run_parallel_test "Stress Test (10MB) - 10 Clients" "*10MB*" 10

# Ultimate test: Large files with many clients (if available)
if ls ../$LARGE_DIR/*1024MB* >/dev/null 2>&1; then
    echo -e "${RED}⚠️  WARNING: About to test 1GB files with multiple clients${NC}"
    echo "This will download several GB of data and may take time."
    read -p "Continue? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        run_parallel_test "Ultimate Test (1GB) - 2 Clients" "*1024MB*" 2
    else
        echo "Skipping 1GB test."
    fi
fi

# Summary
echo -e "${GREEN}🎯 Test Suite Complete!${NC}"
echo ""
echo "📁 Test files saved in: $RESULTS_DIR/"
echo "📊 Individual client logs: test_*.log, mixed_test_*.log"
echo ""
echo "🧪 Tests performed:"
echo "  1️⃣  Small Files (1MB) - 3 clients per file type"
echo "  2️⃣  Medium Files (10MB) - 5 clients per file type"  
echo "  3️⃣  Large Files (50MB) - 3 clients per file type"
echo "  4️⃣  Very Large Files (100MB) - 3 clients per file type"
echo "  🌈 Mixed Workload - ALL different files simultaneously"
echo "  💥 Stress Test (10MB) - 10 clients"
echo "  🔥 Ultimate Test (1GB) - 2 clients (if enabled)"
echo ""
echo "🔍 Key things to verify for your Stage 2 (sequential) server:"
echo "  ✅ All downloads should complete successfully"
echo "  ✅ File sizes should match exactly"  
echo "  ⏱️  Downloads should happen sequentially (different finish times)"
echo "  🌈 Mixed workload shows clear sequential processing pattern"
echo "  💾 Server memory usage should remain constant during test"
echo ""
echo "💡 Pro tip: Monitor your server with 'htop' during these tests!"
echo ""
echo "🗑️  To clean up test files: rm -rf $RESULTS_DIR"