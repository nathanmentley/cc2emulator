#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Define where the binary is relative to the script
BIN_PATH="$SCRIPT_DIR/bin/Compucolor.Impl.App.Headless"

# The headless app links the same hardware libraries that the GUI loads as
# plugins, so make them discoverable before starting it.
export LD_LIBRARY_PATH="$SCRIPT_DIR/plugins${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# Check if the binary exists
if [ ! -f "$BIN_PATH" ]; then
    echo "Error: Headless replay binary not found at $BIN_PATH"
    exit 1
fi

cd "$SCRIPT_DIR"

# Run the headless replay app. Arguments are forwarded unchanged, so paths are
# resolved relative to the dist directory, matching cc2_emu.sh.
exec "$BIN_PATH" "$@"
