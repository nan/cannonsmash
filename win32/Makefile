#
# Makefile for win32
#
############################################################################
.SUFFIXES: .exe .so .dll .a .lib .o .obj .cpp .cc .c .asm .s .y .h .l

.c.obj:
	$(CC) $(Cpre) $(CFLAGS) $(Cpost) $<
.cpp.obj:
	$(CXX) $(CXXpre) $(CXXFLAGS) $(CXXpost) $<
.cc.obj:
	$(CXX) $(CXXpre) $(CXXFLAGS) $(CXXpost) $<

############################################################################
CC = cl -nologo -W3
CXX = $(CC)

Cpost = -c -Tc
CXXpost = -c -Tp

#CFLAGS = -MD -Ox -G6 -vms $(INCDIRS)
#CFLAGS = -MD -Ox -G6 -vms -DHAVE_LIBZ $(INCDIRS)

CXXFLAGS=$(CFLAGS) -GX

O = obj
A = lib
SO = dll
EXE = .exe
AOUT = -Fe

############################################################################
INCDIRS	= -I.

OBJS = GetArgs.obj wsaerror.obj getopt.obj

TARGET = $(OBJS) wsainit.obj

############################################################################
all: $(TARGET)
	touch all
clean:
	rm -f $(TARGET) *.$(O) all
distclean:
	rm -f $(TARGET) *.$(O) *~ *.bak depend.nt all

depend:
	touch depend.nt
	makedepend -fdepend.nt $(INCDIRS) -D_WIN32 -DHAVE_LIBZ *.cpp
	cat depend.nt | sed -e "s/.:\/.*\/VC98[^ ]*include\/[^ ]*//g" \
	| grep -v ".*:[ ]*$$" > depend.new
	mv depend.new depend.nt

#-include depend.nt
############################################################################
# END
############################################################################
