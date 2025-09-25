# =================================================================================
# ==                  Makefile for ESP32 Arduino CLI Project                   ==
# =================================================================================

# --- CONFIGURATION ---
# IMPORTANT: Update these to match your board and serial port.
# Find your FQBN with `arduino-cli board listall`
# Find your PORT with `arduino-cli board list`
FQBN        := esp32:esp32:esp32
PORT        := $(shell arduino-cli board list | grep usb | awk '{print $$1}')

# --- PROJECT VARS ---
SKETCH_NAME := $(shell basename `pwd`)
SKETCH_INO  := $(SKETCH_NAME).ino
BUILD_DIR   := build

# =================================================================================
# ==                             COMMANDS                                        ==
# =================================================================================
.PHONY: all compile build upload monitor clean deps help

# The default command when running `make`
all: build upload

# Compiles the sketch with verbose output.
compile:
	@echo "--- Compiling sketch: $(SKETCH_INO) for $(FQBN) ---"
	arduino-cli compile --fqbn $(FQBN) --output-dir $(BUILD_DIR)

build: compile

# Compiles and then uploads the sketch with verbose output and reduced speed.
upload:
	@echo "--- Uploading to board on port $(PORT) ---"
	arduino-cli upload -v -p $(PORT) --fqbn $(FQBN) --input-dir $(BUILD_DIR) --board-options UploadSpeed=115200

# Opens the serial monitor to view output from the board.
monitor:
	@echo "--- Opening Serial Monitor on $(PORT) (Press Ctrl+C to exit) ---"
	arduino-cli monitor -p $(PORT) --config baudrate=115200

# Deletes the build directory and all compiled files.
clean:
	@echo "--- Cleaning build directory ---"
	@rm -rf $(BUILD_DIR)

# Installs required libraries (FastLED) from their official Git repositories.
deps:
	@echo "--- Installing project dependencies ---"
	@arduino-cli lib install --git-url https://github.com/FastLED/FastLED.git

# Displays a help message with available commands.
help:
	@echo "Makefile for ESP32 Arduino CLI Project"
	@echo "--------------------------------------"
	@echo "Usage:"
	@echo "  make compile - Compiles the sketch without uploading."
	@echo "  make upload  - Compiles and uploads the sketch (default)."
	@echo "  make monitor - Opens the serial monitor to view board output."
	@echo "  make clean   - Removes all compiled files and the build directory."
	@echo "  make deps    - Installs required libraries (FastLED)."
	@echo "  make help    - Shows this help message."

