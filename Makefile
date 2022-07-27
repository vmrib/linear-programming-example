CC              = gcc
CFLAGS          = -Wall -std=gnu17 -I$(INCLUDEDIR) -I$(LPSOLVEDIR)
CFLAGS_DEBUG    = $(CFLAGS) -g -DDEBUG
CFLAGS_RESULT	= $(CFLAGS) -DRESULT
LDFLAGS			= -L$(LIBDIR) -llpsolve55 -lm -ldl

BUILDDIR        = build
INCLUDEDIR      = include
LIBDIR          = lib
SOURCEDIR       = source
TESTSDIR        = tests

LPSOLVEDIR			= $(LIBDIR)/source/lp_solve_5.5
LPSOLVEINSTALLDIR	= $(LPSOLVEDIR)/lpsolve55
LPSOLVEINSTALL		= ccc

SOURCES         := $(shell find $(SOURCEDIR) -name "*.c")
OBJECTS         := $(patsubst %.c, $(BUILDDIR)/%.o, $(notdir $(SOURCES)))
LIBS			:= $(addprefix $(LIBDIR)/, liblpsolve55.a)
TARGET          := despacho

.PHONY: all result run clean debug purge

all: $(TARGET)

result: CFLAGS := $(CFLAGS_RESULT)
result: $(TARGET)

debug: CFLAGS := $(CFLAGS_DEBUG)
debug: $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

purge:
	rm -rf $(BUILDDIR) $(LIBS) $(TARGET)

$(TARGET): $(OBJECTS) | $(LIBS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OBJECTS): $(SOURCES) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(LIBS):
	@echo Building lpsolve...
	@pushd $(LPSOLVEINSTALLDIR)/ > /dev/null 2>&1; \
	chmod +x $(LPSOLVEINSTALL) > /dev/null 2>&1; \
	./$(LPSOLVEINSTALL) > /dev/null 2>&1; \
	popd > /dev/null 2>&1; \
	cp $(LPSOLVEINSTALLDIR)/bin/ux64/liblpsolve55.a $(LIBDIR) > /dev/null 2>&1
	
