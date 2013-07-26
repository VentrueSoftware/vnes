###############################################
# VConfig Configuration Parser Makefile       #
# Author: Kurt Sassenrath                     #
###############################################

#============ Project Definitions ============#

TOP_DIR  = $(shell pwd)

TARGET_NAME      = vnes
TARGET_DIR       = $(TOP_DIR)
TARGET_SRC_DIR   = $(TARGET_DIR)/src
TARGET_INC_DIR   = $(TARGET_DIR)/include
TARGET_OBJ_DIR   = $(TARGET_DIR)/obj
TARGET_DIST_DIR  = $(TARGET_DIR)/dist
TARGET_SRC_FILES = cpu.c  	\
                   mem.c  	\
                   opcode.c	\
                   vnes.c	\
                   dbg.c



# Create ltarget dependency and object names
TARGET_SRC = $(addprefix $(TARGET_SRC_DIR)/, $(TARGET_SRC_FILES))
TARGET_OBJ = $(addprefix $(TARGET_OBJ_DIR)/, $(TARGET_SRC_FILES:.c=.o))

#============ Compiler Definitions ============#
CC       = gcc
CFLAGS   = -Wall
INCLUDES = $(addprefix -I, $(TARGET_INC_DIR))
LIBS     = -lncurses
DEFS     = -DUSE_INLINING

#============ Linker Definitions ===========#
LD       = ld
LDFLAGS  = -r

V=@

#============ Build Targets ============#

.PHONY: bin

# Build the binary
bin: bin-intro target
	@echo -e "\t* Creating binary $(TARGET_NAME)"
	@mkdir -p $(TARGET_DIST_DIR)
	$(V)$(CC) $(CFLAGS) $(INCLUDES) $(TARGET_OBJ) $(LIBS) $(DEFS) -o $(TARGET_DIST_DIR)/$(TARGET_NAME)
	
# Build target module
target: target-intro $(TARGET_OBJ)
	@echo -e "\t* Creating target module $(TARGET_NAME).o"


# Printouts #
target-intro:
	@echo -e "\t* Compiling target object code."
	@mkdir -p $(TARGET_OBJ_DIR)
	
bin-intro:
	@echo -e "============[ Building $(TARGET_NAME) ]============="


-include $(TARGET_OBJ:.o=.d)

$(TARGET_OBJ_DIR)/%.o: $(TARGET_SRC_DIR)/%.c
	@echo -e "\t\t* Generating $*.d"
	$(V)$(CC) -MM $(CFLAGS) $(INCLUDES) $(DEFS) $(TARGET_SRC_DIR)/$*.c > $(TARGET_OBJ_DIR)/$*.d
	@echo -e "\t\t* Compiling $*.o"
	$(V)$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFS) $(TARGET_SRC_DIR)/$*.c -o $(TARGET_OBJ_DIR)/$*.o
	
    #(Sort of) BLACK MAGIC -- Reformat dependency files for make purposes.
	@mv -f $(TARGET_OBJ_DIR)/$*.d $(TARGET_OBJ_DIR)/$*.d.tmp
	@sed -e 's|.*:|$(TARGET_OBJ_DIR)/$*.o:|' < $(TARGET_OBJ_DIR)/$*.d.tmp > $(TARGET_OBJ_DIR)/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(TARGET_OBJ_DIR)/$*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(TARGET_OBJ_DIR)/$*.d
	@rm -f $(TARGET_OBJ_DIR)/$*.d.tmp
    
clean:
	@rm -rf $(TARGET_DIST_DIR) $(TARGET_OBJ_DIR)
