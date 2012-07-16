# This makefile probably assumes
#    (*) this is being run in UNIX
#    (*) that some gmake specific features are available


# **********
# * CONFIG *
# **********-+
# | Platform |
# +----------+-------------------
# | evaluates to 'Darwin' on mac
# | evaluates to ??? on ???
# +------------------------------
PLATFORM := $(shell uname)

# +----------------+
# | Subdirectories |
# +----------------+
SUBDIRECTORIES := lbfgs libsvm dataset models


# ***********************
# * SOURCE DECLARATIONS *
# ***********************
# | Source Groups |
# +---------------+--------------
# | Logical groupings of sources
# | for later aggregation into
# | targets
# +------------------------------
LBFGS_SRCS   := lbfgs
LIBSVM_SRCS  := svm
DATASET_SRCS := getData features dataset
MODELS_SRCS  := logisticModel knnModel svmModel
BASEDIR_SRCS := \
    uid \
    backend
CTEST_SRCS   := cTest
# ALL_SRCS **MUST** have a list of all source files present!
ALL_SRCS     := \
    $(addprefix lbfgs/,$(LBFGS_SRCS)) \
    $(addprefix libsvm/,$(LIBSVM_SRCS)) \
    $(addprefix dataset/,$(DATASET_SRCS)) \
    $(addprefix models/,$(MODELS_SRCS)) \
    $(BASEDIR_SRCS) \
    $(CTEST_SRCS)


# +-----------------------------------+
# | HEADERS defines headers to export |
# +-----------------------------------+
BASEDIR_HEADERS   := 
HEADERS           := \
    $(BASEDIR_HEADERS)
# do not change:
#HEADER_TARGETS    := $(addprefix include/,$(HEADERS))

# +------------------------------------------------------------+
# | INTERFACES defines swig interface files for python library |
# +------------------------------------------------------------+
PY_INTERFACE      := backend
PY_WRAPPER        := python/wrappers/$(PY_INTERFACE)_wrap.cxx
PY_WRAPPER_OBJ    := python/obj/$(PY_INTERFACE)_wrap.o
PY_LIB            := python/_$(PY_INTERFACE).so

# +----------------+
# | Target sources |
# +----------------+------------------
# | Define exactly which source files
# | each build target requires
# +-----------------------------------
BACKEND_SRCS      := \
    $(addprefix lbfgs/,$(LBFGS_SRCS)) \
    $(addprefix libsvm/,$(LIBSVM_SRCS)) \
    $(addprefix dataset/,$(DATASET_SRCS)) \
    $(addprefix models/,$(MODELS_SRCS)) \
    $(BASEDIR_SRCS)

# +----------------+
# | Target Objects |
# +----------------+
BACKEND_OBJS      := $(addprefix obj/,$(addsuffix .o,$(BACKEND_SRCS)))
CTEST_OBJS        := $(addprefix obj/,$(addsuffix .o,$(CTEST_SRCS)))

PY_LIB_OBJS       := \
    $(addprefix python/obj/,$(addsuffix .o,$(BACKEND_SRCS))) \
    $(PY_WRAPPER_OBJ)

# +--------------------------+
# | Python Modules and Tools |
# +--------------------------+----------------
# | We maintain this separately from the C++
# | b/c the python build is much simpler
# +-------------------------------------------
PY_MOD_SRC        := imagePanel
PY_TOOL_SRC       := runDevelop showImage generateCurves
PY_MODS           := $(addprefix python/,$(addsuffix .py,$(PY_MOD_SRC)))
PY_TOOLS          := $(addprefix bin/,$(PY_TOOL_SRC))

# +------------------+
# | Dependency files |
# +------------------+------
# | List of all files
# | specifying dependencies
# +-------------------------
DEPENDS := $(addprefix depend/,$(addsuffix .d,$(ALL_SRCS)))
DEPENDS := $(DEPENDS) depend/$(PY_INTERFACE)_wrap.d


# ************************
# * LIBRARY DECLARATIONS *
# ************************
# | Individual Libraries |
# +----------------------+-----
# | Libraries provided
# | in the form of ld/cc flags
# +----------------------------
#GMPDIR            := /opt/local
# dynamic form
#GMPLIB            := -L$(GMPDIR)/lib -lgmpxx -lgmp
# static form
#GMPLIB            := $(GMPDIR)/lib/libgmpxx.a $(GMPDIR)/lib/libgmp.a
#GMPINC            := -I$(GMPDIR)/include

# note that Eigen is a headers-only library
EIGEN_DIR          := /Users/gilbo/code
EIGEN_INC          := -I$(EIGEN_DIR)

LIB_INC           := $(EIGEN_INC)
LIB_LINK          := 


# *********
# * TOOLS *
# *********--+
# | Programs |
# +----------+
CC  := gcc
CXX := g++
RM  := rm
CP  := cp
SWIG:= swig

# +--------------+
# | Option Flags |
# +--------------+
# Let the preprocessor know which platform we're on
PREPROC_FLAGS := -DSVM_KEEP_QUIET
ifeq ($(PLATFORM),Darwin)
  PREPROC_FLAGS  := $(PREPROC_FLAGS) -DMACOSX
endif

# flattened includes for all header files here
COMMON_INC := -Isrc/ $(addprefix -Isrc/,$(SUBDIRECTORIES))

PY_INC     := $(shell python-config --includes)
PY_LD      := $(shell python-config --ldflags)

INC        := $(COMMON_INC) $(LIB_INC)

CCFLAGS    := -Wall $(INC) $(PREPROC_FLAGS) -O3 -DNDEBUG
CXXFLAGS   := $(CCFLAGS)
CCDFLAGS   := -Wall $(INC) $(PREPROC_FLAGS) -ggdb
CXXDFLAGS  := $(CCDFLAGS)


LINK       := $(CXXFLAGS) $(LIB_LINK)
LINKD      := $(CXXDFLAGS) $(LIB_LINK)

CC_PY_FLAGS      := $(CCFLAGS) -fPIC $(PY_INC)
CXX_PY_FLAGS     := $(CXXFLAGS) -fPIC $(PY_INC)
PY_LINK          := $(LINK) -shared -fPIC $(PY_LD)

ifeq ($(PLATFORM),Darwin)
  CC_PY_FLAGS    := $(CC_PY_FLAGS) -arch i386
  CXX_PY_FLAGS   := $(CXX_PY_FLAGS) -arch i386
  PY_LINK        := $(PY_LINK) -arch i386
endif


# **********
# * SCRIPT *
# **********-----------+
# | Create Directories |
# +--------------------+
# (HACK: use this dummy variable to get access to shell commands)
SHELL_HACK := $(shell mkdir -p obj depend bin)
SHELL_HACK := $(shell mkdir -p $(addprefix depend/,$(SUBDIRECTORIES)))
SHELL_HACK := $(shell mkdir -p $(addprefix obj/,$(SUBDIRECTORIES)))

SHELL_HACK := $(shell mkdir -p python/obj python/wrappers)
SHELL_HACK := $(shell mkdir -p $(addprefix python/obj/,$(SUBDIRECTORIES)))


# *********
# * RULES *
# *********------+
# | Target Rules |
# +--------------+
all: python cbins

cbins: bin/cTest

python: $(PY_LIB) $(PY_MODS) $(PY_TOOLS)

$(PY_LIB): $(PY_LIB_OBJS)
	@echo "Linking python module $@"
	@$(CXX) -o $@ $(PY_LIB_OBJS) $(PY_LINK)

bin/cTest: $(BACKEND_OBJS) $(CTEST_OBJS)
	@echo "Linking $@"
	@$(CXX) -o $@ $(BACKEND_OBJS) $(CTEST_OBJS) $(LINK)

# +-------------------------+
# | Python Tool Build Rules |
# +-------------------------+
bin/%: python/src/bin-src/%.py
	@echo "Updating $@"
	@$(CP) $< $@ && chmod +x $@

# +------------------------------+
# | Specialized File Build Rules |
# +------------------------------+

$(PY_WRAPPER_OBJ): $(PY_WRAPPER)
	@echo "Compiling python wrapper $@"
	@$(CXX) $(CXX_PY_FLAGS) -o $@ -c $<

# Force dependencies to exist...
ALL_INTERFACE_FILES := $(shell find src -name *.i)

depend/$(PY_INTERFACE)_wrap.d: $(PY_WRAPPER)
	@$(CXX) $(CXX_PY_FLAGS) -MM $< | \
          sed -e 's@^\(.*\)\.o:@depend/\1.d python/obj/\1.o:@' > $@

$(PY_WRAPPER): src/$(PY_INTERFACE).i  $(ALL_INTERFACE_FILES)
	@echo "Generating Wrapper $@"
	@$(SWIG) -c++ -python $(INC) -o $@ -outdir python $<

# +------------------------------------+
# | Generic Source->Object Build Rules |
# +------------------------------------+
obj/%.o: src/%.cpp
	@echo "Compiling $@"
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/%.o: src/%.c
	@echo "Compiling $@"
	@$(CC) $(CCFLAGS) -o $@ -c $<

debug/%.o: src/%.cpp
	@echo "Compiling $@"
	@$(CXX) $(CXXDFLAGS) -o $@ -c $<

debug/%.o: src/%.c
	@echo "Compiling $@"
	@$(CC) $(CCDFLAGS) -o $@ -c $<

python/obj/%.o: src/%.c
	@echo "Compiling dynamic $@"
	@$(CC) $(CC_PY_FLAGS) -o $@ -c $<

python/obj/%.o: src/%.cpp
	@echo "Compiling dynamic $@"
	@$(CXX) $(CXX_PY_FLAGS) -o $@ -c $<

python/%.py: python/src/%.py
	@echo "Updating $@"
	@$(CP) $< $@

# dependency file build rules
depend/%.d: src/%.cpp
	@$(CXX) $(CXXFLAGS) -MM $< | \
          sed -e 's@^\(.*\)\.o:@depend/$*.d debug/$*.o obj/$*.o \
                  python/obj/$*.o:@' > $@

depend/%.d: src/%.c
	@$(CC) $(CCFLAGS) -MM $< | \
          sed -e 's@^\(.*\)\.o:@depend/$*.d debug/$*.o obj/$*.o \
                  python/obj/$*.o:@' > $@

# +---------------+
# | cleaning rule |
# +---------------+
clean:
	-@$(RM) -r depend bin
	-@$(RM) -r python/obj python/wrappers
	-@$(RM) python/*.pyc
	-@$(RM) python/*.py
	-@$(RM) python/*.so


# **********************
# * DEPENDENCY INCLUDE *
# **********************
-include $(DEPENDS)







