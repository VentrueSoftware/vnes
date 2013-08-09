###############################################
# VConfig Configuration Parser Makefile       #
# Author: Kurt Sassenrath                     #
###############################################

#============ Project Definitions ============#
PRINT = printf
TOP_DIR  = $(shell pwd)

TARGETS = vnes    \
		  dbg-gui \
		  loadtest

TARGET_NAME      = vnes
TARGET_DIR       = $(TOP_DIR)
TARGET_SRC_DIR   = $(TARGET_DIR)/src
TARGET_INC_DIR   = $(TARGET_DIR)/include
TARGET_OBJ_DIR   = $(TARGET_DIR)/obj
TARGET_DIST_DIR  = $(TARGET_DIR)/dist
TARGET_SRC_FILES = cpu.c  			\
                   mem.c  			\
                   opcode.c			\
                   vnes.c			\
                   cart.c			\
                   ines-cart.c  	\
                   ppu.c        	\
                   render.c     	\
                   dbg-new.c		\
                   display.c


ifeq ($(MAKECMDGOALS), dbg-gui)
TARGET_NAME      = dbg-gui
TARGET_DIR       = $(TOP_DIR)
TARGET_SRC_DIR   = $(TARGET_DIR)/src
TARGET_INC_DIR   = $(TARGET_DIR)/include
TARGET_OBJ_DIR   = $(TARGET_DIR)/obj
TARGET_DIST_DIR  = $(TARGET_DIR)/dist
TARGET_SRC_FILES = dbg-pane.c  	\
                   dbg-gui.c  	
endif

ifeq ($(MAKECMDGOALS), loadtest)
TARGET_NAME      = loadtest
TARGET_DIR       = $(TOP_DIR)
TARGET_SRC_DIR   = $(TARGET_DIR)/src
TARGET_INC_DIR   = $(TARGET_DIR)/include
TARGET_OBJ_DIR   = $(TARGET_DIR)/obj
TARGET_DIST_DIR  = $(TARGET_DIR)/dist
TARGET_SRC_FILES = cart.c  	\
                   ines-cart.c  \
                   loadtest.c
endif

# Create ltarget dependency and object names
TARGET_SRC = $(addprefix $(TARGET_SRC_DIR)/, $(TARGET_SRC_FILES))
TARGET_OBJ = $(addprefix $(TARGET_OBJ_DIR)/, $(TARGET_SRC_FILES:.c=.o))

#============ Compiler Definitions ============#
CC       = gcc
CFLAGS   = -Wall
INCLUDES = $(addprefix -I, $(TARGET_INC_DIR))
LIBS     = -lncurses -lX11 -lGL -lGLU
DEFS     = -DUSE_INLINING

ifeq ($(DEBUG), true)
	CFLAGS += -g
endif

#============ Linker Definitions ===========#
LD       = ld
LDFLAGS  = -r

V=@

#============ Build Targets ============#

#.PHONY: bin
$(TARGETS): bin

# Build the binary
bin: bin-intro target
	@$(PRINT) "\t* Creating binary $(TARGET_NAME)\n"
	@mkdir -p $(TARGET_DIST_DIR)
	$(V)$(CC) $(CFLAGS) $(INCLUDES) $(TARGET_OBJ) $(LIBS) $(DEFS) -o $(TARGET_DIST_DIR)/$(TARGET_NAME)
	
# Build target module
target: target-intro $(TARGET_OBJ)
	@$(PRINT) "\t* Creating target module $(TARGET_NAME).o\n"


# Printouts #
target-intro:
	@$(PRINT) "\t* Compiling target object code.\n"
	@mkdir -p $(TARGET_OBJ_DIR)
	
bin-intro:
	@$(PRINT) "============[ Building $(TARGET_NAME) ]=============\n"


-include $(TARGET_OBJ:.o=.d)

$(TARGET_OBJ_DIR)/%.o: $(TARGET_SRC_DIR)/%.c
	@$(PRINT) "\t\t* Generating $*.d\n"
	$(V)$(CC) -MM $(CFLAGS) $(INCLUDES) $(DEFS) $(TARGET_SRC_DIR)/$*.c > $(TARGET_OBJ_DIR)/$*.d
	@$(PRINT) "\t\t* Compiling $*.o\n"
	$(V)$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFS) $(TARGET_SRC_DIR)/$*.c -o $(TARGET_OBJ_DIR)/$*.o
	
    #(Sort of) BLACK MAGIC -- Reformat dependency files for make purposes.
	@mv -f $(TARGET_OBJ_DIR)/$*.d $(TARGET_OBJ_DIR)/$*.d.tmp
	@sed -e 's|.*:|$(TARGET_OBJ_DIR)/$*.o:|' < $(TARGET_OBJ_DIR)/$*.d.tmp > $(TARGET_OBJ_DIR)/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(TARGET_OBJ_DIR)/$*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(TARGET_OBJ_DIR)/$*.d
	@rm -f $(TARGET_OBJ_DIR)/$*.d.tmp
    
clean:
	@rm -rf $(TARGET_DIST_DIR) $(TARGET_OBJ_DIR)
