CXX = g++
#O3 for max optimization (go to 0 for debug)
CXXFLAGS = -Wall -O2 -Wextra -pedantic -fPIC -Wshadow -Wno-unused-local-typedefs -Wno-deprecated-declarations -std=c++11 -g
ifeq "$(GCCVERSION)" "1"
  CXXFLAGS += -Wno-error=misleading-indentation
endif

#Jet shapedir function
define JETSHAPEDIRERR
 JETSHAPEDIR is not set at all. Please set this environment variable to point to your build - this should be either
export JETSHAPEDIR=$(PWD)
or
source setEnv.sh
if you have made appropriate changes.
For more, see README for full setup recommendations
endef

define PYTHIA8DIRERR
 PYTHIA8DIR is not set at all. Please set this environment variable to point to your build - this should be set with 'source setEnv.sh' if you have made appropriate changes. For more, see README for full setup recommendations
endef

ifndef JETSHAPEDIR
$(error "$(JETSHAPEDIRERR)")	
endif

ifndef PYTHIA8DIR
$(error "$(PYTHIA8DIRERR)")	
endif

INCLUDE=-I$(JETSHAPEDIR) -I$(PYTHIA8DIR)/include
LIB=-L$(JETSHAPEDIR)/lib -lJetShapes -L$(PYTHIA8DIR)/lib -lpythia8

ROOT=`root-config --cflags --glibs`
FASTJET=`fastjet-config --cxxflags --libs --plugins`

MKDIR_BIN=mkdir -p $(JETSHAPEDIR)/bin
MKDIR_LIB=mkdir -p $(JETSHAPEDIR)/lib
MKDIR_OBJ=mkdir -p $(JETSHAPEDIR)/obj
MKDIR_OUTPUT=mkdir -p $(JETSHAPEDIR)/output
MKDIR_PDF=mkdir -p $(JETSHAPEDIR)/pdfDir

all: mkdirBin mkdirLib mkdirObj mkdirOutput mkdirPdf  obj/globalDebugHandler.o lib/libJetShapes.so bin/createPYTHIA.exe bin/createJetSpectraAndShapes.exe bin/plotJetSpectraAndShapes.exe

mkdirBin:
	$(MKDIR_BIN)

mkdirLib:
	$(MKDIR_LIB)

mkdirObj:
	$(MKDIR_OBJ)

mkdirOutput:
	$(MKDIR_OUTPUT)

mkdirPdf:
	$(MKDIR_PDF)

obj/globalDebugHandler.o: src/globalDebugHandler.C
	$(CXX) $(CXXFLAGS) -fPIC -c src/globalDebugHandler.C -o obj/globalDebugHandler.o $(ROOT) $(INCLUDE)

lib/libJetShapes.so:
	$(CXX) $(CXXFLAGS) -fPIC -shared -o lib/libJetShapes.so obj/globalDebugHandler.o $(ROOT) $(INCLUDE)

bin/createPYTHIA.exe: src/createPYTHIA.C
	$(CXX) $(CXXFLAGS) src/createPYTHIA.C -o bin/createPYTHIA.exe $(ROOT) $(FASTJET) $(INCLUDE) $(LIB)

bin/createJetSpectraAndShapes.exe: src/createJetSpectraAndShapes.C
	$(CXX) $(CXXFLAGS) src/createJetSpectraAndShapes.C -o bin/createJetSpectraAndShapes.exe $(ROOT) $(INCLUDE) $(LIB)

bin/plotJetSpectraAndShapes.exe: src/plotJetSpectraAndShapes.C
	$(CXX) $(CXXFLAGS) src/plotJetSpectraAndShapes.C -o bin/plotJetSpectraAndShapes.exe $(ROOT) $(INCLUDE) $(LIB)

clean:
	rm -f ./*~
	rm -f ./#*#
	rm -f bash/*~
	rm -f bash/#*#
	rm -f bin/*.exe
	rm -rf bin
	rm -f include/*~
	rm -f include/#*#
	rm -f input/*~
	rm -f input/#*#
	rm -f lib/*.so
	rm -rf lib
	rm -f obj/*.o
	rm -rf obj
	rm -f src/*~
	rm -f src/#*#
