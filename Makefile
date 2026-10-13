.EXTRA_PREREQS := $(abspath $(lastword $(MAKEFILE_LIST)))

.DEFAULT_GOAL := all

# A rule whose check fails deletes what it made, so that make runs it again
.DELETE_ON_ERROR:

# a recipe's pipe fails when any of its commands does (iconv in the asm rule)
SHELL := /bin/bash
.SHELLFLAGS := -o pipefail -c

-include local.mk

# The version of the game to build: jp, the Japanese release, or us, the USA
# one. Each has its settings in mk/version/<version>.mk; the C and the
# assembly see VERSION_JP and VERSION_US (include/version.h).
VERSION ?= jp
VERSIONS := jp us
ifeq ($(filter $(VERSION),$(VERSIONS)),)
$(error unsupported VERSION $(VERSION); supported: $(VERSIONS))
endif
VERSION_UPPER := $(shell echo $(VERSION) | tr a-z A-Z)
include mk/version/$(VERSION).mk
export VERSION

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

# The code overlays, the files of A.VFS's bin/: the executable loads
# movie.bin, title.bin and game.bin at OVERLAY_MEMORY (include/overlay.h),
# and game.bin loads the battle's stage, st<NN>.bin, after itself. Each one
# has its own splat config ($(CONFIG_DIR)/<name>.yaml), symbols
# ($(CONFIG_DIR)/symbols_<name>.txt), sources (its config's src_path, and
# $(ASM_DIR)/<name>/) and output ($(BUILDDIR)/bin/<name>.bin), checked by
# $(CONFIG_DIR)/overlays.sha1.
STAGES := st00 st01 st02 st03 st04 st05 st06 st07 st08 st09
OVERLAYS := movie title game $(STAGES)
# a stage links against game.bin's symbols too
$(foreach s,$(STAGES),$(eval OVL_PARENT_$(s) := game))
BINARIES := main $(OVERLAYS)

# The overlays' configs read them from A.VFS unpacked (tools/unpack_vfs.py),
# inflated
VFS := disks/$(VERSION)/A.VFS
ASSETS_DIR := assets/$(VERSION)
VFS_UNPACKED := $(ASSETS_DIR)/.unpacked
$(VFS_UNPACKED): $(VFS) tools/unpack_vfs.py
	$(PYTHON) tools/unpack_vfs.py $(VFS) $(ASSETS_DIR)
	@touch $@

# Each binary's sources are under its config's src_path (src/engine for
# main), and splat's full disassembly of each of its C and C++ files, under
# $(ASM_DIR)/<binary>/, is objdiff's target, not part of the build
$(foreach b,$(BINARIES),$(eval SRC_DIR_$(b) := $(shell sed -n 's/^ *src_path: *//p' $(CONFIG_DIR)/$(b).yaml)))
# (a binary's files are those under its folder but not under another
# binary's folder inside it: the stages' are inside the battle's)
own_files = $(filter-out $(foreach c,$(filter-out $(1),$(BINARIES)),\
	$(if $(filter $(SRC_DIR_$(1))/%,$(SRC_DIR_$(c))),$(SRC_DIR_$(c))/%)),$(filter $(SRC_DIR_$(1))/%,$(2)))
$(foreach b,$(BINARIES),$(eval $(b)_C_SRC := $(call own_files,$(b),$(C_SRC))))
$(foreach b,$(BINARIES),$(eval $(b)_CXX_SRC := $(call own_files,$(b),$(CXX_SRC))))
$(foreach b,$(BINARIES),$(eval $(b)_TARGET_ASM := \
	$(patsubst $(SRC_DIR_$(b))/%.c,$(ASM_DIR)/$(b)/%.s,$($(b)_C_SRC)) \
	$(patsubst $(SRC_DIR_$(b))/%.cpp,$(ASM_DIR)/$(b)/%.s,$($(b)_CXX_SRC))))
TARGET_ASM := $(foreach b,$(BINARIES),$($(b)_TARGET_ASM))
$(foreach b,$(BINARIES),$(eval $(b)_ASM_SRC := $(filter-out $(TARGET_ASM),\
	$(shell find $(ASM_DIR)/$(b) -name '*.s' -not -path '*/nonmatchings/*' -not -path '*/matchings/*' 2> /dev/null))))
$(foreach b,$(BINARIES),$(eval $(b)_OBJ := $($(b)_C_SRC:%.c=$(BUILDDIR)/%.c.o) \
	$($(b)_CXX_SRC:%.cpp=$(BUILDDIR)/%.cpp.o) $($(b)_ASM_SRC:%.s=$(BUILDDIR)/%.s.o)))

C_OBJ := $(C_SRC:%.c=$(BUILDDIR)/%.c.o) $(CXX_SRC:%.cpp=$(BUILDDIR)/%.cpp.o)
TARGET_OBJ := $(TARGET_ASM:%.s=$(BUILDDIR)/%.s.o)
OBJ := $(main_OBJ)

# The executable's own symbols, for the overlays to link against: all but
# those it names in the overlays' memory, OVERLAY_VRAM-OVERLAY_END, where
# every overlay has its own
NM := $(TOOLCHAIN)nm
MAIN_SYMS := $(BUILDDIR)/main_syms.ld
lower_hex = $(shell printf '%08x' $$(($(1))))
$(MAIN_SYMS): $(ELF)
	$(NM) $< | awk '$$2 ~ /^[TDRBSG]$$/ && ($$1 < "$(call lower_hex,$(OVERLAY_VRAM))" || $$1 >= "$(call lower_hex,$(OVERLAY_END))") \
		{ printf "%s = 0x%s;\n", $$3, $$1 }' > $@

# An overlay loaded after another one, its parent (OVL_PARENT_<name>), also
# links against the parent's own symbols
$(BUILDDIR)/%_syms.ld: $(BUILDDIR)/bin/%.elf
	$(NM) $< | awk '$$2 ~ /^[TDRBSG]$$/ { printf "%s = 0x%s;\n", $$3, $$1 }' > $@

# An overlay links against the executable's symbols and its parent's, and
# takes the rest of what it uses from splat's symbol files
# (tools/link_imports.py)
define OVERLAY_template
$$(GENDIR)/$(1).ld: .EXTRA_PREREQS :=
$$(GENDIR)/$(1).ld: $$(CONFIG_DIR)/$(1).yaml $$(CONFIG_DIR)/symbols.txt $$(CONFIG_DIR)/symbols_$(1).txt \
		$$(if $$(OVL_PARENT_$(1)),$$(CONFIG_DIR)/symbols_$$(OVL_PARENT_$(1)).txt) $$(VFS_UNPACKED)
	$$(SPLAT) $$< --disassemble-all --make-full-disasm-for-code
	tools/sjis_includes.sh $$(ASM_DIR)/$(1)
	@touch $$@

$(1)_SYMS := $$(MAIN_SYMS) $$(if $$(OVL_PARENT_$(1)),$$(BUILDDIR)/$$(OVL_PARENT_$(1))_syms.ld)
$(1)_AUTO_SYMS := $$(BUILDDIR)/$(1)_auto_syms.ld
$$($(1)_AUTO_SYMS): $$($(1)_OBJ) $$(GENDIR)/$(1).ld $$($(1)_SYMS) tools/link_imports.py
	$$(PYTHON) tools/link_imports.py $$@ $$($(1)_OBJ) \
		--from $$(GENDIR)/undefined_syms_auto_$(1).txt $$(GENDIR)/undefined_funcs_auto_$(1).txt \
		--linked $$($(1)_SYMS)

$$(BUILDDIR)/bin/$(1).elf: $$($(1)_OBJ) $$(GENDIR)/$(1).ld $$($(1)_SYMS) $$($(1)_AUTO_SYMS)
	@mkdir -p $$(dir $$@)
	$$(LD) -nostdlib --no-check-sections -Map $$(BUILDDIR)/bin/$(1).map \
		-T $$(GENDIR)/$(1).ld $$(addprefix -T ,$$($(1)_SYMS) $$($(1)_AUTO_SYMS)) -o $$@

$$(BUILDDIR)/bin/$(1).bin: $$(BUILDDIR)/bin/$(1).elf
	$$(OBJCOPY) -O binary $$< $$@
endef
$(foreach o,$(OVERLAYS),$(eval $(call OVERLAY_template,$(o))))
OVL_BIN := $(OVERLAYS:%=$(BUILDDIR)/bin/%.bin)

all: $(EXE) $(OVL_BIN)

# Only rerun splat when its own inputs change, never for Makefile edits
$(GENDIR)/main.ld: .EXTRA_PREREQS :=
$(GENDIR)/main.ld: $(CONFIG_DIR)/main.yaml $(CONFIG_DIR)/symbols.txt $(CONFIG_DIR)/symbols_main.txt
	$(SPLAT) $< --disassemble-all --make-full-disasm-for-code

generate: $(GENDIR)/main.ld $(OVERLAYS:%=$(GENDIR)/%.ld)

regenerate: reset
	$(MAKE) generate

compare: $(EXE) $(OVL_BIN)
	@sha1sum -c $(CONFIG_DIR)/$(EXE_NAME).sha1 $(CONFIG_DIR)/overlays.sha1

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

# the executable was built with -G8 (its strings up to 8 bytes are in .sdata),
# but these two read their handle tables (TASK_HANDLES in .data,
# ENTITY_HANDLES in .bss) out of .sdata, as -G0 code does
G0_UNITS := src/engine/task/task.c src/engine/task/entity.c
# the overlays were built with -G0: none of them reads anything through $gp,
# and their short strings are in .rodata
G0_OBJ := $(G0_UNITS:%=$(BUILDDIR)/%.o) $(foreach b,$(OVERLAYS),$(filter-out %.s.o,$($(b)_OBJ)))
$(G0_OBJ): CC1FLAGS := $(subst -G8,-G0,$(CC1FLAGS))
$(G0_OBJ): MASPSXFLAGS := $(subst -G8,-G0,$(MASPSXFLAGS))

# two fighters of the battle overlay were built without CSE following jumps:
# their entity updates reload a count in the else block after `if (count > N)
# delete;`, which -O2 gives only with -fno-cse-follow-jumps; every other
# function of these units matches either way (fighter_05 to fighter_07, between
# them, match either way too and have nothing that tells)
NOFOLLOW_OBJ := $(foreach n,04 08,$(BUILDDIR)/src/battle/fighters/fighter_$(n).c.o)
$(NOFOLLOW_OBJ): CC1FLAGS += -fno-cse-follow-jumps

# splat writes the game's Shift JIS text as UTF-8 (string_encoding), which
# gas would keep: the assembly is assembled in Shift JIS. gas aligns these
# sections to 16 bytes, psylink packed them to 4.
$(BUILDDIR)/%.s.o: %.s
	@mkdir -p $(dir $@)
	iconv -f UTF-8 -t SHIFT-JIS $< | $(AS) $(ASFLAGS) -o $@
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
