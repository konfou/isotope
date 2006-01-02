################
# PATHS AND NAMES
################
TARGET_PATH = ../../lib
TARGET_NAME_OPT = GClasses.a
TARGET_NAME_DBG = GClassesDbg.a
OBJ_PATH = ../../obj/obj
GCLASSES_PATH = .
UNAME = $(shell uname -s)
CFLAGS = $(shell sdl-config --cflags)
ifeq ($(UNAME),Darwin)
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG -DDARWIN -no-cpp-precomp
OPT_CFLAGS = $(CFLAGS) -O3 -DDARWIN -no-cpp-precomp
else
#DBG_CFLAGS = $(CFLAGS) -fpermissive -g -D_DEBUG -Wall
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG -Wall
OPT_CFLAGS = $(CFLAGS) -O3 -Wall
endif

################
# GClasses Sources
################

GCLASSES_SOURCES = $(GCLASSES_PATH)/GArff.cpp\
                   $(GCLASSES_PATH)/GArray.cpp\
                   $(GCLASSES_PATH)/GAVLTree.cpp\
                   $(GCLASSES_PATH)/GBezier.cpp\
                   $(GCLASSES_PATH)/GBigNumber.cpp\
                   $(GCLASSES_PATH)/GBillboardCamera.cpp\
                   $(GCLASSES_PATH)/GBits.cpp\
                   $(GCLASSES_PATH)/GBitTable.cpp\
                   $(GCLASSES_PATH)/GBlindGreedy.cpp\
                   $(GCLASSES_PATH)/GCautiousGreedy.cpp\
                   $(GCLASSES_PATH)/GCompress.cpp\
                   $(GCLASSES_PATH)/GConfSearch.cpp\
                   $(GCLASSES_PATH)/GCppParser.cpp\
                   $(GCLASSES_PATH)/GCrypto.cpp\
                   $(GCLASSES_PATH)/GDaftLearner.cpp\
                   $(GCLASSES_PATH)/GDataBase.cpp\
                   $(GCLASSES_PATH)/GDate.cpp\
                   $(GCLASSES_PATH)/GDecisionTree.cpp\
                   $(GCLASSES_PATH)/GDiff.cpp\
                   $(GCLASSES_PATH)/GDirList.cpp\
                   $(GCLASSES_PATH)/GEZSocket.cpp\
                   $(GCLASSES_PATH)/GFile.cpp\
                   $(GCLASSES_PATH)/GFourier.cpp\
                   $(GCLASSES_PATH)/GFuzzyLogic.cpp\
                   $(GCLASSES_PATH)/GGenetic.cpp\
                   $(GCLASSES_PATH)/GGrammar.cpp\
                   $(GCLASSES_PATH)/GHardFont.cpp\
                   $(GCLASSES_PATH)/GHashTable.cpp\
                   $(GCLASSES_PATH)/GHeap.cpp\
                   $(GCLASSES_PATH)/GHtml.cpp\
                   $(GCLASSES_PATH)/GHttp.cpp\
                   $(GCLASSES_PATH)/GImage.cpp\
                   $(GCLASSES_PATH)/GKeyPair.cpp\
                   $(GCLASSES_PATH)/GKNN.cpp\
                   $(GCLASSES_PATH)/GLearner.cpp\
                   $(GCLASSES_PATH)/GLList.cpp\
                   $(GCLASSES_PATH)/GManifold.cpp\
                   $(GCLASSES_PATH)/GMath.cpp\
                   $(GCLASSES_PATH)/GMatrix.cpp\
                   $(GCLASSES_PATH)/GNaiveBayes.cpp\
                   $(GCLASSES_PATH)/GNeuralNet.cpp\
                   $(GCLASSES_PATH)/GParseTree.cpp\
                   $(GCLASSES_PATH)/GParticleSwarm.cpp\
                   $(GCLASSES_PATH)/GPNG.cpp\
                   $(GCLASSES_PATH)/GPointerQueue.cpp\
                   $(GCLASSES_PATH)/GPolynomial.cpp\
                   $(GCLASSES_PATH)/GQueue.cpp\
                   $(GCLASSES_PATH)/GRand.cpp\
                   $(GCLASSES_PATH)/GRayTrace.cpp\
                   $(GCLASSES_PATH)/GRiddle.cpp\
                   $(GCLASSES_PATH)/GSDL.cpp\
                   $(GCLASSES_PATH)/GSearch.cpp\
                   $(GCLASSES_PATH)/GSocket.cpp\
                   $(GCLASSES_PATH)/GSphereImage.cpp\
                   $(GCLASSES_PATH)/GSpinLock.cpp\
                   $(GCLASSES_PATH)/GStabSearch.cpp\
                   $(GCLASSES_PATH)/GStack.cpp\
                   $(GCLASSES_PATH)/GStemmer.cpp\
                   $(GCLASSES_PATH)/GStream.cpp\
                   $(GCLASSES_PATH)/GString.cpp\
                   $(GCLASSES_PATH)/GThread.cpp\
                   $(GCLASSES_PATH)/GTime.cpp\
                   $(GCLASSES_PATH)/GTrie.cpp\
                   $(GCLASSES_PATH)/GTrigTable.cpp\
                   $(GCLASSES_PATH)/GVirtualMem.cpp\
                   $(GCLASSES_PATH)/GVector.cpp\
                   $(GCLASSES_PATH)/GVSM.cpp\
                   $(GCLASSES_PATH)/GWidgets.cpp\
                   $(GCLASSES_PATH)/GXML.cpp\
                   $(GCLASSES_PATH)/sha1.cpp\
                   $(GCLASSES_PATH)/sha2.cpp

GCLASSES_HEADER =  $(GCLASSES_PATH)/GArff.h\
                   $(GCLASSES_PATH)/GArray.h\
                   $(GCLASSES_PATH)/GAVLTree.h\
                   $(GCLASSES_PATH)/GBezier.h\
                   $(GCLASSES_PATH)/GBigNumber.h\
                   $(GCLASSES_PATH)/GBillboardCamera.h\
                   $(GCLASSES_PATH)/GBits.h\
                   $(GCLASSES_PATH)/GBitTable.h\
                   $(GCLASSES_PATH)/GBlindGreedy.h\
                   $(GCLASSES_PATH)/GBuffer.h\
                   $(GCLASSES_PATH)/GCautiousGreedy.h\
                   $(GCLASSES_PATH)/GCompress.h\
                   $(GCLASSES_PATH)/GConfSearch.h\
                   $(GCLASSES_PATH)/GCppParser.h\
                   $(GCLASSES_PATH)/GCrypto.h\
                   $(GCLASSES_PATH)/GDaftLearner.h\
                   $(GCLASSES_PATH)/GDataBase.h\
                   $(GCLASSES_PATH)/GDecisionTree.h\
                   $(GCLASSES_PATH)/GEZSocket.h\
                   $(GCLASSES_PATH)/GFile.h\
                   $(GCLASSES_PATH)/GFlipTable.h\
                   $(GCLASSES_PATH)/GFourier.h\
                   $(GCLASSES_PATH)/GFuzzyLogic.h\
                   $(GCLASSES_PATH)/GGenetic.h\
                   $(GCLASSES_PATH)/GGrammar.h\
                   $(GCLASSES_PATH)/GHardFont.h\
                   $(GCLASSES_PATH)/GHashTable.h\
                   $(GCLASSES_PATH)/GHeap.h\
                   $(GCLASSES_PATH)/GHtml.h\
                   $(GCLASSES_PATH)/GHttp.h\
                   $(GCLASSES_PATH)/GImage.h\
                   $(GCLASSES_PATH)/GKeyPair.h\
                   $(GCLASSES_PATH)/GKNN.h\
                   $(GCLASSES_PATH)/GLearner.h\
                   $(GCLASSES_PATH)/GLList.h\
                   $(GCLASSES_PATH)/GMacros.h\
                   $(GCLASSES_PATH)/GManifold.h\
                   $(GCLASSES_PATH)/GMemChunk.h\
                   $(GCLASSES_PATH)/GNaiveBayes.h\
                   $(GCLASSES_PATH)/GNeuralNet.h\
                   $(GCLASSES_PATH)/GParseTree.h\
                   $(GCLASSES_PATH)/GParticleSwarm.h\
                   $(GCLASSES_PATH)/GPNG.h\
                   $(GCLASSES_PATH)/GPointerQueue.h\
                   $(GCLASSES_PATH)/GPolynomial.h\
                   $(GCLASSES_PATH)/GQueue.h\
                   $(GCLASSES_PATH)/GRand.h\
                   $(GCLASSES_PATH)/GRayTrace.h\
                   $(GCLASSES_PATH)/GRiddle.h\
                   $(GCLASSES_PATH)/GSDL.h\
                   $(GCLASSES_PATH)/GSearch.h\
                   $(GCLASSES_PATH)/GSocket.h\
                   $(GCLASSES_PATH)/GSphereImage.h\
                   $(GCLASSES_PATH)/GSpinLock.h\
                   $(GCLASSES_PATH)/GStabSearch.h\
                   $(GCLASSES_PATH)/GStemmer.h\
                   $(GCLASSES_PATH)/GString.h\
                   $(GCLASSES_PATH)/GThread.h\
                   $(GCLASSES_PATH)/GTime.h\
                   $(GCLASSES_PATH)/GVector.h\
                   $(GCLASSES_PATH)/GVSM.h\
                   $(GCLASSES_PATH)/GWidgets.h\
                   $(GCLASSES_PATH)/GXML.h\
                   $(GCLASSES_PATH)/sha1.h\
                   $(GCLASSES_PATH)/sha2.h\
                   $(GCLASSES_PATH)/uitypes.h

################
# Lists
################

TEMP_LIST_1 = $(GCLASSES_SOURCES:$(GCLASSES_PATH)/%=$(OBJ_PATH)/%)
GCLASSES_OBJECTS_OPT = $(TEMP_LIST_1:%.cpp=%.o)
GCLASSES_OBJECTS_DBG = $(TEMP_LIST_1:%.cpp=%.dbg.o)

ALL_OBJECTS_OPT = $(GCLASSES_OBJECTS_OPT)
ALL_OBJECTS_DBG = $(GCLASSES_OBJECTS_DBG)
ALL_SOURCES = $(GCLASSES_SOURCES)
ALL_HEADERS = $(GCLASSES_HEADER)

################
# Rules
################

dbg: skipsomelines $(TARGET_PATH)/$(TARGET_NAME_DBG)

opt: skipsomelines $(TARGET_PATH)/$(TARGET_NAME_OPT)

skipsomelines:
	echo
	echo                    ################
	echo                        GClasses
	echo                    ################
	echo

usage:
	#
	# Usage:
	#  make usage   (to see this info)
	#  make clean   (to delete all the .o files)
	#  make dbg     (to build a debug version)
	#  make opt     (to build an optimized version)
	#

$(TARGET_PATH)/$(TARGET_NAME_OPT) : $(ALL_OBJECTS_OPT)
	ar -cq $(TARGET_PATH)/$(TARGET_NAME_OPT) $(ALL_OBJECTS_OPT)

$(TARGET_PATH)/$(TARGET_NAME_DBG) : $(ALL_OBJECTS_DBG)
	ar -cq $(TARGET_PATH)/$(TARGET_NAME_DBG) $(ALL_OBJECTS_DBG)

$(GCLASSES_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(GCLASSES_PATH)/%.cpp $(GCLASSES_HEADER)
	g++ $(OPT_CFLAGS) -c $< -o $@

$(GCLASSES_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(GCLASSES_PATH)/%.cpp $(GCLASSES_HEADER)
	g++ $(DBG_CFLAGS) -c $< -o $@

clean :
	rm -f $(ALL_OBJECTS_OPT)
	rm -f $(ALL_OBJECTS_DBG)
	rm -f $(TARGET_PATH)/GClasses.a
	rm -f $(TARGET_PATH)/GClassesDbg.a
