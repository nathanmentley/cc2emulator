#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Define where the binary and libraries are relative to the script
# If you run this from the 'dist' folder, adjust these paths:
BIN_PATH="$SCRIPT_DIR/bin/Compucolor.Impl.App.Sfml"
LIB_PATH="$SCRIPT_DIR/lib"

# Check if the binary exists
if [ ! -f "$BIN_PATH" ]; then
    echo "Error: Emulator binary not found at $BIN_PATH"
    exit 1
fi

cd "$SCRIPT_DIR"

# Run the emulator
# Using "$@" allows you to pass command line arguments to the script
# which are then forwarded to the emulator.
exec "$BIN_PATH" "$@"