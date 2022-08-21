# WinARM template makefile 
# by Martin Thomas, Kaiserslautern, Germany 
# <eversmith(at)heizung-thomas(dot)de>
#
# Released to the Public Domain
# Please read the make user manual!
#
# The user-configuration is based on the WinAVR makefile-template
# written by Eric B. Weddington, Jörg Wunsch, et al. but internal
# handling used here is very different.
# This makefile can also be used with the GNU tools included in
# Yagarto, GNUARM or the codesourcery packages. It should work
# on Unix/Linux-Systems too. Just a rather up-to-date GNU make is
# needed.
#
# MD:
# - removed unused stuff to simplify maintnance 

TCHAIN_PREFIX = arm-none-eabi-


M    ?= mangopi

ifeq ($(M), mangopi)
BOARD = mangopi
TARGET = mangopi
CHIP     = AW_T113S2
F_XTAL   = 25000000
RAMSIZE  = 0x10000
LINKERSCRIPTPATH = .
endif


# *** This example only supports "FLASH_RUN" ***
# RUN_MODE is passed as define and used for the linker-script filename,
# the user has to implement the necessary operations for 
# the used mode(s) (i.e. no copy of .data, remapping)
# Create FLASH-Image
RUN_MODE=FLASH_RUN
# Create RAM-Image
#RUN_MODE=RAM_RUN

# Exception-vectors placement option is just passed as define,
# the user has to implement the necessary operations (i.e. remapping)
# Exception vectors in FLASH:
VECTOR_TABLE_LOCATION=VECT_TAB_FLASH
#VECTOR_TABLE_LOCATION=VECT_TAB_RAM

# Directory for output files (lst, obj, dep, elf, sym, map, hex, bin etc.)
OUTDIR = $(RUN_MODE)/$(TARGET)

# Target file name (without extension).
# TARGET = $(OUT_FILE)

# Pathes to libraries
APPLIBDIR = ../lib
TINYUSB = ../lib/tinyusb-ohci


BSP = ../bsp/$(BOARD)

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

## Application:
SRC = main.c syscalls.c
SRC += led.c
SRC += hid_app.c usb_task.c
SRC += sdfs.c diskio.c

#SRC += ../common/arm/arm_timer.c
SRC += ../common/arm/irq.c
SRC += ../common/arm/mmu.c

SRC += ../common/aw/ccu.c
SRC += ../common/aw/gpio.c
SRC += ../common/aw/tcon_lcd.c
SRC += ../common/aw/de.c
SRC += ../common/aw/uart.c
SRC += ../common/aw/dmac.c
SRC += ../common/aw/smhc.c
SRC += ../common/aw/timer.c

SRC += ../common/fifo.c
SRC += ../common/gr.c
SRC += ../common/random.c
SRC += ../common/memmgr.c

SRC += $(APPLIBDIR)/ff/ff.c
SRC += $(APPLIBDIR)/ff/ffsystem.c

SRC += $(APPLIBDIR)/tinyprintf/tinyprintf.c
SRC += $(APPLIBDIR)/tlsf/tlsf.c

SRC += $(APPLIBDIR)/FreeRTOS-Kernel-main/croutine.c
SRC += $(APPLIBDIR)/FreeRTOS-Kernel-main/event_groups.c
SRC += $(APPLIBDIR)/FreeRTOS-Kernel-main/list.c
SRC += $(APPLIBDIR)/FreeRTOS-Kernel-main/queue.c
SRC += $(APPLIBDIR)/FreeRTOS-Kernel-main/stream_buffer.c
SRC += $(APPLIBDIR)/FreeRTOS-Kernel-main/tasks.c
SRC += $(APPLIBDIR)/FreeRTOS-Kernel-main/timers.c
SRC += $(APPLIBDIR)/FreeRTOS-Kernel-main/portable/MemMang/heap_3.c
SRC += $(APPLIBDIR)/FreeRTOS-Kernel-main/portable/GCC/ARM_CA9/port.c

SRC += $(TINYUSB)/src/tusb.c
SRC += $(TINYUSB)/src/class/cdc/cdc_host.c
SRC += $(TINYUSB)/src/class/hid/hid_host.c
SRC += $(TINYUSB)/src/class/msc/msc_host.c
SRC += $(TINYUSB)/src/host/hub.c
SRC += $(TINYUSB)/src/host/usbh.c
SRC += $(TINYUSB)/src/portable/ohci/ohci.c
#SRC += $(TINYUSB)/src/portable/ehci/ehci.c
#SRC += $(TINYUSB)/src/portable/nxp/lpc17_40/hcd_lpc17_40.c


# List C source files here which must be compiled in ARM-Mode (no -mthumb).
# use file-extension c for "c-only"-files
## just for testing, timer.c could be compiled in thumb-mode too
SRCARM = 

# List C++ source files here.
# use file-extension .cpp for C++-files (not .C)
CPPSRC = 

# List C++ source files here which must be compiled in ARM-Mode.
# use file-extension .cpp for C++-files (not .C)
CPPSRCARM = 

# List Assembler source files here.
# Make them always end in a capital .S. Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = 

# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM = start.S
ASRCARM += asm/memcpy.S asm/memcmp.S asm/memset.S asm/memmove.S
ASRCARM += $(APPLIBDIR)/FreeRTOS-Kernel-main/portable/GCC/ARM_CA9/portASM.S

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS  = ../common ../common/arm ../common/aw
EXTRAINCDIRS += $(APPLIBDIR)/aw
EXTRAINCDIRS += $(APPLIBDIR)/CMSIS_5/CMSIS/Core/Include
EXTRAINCDIRS += $(APPLIBDIR)/CMSIS_5/CMSIS/Core_A/Include
EXTRAINCDIRS += $(APPLIBDIR)/tinyprintf/
EXTRAINCDIRS += $(APPLIBDIR)/tlsf/
EXTRAINCDIRS += $(APPLIBDIR)/ff/
EXTRAINCDIRS += $(APPLIBDIR)/FreeRTOS-Kernel-main/include
EXTRAINCDIRS += $(APPLIBDIR)/FreeRTOS-Kernel-main/portable/GCC/ARM_CA9/

EXTRAINCDIRS += $(TINYUSB)/src

EXTRAINCDIRS += ../common
EXTRAINCDIRS += ../bsp

# List any extra directories to look for library files here.
# Also add directories where the linker should search for
# includes from linker-script to the list
#     Each directory must be seperated by a space.
EXTRA_LIBDIRS =

# Extra libraries
#    Each library-name must be seperated by a space.
#    i.e. to link with libxyz.a, libabc.a and libefsl.a: 
#    EXTRA_LIBS = xyz abc efsl
# for newlib-lpc (file: libnewlibc-lpc.a):
#    EXTRA_LIBS = newlib-lpc
EXTRA_LIBS =


# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = s
#OPT = 2
#OPT = 3
#OPT = 0

# Output format. (can be ihex or binary or both)
#  binary to create a load-image in raw-binary format i.e. for SAM-BA, 
#  ihex to create a load-image in Intel hex format i.e. for lpc21isp
#LOADFORMAT = ihex
#LOADFORMAT = binary
#LOADFORMAT = both

# Debugging format.
DEBUG_FORMAT = dwarf-2

# Place project-specific -D (define) and/or 
# -U options for C here.
CDEFS =

# Place project-specific -D and/or -U options for 
# Assembler with preprocessor here.
ADEFS = -D__ASSEMBLY__

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

#-----

ifdef VECTOR_TABLE_LOCATION
CDEFS += -D$(VECTOR_TABLE_LOCATION)
ADEFS += -D$(VECTOR_TABLE_LOCATION)
endif


CDEFS += -D$(RUN_MODE) -D$(CHIP)
CDEFS += -DSW_VER=$$(cat ../.version)
CDEFS += -DSW_BUILD=$$(cat ../.build)
ADEFS += -D$(RUN_MODE) -D$(CHIP)

THUMB    = #-mthumb


# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS =  -g$(DEBUG_FORMAT)
#CFLAGS += -O$(OPT)
CFLAGS +=  $(THUMB_IW) -march=armv7-a -mtune=cortex-a7 -mfpu=vfpv4 -mfloat-abi=hard -marm -mno-thumb-interwork -mno-unaligned-access
CFLAGS += $(CDEFS)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.
# when using ".ramfunc"s without longcall:
##CFLAGS += -mlong-calls
# -mapcs-frame is important if gcc's interrupt attributes are used
# (at least from my eabi tests), not needed if assembler-wrapper is used 
##CFLAGS += -mapcs-frame 
##CFLAGS += -fomit-frame-pointer
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall -Wextra -Wno-redundant-decls -Wno-nested-externs
CFLAGS += -Wimplicit -Wcast-align -Wpointer-arith
CFLAGS += -Wshadow
#CFLAGS += -Wcast-qual
CFLAGS += -Wcast-align
#CFLAGS += -pedantic
CFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
# Compiler flags to generate dependency files:
CFLAGS += -MD -MP -MF $(OUTDIR)/dep/$(@F).d

# flags only for C
#CONLYFLAGS += -Wnested-externs 
CONLYFLAGS += $(CSTANDARD)

# flags only for C++ (arm-elf-g++)
CPPFLAGS = -fno-rtti -fno-exceptions
CPPFLAGS = 

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS  = $(THUMB_IW) -I. -x assembler-with-cpp
ASFLAGS += $(ADEFS)
ASFLAGS += -march=armv7-a -mtune=cortex-a7 -mfpu=vfpv4 -mfloat-abi=hard -marm -mno-thumb-interwork -mno-unaligned-access
#ASFLASG += -ahls -mapcs-32
ASFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
ASFLAGS += -Wa,-g$(DEBUG)
ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

MATH_LIB = -lm

# Link with the GNU C++ stdlib.
#CPLUSPLUS_LIB = -lstdc++

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -nostartfiles -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))
LDFLAGS += $(MATH_LIB)
LDFLAGS += $(CPLUSPLUS_LIB)
LDFLAGS += #-lc -lgcc 
#LDFLAGS += -Wl,-wrap,_malloc_r -Wl,-wrap,_free_r
#LDFLAGS += --specs=nano.specs

# Set linker-script name depending on selected run-mode and submodel name
LDFLAGS +=-T$(LINKERSCRIPTPATH)/$(CHIP).ld


# Define programs and commands.
CC      = $(TCHAIN_PREFIX)gcc
CPP     = $(TCHAIN_PREFIX)g++
AR      = $(TCHAIN_PREFIX)ar
OBJCOPY = $(TCHAIN_PREFIX)objcopy
OBJDUMP = $(TCHAIN_PREFIX)objdump
SIZE    = $(TCHAIN_PREFIX)size
NM      = $(TCHAIN_PREFIX)nm

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = "-------- begin (mode: $(RUN_MODE)) --------"
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before: 
MSG_SIZE_AFTER = Size after build:
MSG_LOAD_FILE = Creating load file:
MSG_EXTENDED_LISTING = Creating Extended Listing/Disassembly:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = "**** Linking :"
MSG_COMPILING = "**** Compiling C :"
MSG_COMPILING_ARM = "**** Compiling C (ARM-only):"
MSG_COMPILINGCPP = "Compiling C++ :"
MSG_COMPILINGCPP_ARM = "Compiling C++ (ARM-only):"
MSG_ASSEMBLING = "**** Assembling:"
MSG_ASSEMBLING_ARM = "****Assembling (ARM-only):"
MSG_CLEANING = Cleaning project:
MSG_FORMATERROR = Can not handle output-format
MSG_LPC21_RESETREMINDER = You may have to bring the target in bootloader-mode now.
MSG_ASMFROMC = "Creating asm-File from C-Source:"
MSG_ASMFROMC_ARM = "Creating asm-File from C-Source (ARM-only):"

# List of all source files.
ALLSRC     = $(ASRCARM) $(ASRC) $(SRCARM) $(SRC) $(CPPSRCARM) $(CPPSRC)
# List of all source files without directory and file-extension.
ALLSRCBASE = $(notdir $(basename $(ALLSRC)))

# Define all object files.
ALLOBJ     = $(addprefix $(OUTDIR)/, $(addsuffix .o, $(ALLSRCBASE)))

# Define all listing files (used for make clean).
LSTFILES   = $(addprefix $(OUTDIR)/, $(addsuffix .lst, $(ALLSRCBASE)))
# Define all depedency-files (used for make clean).
DEPFILES   = $(addprefix $(OUTDIR)/dep/, $(addsuffix .o.d, $(ALLSRCBASE)))

# Default target.
all: begin gccversion build sizeafter finished end

elf: $(OUTDIR)/$(TARGET).elf
lss: $(OUTDIR)/$(TARGET).lss 
sym: $(OUTDIR)/$(TARGET).sym
hex: $(OUTDIR)/$(TARGET).hex
bin: $(OUTDIR)/$(TARGET).bin

ifeq ($(LOADFORMAT),ihex)
build: elf hex lss sym
else 
ifeq ($(LOADFORMAT),binary)
build: elf bin lss sym
else 
ifeq ($(LOADFORMAT),both)
build: elf hex bin lss sym
else 
build: elf lss sym
endif
endif
endif


# Eye candy.
begin:
##	@echo
	@echo $(MSG_BEGIN)

finished:
##	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
##	@echo

# Display sizes of sections.
ELFSIZE = $(SIZE) -A -x $(OUTDIR)/$(TARGET).elf

sizeafter:
	@echo $(MSG_SIZE_AFTER)
	$(ELFSIZE)
	
# Display compiler version information.
gccversion : 
	@$(CC) --version
#	@echo $(ALLOBJ)

flash_dump:
	@echo "Dumping flash"
	openocd -f openocd.cfg -c "init" -c "halt" -c "flash probe 0" -c "dump_image flash.img 0x08000000 0x20000" -c "reset run" -c "exit"

flash_load: bin
#	@echo "Programming with OPENOCD"
#	openocd -f openocd.cfg -c "init" -c "halt" -c "stm32x mass_erase 0" -c "flash write_image $(OUTDIR)/$(TARGET).elf elf" -c "soft_reset_halt" -c "reset run" -c "exit"
	../dfuse-dfu-util/src/dfu-util --dfuse 0x8002000 -D $(OUTDIR)/stm32_ises_blue_usb.bin -d 0x0483:0xdf11 -R



# Create final output file (.hex) from ELF output file.
%.hex: %.elf
##	@echo
	@echo $(MSG_LOAD_FILE) $@
	$(OBJCOPY) -O ihex $< $@
	
# Create final output file (.bin) from ELF output file.
%.bin: %.elf
##	@echo
	@echo $(MSG_LOAD_FILE) $@
	$(OBJCOPY) -O binary $< $@

# Create extended listing file/disassambly from ELF output file.
# using objdump testing: option -C
%.lss: %.elf
##	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S -C -r $< > $@
#	$(OBJDUMP) -x -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
##	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(ALLOBJ)
%.elf:  $(ALLOBJ)
	@echo
	@echo $(MSG_LINKING) $@ && $(CC) $(THUMB) $(CFLAGS) $(ALLOBJ) --output $@ $(LDFLAGS)
# use $(CC) for C-only projects or $(CPP) for C++-projects:
#	$(CPP) $(THUMB) $(CFLAGS) $(ALLOBJ) --output $@ $(LDFLAGS)


# Assemble: create object files from assembler source files.
define ASSEMBLE_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_ASSEMBLING) $$< "->" $$@ && $(CC) -c $(THUMB) $$(ASFLAGS) $$< -o $$@ 
endef
$(foreach src, $(ASRC), $(eval $(call ASSEMBLE_TEMPLATE, $(src)))) 

# Assemble: create object files from assembler source files. ARM-only
define ASSEMBLE_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_ASSEMBLING_ARM) $$< "->" $$@ && $(CC) -c $$(ASFLAGS) $$< -o $$@ 
endef
$(foreach src, $(ASRCARM), $(eval $(call ASSEMBLE_ARM_TEMPLATE, $(src)))) 


# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
##	echo $(CC) -c $(THUMB) $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@ 
	@echo $(MSG_COMPILING) $$< "->" $$@ && 	$(CC) -c $(THUMB) $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@ 
endef
$(foreach src, $(SRC), $(eval $(call COMPILE_C_TEMPLATE, $(src)))) 

# Compile: create object files from C source files. ARM-only
define COMPILE_C_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_COMPILING_ARM) $$< "->" $$@ && $(CC) -c $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@ 
endef
$(foreach src, $(SRCARM), $(eval $(call COMPILE_C_ARM_TEMPLATE, $(src)))) 


# Compile: create object files from C++ source files.
define COMPILE_CPP_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_COMPILINGCPP) $$< "->" $$@ && $(CC) -c $(THUMB) $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@ 
endef
$(foreach src, $(CPPSRC), $(eval $(call COMPILE_CPP_TEMPLATE, $(src)))) 

# Compile: create object files from C++ source files. ARM-only
define COMPILE_CPP_ARM_TEMPLATE
$(OUTDIR)/$(notdir $(basename $(1))).o : $(1)
##	@echo
	@echo $(MSG_COMPILINGCPP_ARM) $$< "->" $$@ && $(CC) -c $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@ 
endef
$(foreach src, $(CPPSRCARM), $(eval $(call COMPILE_CPP_ARM_TEMPLATE, $(src)))) 


# Compile: create assembler files from C source files. ARM/Thumb
$(SRC:.c=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC) $< to $@
	$(CC) $(THUMB) -S $(CFLAGS) $(CONLYFLAGS) $< -o $@

# Compile: create assembler files from C source files. ARM only
$(SRCARM:.c=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC_ARM) $< to $@
	$(CC) -S $(CFLAGS) $(CONLYFLAGS) $< -o $@

# Target: clean project.
clean: begin clean_list finished end

clean_list :
##	@echo
	@echo $(MSG_CLEANING)
	rm -f $(OUTDIR)/$(TARGET).map
	rm -f $(OUTDIR)/$(TARGET).elf
	rm -f $(OUTDIR)/$(TARGET).hex
	rm -f $(OUTDIR)/$(TARGET).bin
	rm -f $(OUTDIR)/$(TARGET).sym
	rm -f $(OUTDIR)/$(TARGET).lss
	rm -f $(ALLOBJ)
	rm -f $(LSTFILES)
	rm -f $(DEPFILES)
	rm -f $(SRC:.c=.s)
	rm -f $(SRCARM:.c=.s)
	rm -f $(CPPSRC:.cpp=.s)
	rm -f $(CPPSRCARM:.cpp=.s)


# Create output files directory
# all known MS Windows OS define the ComSpec environment variable
$(shell mkdir -p $(OUTDIR) 2>/dev/null)

# Include the dependency files.
-include $(shell mkdir -p $(OUTDIR) 2>/dev/null) $(shell mkdir -p $(OUTDIR)/dep 2>/dev/null) $(wildcard $(OUTDIR)/dep/*)


# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex bin lss sym clean clean_list program

