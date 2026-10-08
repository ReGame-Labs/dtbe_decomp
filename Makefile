.EXTRA_PREREQS := $(abspath $(lastword $(MAKEFILE_LIST)))

.DEFAULT_GOAL := all

# A rule whose check fails deletes what it made, so that make runs it again
.DELETE_ON_ERROR:

-include local.mk

# The version of the game to build. Only the Japanese release exists.
VERSION ?= jp
VERSIONS := jp
ifeq ($(filter $(VERSION),$(VERSIONS)),)
$(error unsupported VERSION $(VERSION); supported: $(VERSIONS))
endif
VERSION_UPPER := $(shell echo $(VERSION) | tr a-z A-Z)

EXE_NAME := SLPS_033.57
# The BIOS loads the header's t_size bytes after the header, and the file
# keeps .bss as zeros up to there
EXE_SIZE := 0x10D000

TOOLCHAIN ?= mipsel-linux-gnu-

CONFIG_DIR := config/$(VERSION)
BUILDDIR := build/$(VERSION)
ASM_DIR := asm/$(VERSION)
EXPECTEDDIR := expected/$(VERSION)
GENDIR := $(BUILDDIR)/generated

ELF := $(BUILDDIR)/$(EXE_NAME).elf
EXE := $(BUILDDIR)/$(EXE_NAME)
MAP := $(BUILDDIR)/$(EXE_NAME).map

CPP := $(TOOLCHAIN)cpp
AS := $(TOOLCHAIN)as
LD := $(TOOLCHAIN)ld
OBJCOPY := $(TOOLCHAIN)objcopy

PYTHON := python3
SPLAT := $(PYTHON) -m splat split

# the prebuilt compiler and tools that tools/dl_deps.sh downloads
BIN_DIR ?= bin

# GCC 2.95.2 at -O2 schedules the loads as the game's code has them
# (2.7.2 and 2.8.x don't)
GCC_VERSION ?= 2.95.2
CC1 ?= $(BIN_DIR)/gcc-$(GCC_VERSION)-psx/cc1

MASPSX := $(PYTHON) external/maspsx/maspsx.py
OBJDIFF ?= $(BIN_DIR)/objdiff-cli-linux-x86_64

INC := -Iinclude -Iexternal/psyq_headers/psyq_lib47/include

CPPFLAGS := $(INC) -undef -nostdinc -Wundef \
	    -D__GNUC__=2 -D__GNUC_MINOR__=$(word 2,$(subst ., ,$(GCC_VERSION))) -Dmips -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
	    -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C \
	    -DVERSION_$(VERSION_UPPER) -DASM_DIR='"$(ASM_DIR)"'
CC1FLAGS := -quiet -O2 -G0 -mips1 -mcpu=3000 -mgas -msoft-float \
	    -fgnu-linker -fsigned-char -Wall -Wno-unused
MASPSXFLAGS := --aspsx-version=2.86 -G0
ASFLAGS := -EL -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 $(INC)
LDFLAGS := -nostdlib --no-check-sections -Map $(MAP) \
	   -T $(GENDIR)/main.ld \
	   -T $(CONFIG_DIR)/undefined_syms.txt \
	   -T $(GENDIR)/undefined_syms_auto_main.txt \
	   -T $(GENDIR)/undefined_funcs_auto_main.txt

C_SRC := $(shell find src -name '*.c' 2> /dev/null)
# splat's full disassembly of each C file is objdiff's target, not part of
# the build
TARGET_ASM := $(C_SRC:src/%.c=$(ASM_DIR)/%.s)
ASM_SRC := $(filter-out $(TARGET_ASM),$(shell find $(ASM_DIR) -name '*.s' \
	   -not -path '*/nonmatchings/*' -not -path '*/matchings/*' 2> /dev/null))

C_OBJ := $(C_SRC:%.c=$(BUILDDIR)/%.c.o)
ASM_OBJ := $(ASM_SRC:%.s=$(BUILDDIR)/%.s.o)
TARGET_OBJ := $(TARGET_ASM:%.s=$(BUILDDIR)/%.s.o)
OBJ := $(C_OBJ) $(ASM_OBJ)

all: $(EXE)

# Only rerun splat when its own inputs change, never for Makefile edits
$(GENDIR)/main.ld: .EXTRA_PREREQS :=
$(GENDIR)/main.ld: $(CONFIG_DIR)/main.yaml $(CONFIG_DIR)/symbols.txt
	$(SPLAT) $< --disassemble-all --make-full-disasm-for-code

generate: $(GENDIR)/main.ld

regenerate: reset
	$(MAKE) generate

compare: $(EXE)
	@sha1sum -c $(CONFIG_DIR)/$(EXE_NAME).sha1

$(EXE): $(ELF)
	$(OBJCOPY) -O binary $< $@
	@truncate -s $$(($(EXE_SIZE))) $@

$(ELF): $(OBJ) $(GENDIR)/main.ld $(CONFIG_DIR)/undefined_syms.txt
	$(LD) $(LDFLAGS) -o $@

$(BUILDDIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -MMD -MP -MT $@ -MF $(@:.o=.d) $< -o $(@:.o=.i)
	$(CC1) $(CC1FLAGS) -o $(@:.o=.cc1.s) $(@:.o=.i)
	$(MASPSX) $(MASPSXFLAGS) < $(@:.o=.cc1.s) > $(@:.o=.s)
	$(AS) $(ASFLAGS) -o $@ $(@:.o=.s)

# gas aligns these sections to 16 bytes, psylink packed them to 4
$(BUILDDIR)/%.s.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<
	@$(OBJCOPY) --set-section-alignment .text=4 \
				--set-section-alignment .rodata=4 \
				--set-section-alignment .data=4 \
				--set-section-alignment .bss=4 $@

expected: $(TARGET_OBJ) $(C_OBJ)
	rm -rf $(EXPECTEDDIR)
	@mkdir -p $(EXPECTEDDIR)
	cp -r $(BUILDDIR)/$(ASM_DIR) $(EXPECTEDDIR)/asm

objdiff: expected
	$(PYTHON) tools/objdiff_generate.py $(VERSION)

report: objdiff
	$(OBJDIFF) report generate -o $(BUILDDIR)/report.json

clean:
	rm -rf $(BUILDDIR)

reset: clean
	rm -rf $(ASM_DIR) $(EXPECTEDDIR)

-include $(C_OBJ:.o=.d)

.PHONY: all generate regenerate compare expected objdiff report clean reset
