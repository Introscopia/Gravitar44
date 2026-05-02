CC = gcc

OBJS = basics.c
OBJS += Chipmunk/chipmunk.c Chipmunk/cpArbiter.c Chipmunk/cpArray.c Chipmunk/cpBBTree.c Chipmunk/cpBody.c Chipmunk/cpCollision.c Chipmunk/cpConstraint.c Chipmunk/cpDampedRotarySpring.c Chipmunk/cpDampedSpring.c Chipmunk/cpGearJoint.c Chipmunk/cpGrooveJoint.c Chipmunk/cpHashSet.c Chipmunk/cpHastySpace.c Chipmunk/cpMarch.c Chipmunk/cpPinJoint.c Chipmunk/cpPivotJoint.c Chipmunk/cpPolyline.c Chipmunk/cpPolyShape.c Chipmunk/cpRatchetJoint.c Chipmunk/cpRobust.c Chipmunk/cpRotaryLimitJoint.c Chipmunk/cpShape.c Chipmunk/cpSimpleMotor.c Chipmunk/cpSlideJoint.c Chipmunk/cpSpace.c Chipmunk/cpSpaceComponent.c Chipmunk/cpSpaceDebug.c Chipmunk/cpSpaceHash.c Chipmunk/cpSpaceQuery.c Chipmunk/cpSpaceStep.c Chipmunk/cpSpatialIndex.c Chipmunk/cpSweep1D.c
OBJS += cvec.c vec2d.c primitives.c transform.c geometry.c svg.c input.c ciol.c game.c Grav44.c

# -w (suppresses all warnings)
# -Wl,-subsystem (windows gets rid of the console window)

COMPILER_FLAGS_RELEASE = -w -Wl,-subsystem,windows
COMPILER_FLAGS_QUICK = -w
COMPILER_FLAGS_DEBUG = -fmax-errors=3 -Waddress -Warray-bounds=1 -Wbool-compare -Wformat -Wimplicit -Wlogical-not-parentheses -Wmaybe-uninitialized -Wmemset-elt-size -Wmemset-transposed-args -Wmissing-braces -Wmultistatement-macros -Wopenmp-simd -Wparentheses -Wpointer-sign -Wrestrict -Wreturn-type -Wsequence-point -Wsizeof-pointer-div -Wsizeof-pointer-memaccess -Wstrict-aliasing -Wstrict-overflow=1 -Wtautological-compare -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wvolatile-register-var -Wcast-function-type -Wmissing-field-initializers -Wmissing-parameter-type -Woverride-init -Wsign-compare -Wtype-limits -Wshift-negative-value
COMPILER_FLAGS_MAX = -Wall -Wextra -Werror -O2 -std=c99 -pedantic

ifeq ($(OS),Windows_NT) # Windows_NT is the identifier for all versions of Windows
	DETECTED_OS := Windows
else
	DETECTED_OS := $(shell uname)
endif

ifeq ($(DETECTED_OS),Windows)
	INCLUDE_PATHS = -IC:/SDL/SDL3-3.4.0/x86_64-w64-mingw32/include/SDL3
	INCLUDE_PATHS += -IC:/SDL/SDL3-3.4.0/x86_64-w64-mingw32/include/
	#INCLUDE_PATHS += -IClipper2

	LIBRARY_PATHS = -LC:/SDL/SDL3-3.4.0/x86_64-w64-mingw32/lib
	#LIBRARY_PATHS += -LClipper2

	LINKER_FLAGS = -lSDL3 #-Llib -l:libClipper2.a
else
	INCLUDE_PATHS = -I/usr/include/SDL3
	LINKER_FLAGS = -lSDL3
endif

OBJ_NAME = Gravitar44

release : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_RELEASE) $(LINKER_FLAGS) -o $(OBJ_NAME)
quick : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_QUICK) $(LINKER_FLAGS) -o $(OBJ_NAME)
debug : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_DEBUG) $(LINKER_FLAGS) -g -o $(OBJ_NAME)
max : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS_MAX) $(LINKER_FLAGS) -o $(OBJ_NAME)