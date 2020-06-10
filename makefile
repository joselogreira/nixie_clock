
###############################################################################
#	INPUT & OUTPUT FILES
###############################################################################

# Output directory
OUTDIR := output
SRCDIR := src

# Binary file name
PROGRAM = main

# Source code files
SRC = $(notdir $(wildcard ./$(SRCDIR)/*.c))
INC = -I ./$(SRCDIR)/

# Object files tracking based on $(SOURCES)
OBJ  := $(SRC:.c=.o)

###############################################################################
#	AVRDUDE PARAMETERS
###############################################################################

# enter the parameters for the avrdude isp tool
AVRDUDE 			= avrdude
AVRDUDE_PORT	   	= usb
AVRDUDE_PROGRAMMER 	= avrisp2
ifeq ($(AVRDUDE_PROGRAMMER),avrisp2)
	AVRDUDE_FREQ = -B2.0
endif

MCU 		= atmega324pb

# AVRDude
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -v $(AVRDUDE_FREQ) -F
AVRDUDE_WRITE_FLASH = -U flash:w:$(OUTDIR)/$(PROGRAM).hex
AVRDUDE_READ_FLASH = -U flash:r:$(OUTDIR)/$(PROGRAM).hex
AVRDUDE_READ_EEPROM = -U eeprom:r:$(OUTDIR)/$(PROGRAM).eep
AVRDUDE_ERASE_CHIP = -e
#AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(PROGRAM).eep
AVRDUDE_WRITE_FUSES = lock:w:$(LOCK):m -U efuse:w:$(EFUSE):m -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m
SAFEMODE = -u
# Fuses
HFUSE := 0x
LFUSE := 0x
EFUSE := 0x
LOCK  := 0x3F

###############################################################################
#	COMPILER/LINKER PARAMETERS
###############################################################################

CC          = avr-gcc
CC_SIZE		= avr-size
OBJCOPY     = avr-objcopy
OBJDUMP     = avr-objdump

VERBOSE 	= -v
DEBUGSYMB	= -g
OPTIMIZE   	= -O1
LDMAP 		= -Map,./$(OUTDIR)/$(PROGRAM).map

CFLAGS    	= $(DEBUGSYMB) -Wall $(OPTIMIZE) -mmcu=$(MCU) $(INC)
LDFLAGS   	= -Wl,$(LDMAP)

CSIZE_FLAGS_AVR	= -Cd --mcu=$(MCU)
CSIZE_FLAGS_SYS	= -Ad

# Intermix source code with disassembly. Test -d and -h flags to explore the output
OBJDUMP_FLAGS = -h -S
OBJCOPY_FLAGS_HEX 	= -j .text -j .data -O ihex
OBJCOPY_FLAGS_SREC 	= -j .text -j .data -O srec
OBJCOPY_FLAGS_BIN 	= -j .text -j .data -O binary

###############################################################################
#	MAKEFILE RULES
###############################################################################

.PHONY: build program program_fuses poke clean erase hello

$(OUTDIR):
	mkdir -p ./$(OUTDIR)

hello:
	@echo SRC = $(SRC)
	@echo INC = $(INC)
	@echo OBJ = $(OBJ)

build: $(OUTDIR) $(PROGRAM).hex
	@echo
	@echo ">> Build Finished =)"

# INTERFACING -----------------------------------------------------------------

program: $(OUTDIR)
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)	

program_fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(SAFEMODE) $(AVRDUDE_WRITE_FUSES)

read: $(OUTDIR)
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_READ_FLASH) $(AVRDUDE_READ_EEPROM)	

poke:
	$(AVRDUDE) $(AVRDUDE_FLAGS)

erase:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_ERASE_CHIP)	

clean:
	rm $(OUTDIR)/*
	rmdir $(OUTDIR)

# FILES -----------------------------------------------------------------------

%.elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o ./$(OUTDIR)/$@ $(addprefix ./$(OUTDIR)/,$^)
	@echo
	@echo " < SYSV SIZE REPORT >"
	@echo
	@$(CC_SIZE) $(CSIZE_FLAGS_SYS) ./$(OUTDIR)/$(PROGRAM).elf
	@$(CC_SIZE) $(CSIZE_FLAGS_AVR) ./$(OUTDIR)/$(PROGRAM).elf

%.hex: %.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS_HEX) ./$(OUTDIR)/$< ./$(OUTDIR)/$@

# Alternative assembly LISTING output format  
%.lst: %.elf
	$(OBJDUMP) $(OBJDUMP_FLAGS) ./$(OUTDIR)/$< > ./$(OUTDIR)/$@
# Alternative Motorola S-Record output format
%.srec: %.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS_SREC) ./$(OUTDIR)/$< ./$(OUTDIR)/$@
# Alternative Binary output format
%.bin: %.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS_BIN) ./$(OUTDIR)/$< ./$(OUTDIR)/$@

# UTILITY RULES ---------------------------------------------------------------

# Dependency files 
%.dep: ./$(SRCDIR)/%.c $(OUTDIR)
	@echo " >> Creating DEPENDENCY file"
	$(CC) $(CFLAGS) -M -o ./$(OUTDIR)/$@ $<

# Preprocessed files
%.i: ./$(SRCDIR)/%.c $(OUTDIR)
	@echo " >> Creating PREPROCESSED file"
	$(CC) $(CFLAGS) -E -o ./$(OUTDIR)/$@ $<

# Assembly files
%.asm: ./$(SRCDIR)/%.c $(OUTDIR)
	@echo " >> Creating ASSEMBLY file"
	$(CC) $(CFLAGS) -S -o ./$(OUTDIR)/$@ $<

# Object files
%.o: ./$(SRCDIR)/%.c $(OUTDIR)
	@echo " >> Creating OBJECT file"
	@echo $(notdir $@)
	$(CC) $(CFLAGS) -c -o ./$(OUTDIR)/$@ $<