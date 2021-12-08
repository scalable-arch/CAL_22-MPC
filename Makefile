export TARGET = compressor

export CC = g++
export CFLAGS =-O3

export LDFLAGS = -lfmt -ljsoncpp

export OBJDIR = $(PWD)/obj
export SRCDIR = $(PWD)/src
export BINDIR = $(PWD)/bin

MAINDIR       = $(SRCDIR)
COMPRESSORDIR = $(SRCDIR)/compressor
LOADERDIR     = $(SRCDIR)/loader
SUBDIRS       = $(LOADERDIR) $(COMPRESSORDIR) $(MAINDIR)

all : $(SUBDIRS)

$(SUBDIRS) :
	@$(MAKE) -C $@ $(MAKECMDGOALS)

$(MAINDIR) : $(COMPRESSORDIR) $(LOADERDIR)


clean :
	rm -f $(BINDIR)/$(TARGET)
	rm -f $(OBJDIR)/*.o

.PHONY: all clean $(SUBDIRS)
