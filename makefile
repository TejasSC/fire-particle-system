LIB_PATHS= -L/usr/X11/lib -L/usr/X11R6/lib
HDR_PATHS= -I/usr/X11R6/include
OGL_LIBS= -lglut -lGL -lGLU -lGL
X_LIBS= -lX11 -lXext -lXmu -lXt -lXi
LIBS= $(OGL_LIBS)  $(X_LIBS) -lm
CFLAGS=$(HDR_PATHS)
CC=cc

PROG = ex1

# put your list of object modules here
# OBJECTS = \
#   part1.o\
#           part2.o\
#           another_bit.o
OBJECTS = ex1.o

##### TARGETS ######

$(PROG): $(OBJECTS)
 $(CC)  $(CFLAGS)  $(LIB_PATHS)  $(OBJECTS)  -o $(PROG) $(LIBS)

clean:
	-rm -f  *.o core a.out $(PROG) *~
