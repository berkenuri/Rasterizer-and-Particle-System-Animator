################################################################################
## OS
################################################################################
# OS = LINUX
OS = OSX

################################################################################
## GCC
################################################################################
CC = g++ -std=c++14 -w
OPTS = -O3
#OPTS = -g
FLAGS = -Wall -Werror
ifeq "$(OS)" "LINUX"
  DEFS = -DLINUX
else
  ifeq "$(OS)" "OSX"
  DEFS = -DOSX
endif
endif

################################################################################
## GL and QT
################################################################################
# Open gl
ifeq "$(OS)" "LINUX"
  GL_LIBS = -lglut -lGL
else
  ifeq "$(OS)" "OSX"
  GL_LIBS = -framework GLUT -lglfw -lglew -L/usr/local/lib -lsoil2-debug -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
endif
endif

################################################################################
## Rules
################################################################################
INCL = $(GL_INCL)
LIBS = $(GL_LIBS)

OBJS = \
			 CompileShaders.o \
			 objParser.o \
			 ray.o \
			 camera.o \
			 light.o \
			 material.o \
			 object.o \
			 scene.o \
			 random.o \
			 particlesystem.o \
       main.o

EXECUTABLE = spiderling

default: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS) $(OBJMOC)
	$(CC) $(OPTS) $(FLAGS) $(DEFS) $(OBJS) $(LIBS) -o $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) Dependencies $(OBJS)

.cpp.o:
	$(CC) $(OPTS) $(DEFS) -MMD $(INCL) -c $< -o $@
	cat $*.d >> Dependencies
	rm -f $*.d

-include Dependencies
