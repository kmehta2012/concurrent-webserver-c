#!/bin/bash

# WebServer Development Environment Setup Script
# This script detects your OS and installs the necessary tools for the web server project

set -e  # Exit on any error

echo "==== Web Server Development Environment Setup ===="
echo "Detecting operating system..."

# Detect OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS detected
    echo "macOS detected"
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found. Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        
        # Add Homebrew to PATH if it's not already there
        if [[ $(uname -m) == 'arm64' ]]; then
            echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zshrc
            eval "$(/opt/homebrew/bin/brew shellenv)"
        fi
    else
        echo "Homebrew already installed"
    fi
    
    echo "Installing development tools..."
    brew update
    
    # Install required tools
    brew install llvm       # Includes clang compiler
    brew install make
    brew install cmake
    
    # Install debugging tools - Note: LLDB is already included with Xcode tools
    # Install Xcode Command Line Tools if not already installed
    if ! xcode-select -p &> /dev/null; then
        echo "Installing Xcode Command Line Tools..."
        xcode-select --install
        # This will prompt the user to install the tools
        echo "Please complete the Xcode Command Line Tools installation prompt"
        echo "and press Enter when finished..."
        read -r
    else
        echo "Xcode Command Line Tools already installed"
    fi
    
    # Install analysis tools
    brew install httpd      # Apache HTTP Server (includes ab)         
    brew install wireshark  # Packet analyzer
    
    # Add the Homebrew LLVM to PATH
    if ! grep -q "export PATH=\"/opt/homebrew/opt/llvm/bin:\$PATH\"" ~/.zshrc; then
        echo "Adding LLVM to your PATH..."
        echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.zshrc
        # Also set environment variables for compilers to find llvm
        echo 'export LDFLAGS="-L/opt/homebrew/opt/llvm/lib"' >> ~/.zshrc
        echo 'export CPPFLAGS="-I/opt/homebrew/opt/llvm/include"' >> ~/.zshrc
    else
        echo "LLVM is already in your PATH"
    fi
    
    # Add GNU Make to PATH so it's used instead of macOS Make
    if ! grep -q "PATH=\"/opt/homebrew/opt/make/libexec/gnubin:\$PATH\"" ~/.zshrc; then
        echo "Adding GNU Make to your PATH..."
        echo 'PATH="/opt/homebrew/opt/make/libexec/gnubin:$PATH"' >> ~/.zshrc
    else
        echo "GNU Make is already in your PATH"
    fi
    
    echo "Installing other useful utilities..."
    brew install jq         # JSON processor
    brew install httpie     # HTTP client
    
    # Apply changes to current shell
    echo "To apply these changes to your current terminal session, run:"
    echo "  source ~/.zshrc"
    
else
    # Linux detected
    echo "Linux detected"
    
    # Detect package manager
    if command -v apt-get &> /dev/null; then
        # Debian/Ubuntu
        echo "Debian/Ubuntu based system detected"
        
        echo "Updating package lists..."
        sudo apt-get update
        
        echo "Installing development tools..."
        sudo apt-get install -y build-essential
        sudo apt-get install -y gcc
        sudo apt-get install -y gdb
        sudo apt-get install -y make
        sudo apt-get install -y cmake
        
        echo "Installing analysis tools..."
        sudo apt-get install -y valgrind
        sudo apt-get install -y apache2-utils  # Includes ab (Apache Bench)
        sudo apt-get install -y wireshark-qt  # Packet analyzer
        
        echo "Installing other useful utilities..."
        sudo apt-get install -y jq       # JSON processor
        sudo apt-get install -y httpie   # HTTP client
        
    elif command -v dnf &> /dev/null; then
        # Fedora/CentOS/RHEL
        echo "Fedora/CentOS/RHEL based system detected"
        
        echo "Installing development tools..."
        sudo dnf groupinstall -y "Development Tools"
        sudo dnf install -y gcc
        sudo dnf install -y gdb
        sudo dnf install -y make
        sudo dnf install -y cmake
        
        echo "Installing analysis tools..."
        sudo dnf install -y valgrind
        sudo dnf install -y httpd-tools  # Includes ab (Apache Bench)
        sudo dnf install -y wireshark    # Packet analyzer
        
        echo "Installing other useful utilities..."
        sudo dnf install -y jq      # JSON processor
        sudo dnf install -y httpie  # HTTP client
        
    else
        echo "Unsupported Linux distribution. Please install the required packages manually."
        exit 1
    fi
fi

echo "==== Setup Complete ===="
echo "Development environment has been set up."
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "To apply PATH changes to your current terminal session, run:"
    echo "  source ~/.zshrc"
    echo ""
    echo "You now have:"
    echo "  - Clang compiler from Homebrew (will be used when you run 'clang')"
    echo "  - GNU Make (will be used when you run 'make')"
    echo "  - LLDB for debugging"
    echo "  - Other tools: cmake, ab, wireshark, httpie, jq"
fi