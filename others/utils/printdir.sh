#!/bin/bash

# Auxiliar tool for recursively extract folder structure

# Colors
NC="\e[0m"
RED="\e[1;31m"
GREEN="\e[32m"
BLUE="\e[34m"

DIR="${HOME}/Patch"

F_GIT=".git"
F_THIRD="third_party"

function main() {
    if [ ! -d "$DIR" ]; then
        echo -e "${RED}[!] Error: Directory $DIR does not exist.${NC}"
        exit 1
    fi

    if ! command -v tree &> /dev/null; then
        echo -e "${RED}[!] Error: 'tree' is not installed. Try: sudo dnf install tree${NC}"
        exit 1
    fi

    if ! command -v make &> /dev/null; then
        echo -e "${RED}[!] Error: 'make' is not installed. Try: sudo dnf install make${NC}"
        exit 1
    fi

    echo -e "${BLUE}Removing temporal files...${NC}"
    if [ -f "$DIR/engine/makefile" ]; then
        make -C "$DIR/engine" clean > /dev/null
    else
        echo -e "${RED}[!] $DIR/engine/makefile file not found.${NC}"
    fi

    echo -e "${BLUE}Structure of: ${GREEN}$DIR${NC}"
    echo -e "${RED}Excluding: $F_THIRD and $F_GIT${NC}\n"

    tree -I "$F_THIRD|$F_GIT" "$DIR"
}

main