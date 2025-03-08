#!/bin/bash
set -e

# Colors for better output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Git Shrub Installer${NC}"
echo "=============================="

# Check for gcc
if ! command -v gcc &> /dev/null; then
    echo -e "${RED}Error: gcc is not installed.${NC}"
    echo "Please install gcc/build-essential before continuing."
    exit 1
fi

# Define installation directories
SYSTEM_BIN="/usr/local/bin"
USER_BIN="$HOME/.local/bin"
USER_BIN_ALT="$HOME/bin"

# Compile the program
echo -e "\n${YELLOW}Compiling Git Shrub...${NC}"
gcc -Wall -O2 -o git-shrub shrub.c

# Make it executable
chmod +x git-shrub

# Determine installation location
echo -e "\n${YELLOW}Selecting installation location...${NC}"

# Ask if user wants system-wide or user installation
echo "Where would you like to install Git Shrub?"
echo "1) System-wide (${SYSTEM_BIN}, requires sudo)"
echo "2) User only (${USER_BIN})"
echo "3) User only alternate (${USER_BIN_ALT})"
echo "4) Custom location"

read -p "Select option [2]: " INSTALL_OPTION
INSTALL_OPTION=${INSTALL_OPTION:-2}

case $INSTALL_OPTION in
    1)
        INSTALL_DIR="${SYSTEM_BIN}"
        USE_SUDO=1
        ;;
    2)
        INSTALL_DIR="${USER_BIN}"
        USE_SUDO=0
        ;;
    3)
        INSTALL_DIR="${USER_BIN_ALT}"
        USE_SUDO=0
        ;;
    4)
        read -p "Enter custom installation directory: " CUSTOM_DIR
        INSTALL_DIR="${CUSTOM_DIR}"
        # Check if we need sudo for the custom directory
        if [[ ! -w $(dirname "$INSTALL_DIR") ]]; then
            USE_SUDO=1
        else
            USE_SUDO=0
        fi
        ;;
    *)
        echo -e "${RED}Invalid option. Exiting.${NC}"
        exit 1
        ;;
esac

# Create directory if it doesn't exist
echo -e "\n${YELLOW}Installing to ${INSTALL_DIR}...${NC}"

if [[ ! -d "$INSTALL_DIR" ]]; then
    if [[ $USE_SUDO -eq 1 ]]; then
        sudo mkdir -p "$INSTALL_DIR"
    else
        mkdir -p "$INSTALL_DIR"
    fi
fi

# Copy the executable
if [[ $USE_SUDO -eq 1 ]]; then
    sudo cp git-shrub "$INSTALL_DIR/"
else
    cp git-shrub "$INSTALL_DIR/"
fi

# Check if installation directory is in PATH
PATH_UPDATED=0
if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo -e "\n${YELLOW}Adding $INSTALL_DIR to your PATH...${NC}"
    
    # Determine which shell config file to update
    if [[ $SHELL == */zsh ]]; then
        SHELL_CONFIG="$HOME/.zshrc"
    elif [[ $SHELL == */bash ]]; then
        if [[ -f "$HOME/.bashrc" ]]; then
            SHELL_CONFIG="$HOME/.bashrc"
        else
            SHELL_CONFIG="$HOME/.bash_profile"
        fi
    else
        # Default to .profile for other shells
        SHELL_CONFIG="$HOME/.profile"
    fi
    
    # Create config file if it doesn't exist
    if [[ ! -f "$SHELL_CONFIG" ]]; then
        touch "$SHELL_CONFIG"
    fi
    
    # Add to PATH in shell config
    echo "export PATH=\"\$PATH:$INSTALL_DIR\"" >> "$SHELL_CONFIG"
    PATH_UPDATED=1
    
    echo -e "${GREEN}Added $INSTALL_DIR to PATH in $SHELL_CONFIG${NC}"
fi

# Clean up
rm -f git-shrub

echo -e "\n${GREEN}Installation complete!${NC}"

# Provide information on how to use
echo -e "\n${BLUE}Usage:${NC}"
echo "  cd /path/to/git/repository"
echo "  git shrub"

# Notify if PATH was updated and needs reloading
if [[ $PATH_UPDATED -eq 1 ]]; then
    echo -e "\n${YELLOW}Note:${NC} You need to restart your terminal or run:"
    echo "  source $SHELL_CONFIG"
    echo "for the PATH changes to take effect."
fi

echo -e "\n${GREEN}Thank you for installing Git Shrub!${NC}"
