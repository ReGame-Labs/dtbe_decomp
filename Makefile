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
CC1PLUS ?= $(BIN_DIR)/gcc-$(GCC_VERSION)-psx/cc1plus

MASPSX := $(PYTHON) external/maspsx/maspsx.py
OBJDIFF ?= $(BIN_DIR)/objdiff-cli-linux-x86_64

INC := -Iinclude -Iexternal/psyq_headers/psyq_lib47/include

CPPFLAGS := $(INC) -undef -nostdinc -Wundef \
	    -D__GNUC__=2 -D__GNUC_MINOR__=$(word 2,$(subst ., ,$(GCC_VERSION))) -Dmips -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
	    -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C \
	    -DVERSION_$(VERSION_UPPER) -DASM_DIR='"$(ASM_DIR)"'
CC1FLAGS := -quiet -O2 -G8 -mips1 -mcpu=3000 -mgas -msoft-float \
	    -fgnu-linker -fsigned-char -Wall -Wno-unused
# the game is C++; its virtual tables have no type info in their first
# slot, and it has no exception tables
CC1PLUSFLAGS = $(CC1FLAGS) -fno-rtti -fno-exceptions
MASPSXFLAGS := --aspsx-version=2.86 -G8
ASFLAGS := -EL -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 $(INC)
LDFLAGS := -nostdlib --no-check-sections -Map $(MAP) \
	   -T $(GENDIR)/main.ld \
	   -T $(CONFIG_DIR)/undefined_syms.txt \
	   -T $(GENDIR)/undefined_syms_auto_main.txt \
	   -T $(GENDIR)/undefined_funcs_auto_main.txt

C_SRC := $(shell find src -name '*.c' 2> /dev/null)
CXX_SRC := $(shell find src -name '*.cpp' 2> /dev/null)
# splat's full disassembly of each C and C++ file is objdiff's target, not
# part of the build; src/engine holds the executable's (asm/<version>/main)
TARGET_ASM := $(C_SRC:src/engine/%.c=$(ASM_DIR)/main/%.s) $(CXX_SRC:src/engine/%.cpp=$(ASM_DIR)/main/%.s)
ASM_SRC := $(filter-out $(TARGET_ASM),$(shell find $(ASM_DIR) -name '*.s' \
	   -not -path '*/nonmatchings/*' -not -path '*/matchings/*' 2> /dev/null))

C_OBJ := $(C_SRC:%.c=$(BUILDDIR)/%.c.o) $(CXX_SRC:%.cpp=$(BUILDDIR)/%.cpp.o)
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

# The host's cpp has no C++ mode for this target, so C++ files are
# preprocessed as C with __cplusplus defined; cc1plus does the rest.
$(BUILDDIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CPP) -x c $(CPPFLAGS) -D__cplusplus -MMD -MP -MT $@ -MF $(@:.o=.d) $< -o $(@:.o=.ii)
	$(CC1PLUS) $(CC1PLUSFLAGS) -o $(@:.o=.cc1.s) $(@:.o=.ii)
	$(MASPSX) $(MASPSXFLAGS) < $(@:.o=.cc1.s) > $(@:.o=.s)
	$(AS) $(ASFLAGS) -o $@ $(@:.o=.s)

# the game was built with -G8 (its strings up to 8 bytes are in .sdata), but
# these two read their handle tables (TASK_HANDLES in .data, ENTITY_HANDLES
# in .bss), out of .sdata, as -G0 code does
G0_UNITS := task/task task/entity
G0_OBJ := $(G0_UNITS:%=$(BUILDDIR)/src/engine/%.c.o)
$(G0_OBJ): CC1FLAGS := $(subst -G8,-G0,$(CC1FLAGS))
$(G0_OBJ): MASPSXFLAGS := $(subst -G8,-G0,$(MASPSXFLAGS))

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

# The API documentation, $(DOCS_DIR)/html/index.html (docs/Doxyfile):
# doxygen reads the C's comments through tools/doxygen_filter.py, and the
# folders' descriptions from src/README.md (tools/doxygen_dirs.py). It needs
# doxygen, graphviz for the graphs, and the theme, the doxygen-awesome-css
# submodule, whose scripts go in doxygen's own header
# (docs/doxygen_head.html).
DOXYGEN ?= doxygen
DOCS_DIR := build/docs
DOXYGEN_THEME := external/doxygen-awesome-css
docs:
	@command -v $(DOXYGEN) > /dev/null || { echo "make docs needs doxygen, and graphviz for the graphs (apt install doxygen graphviz)" >&2; exit 1; }
	@test -f $(DOXYGEN_THEME)/doxygen-awesome.css || { echo "make docs needs the theme: git submodule update --init $(DOXYGEN_THEME)" >&2; exit 1; }
	@mkdir -p $(DOCS_DIR)
	$(PYTHON) tools/doxygen_dirs.py $(DOCS_DIR)/dirs.dox
	$(DOXYGEN) -w html $(DOCS_DIR)/default_header.html $(DOCS_DIR)/default_footer.html $(DOCS_DIR)/default.css docs/Doxyfile
	awk '/<\/head>/ { while ((getline line < "docs/doxygen_head.html") > 0) print line } 1' \
		$(DOCS_DIR)/default_header.html > $(DOCS_DIR)/header.html
	{ cat docs/Doxyfile; echo "HTML_HEADER = $(DOCS_DIR)/header.html"; \
		command -v dot > /dev/null || echo "HAVE_DOT = NO"; } | $(DOXYGEN) -
	@echo "$(DOCS_DIR)/html/index.html; doxygen's warnings: $(DOCS_DIR)/warnings.log"

docs-clean:
	rm -rf $(DOCS_DIR)

.PHONY: all generate regenerate compare expected objdiff report clean reset docs docs-clean
