###############################################################################
# Source files

# The source files to compile.
SRC      := main.c button_map.c device_drivers/dj_hero_turntable.c device_drivers/guitar_hero_drums.c device_drivers/guitar_hero_guitar.c device_drivers/santroller.c
# Include directories
INC_DIRS :=
# Library directories
LIB_DIRS :=
# The names of libraries to use.
LIBS     :=
# The output directory for compiled results.
BIN      := bin
# The name of the output file to generate.
TARGET 	 := $(BIN)/$(notdir $(CURDIR)).mod
# C compiler flags
CFLAGS   :=
