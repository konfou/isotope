# Linux Makefile for the GashSDK

# macro definitions
CC= g++
# the -g flag tells it to add debug information
FLAGS = -g
EXE_NAME = test

# program executable    
$(EXE_NAME):	GArray.o GBigNumber.o GBitTable.o GBucket.o GCompress.o GDate.o GDiff.o GGrammar.o GHash.o GLList.o GMutex.o GParseTree.o GSocket.o GStream.o GStuff.o GVirtualMem.o main.o
		$(CC) $(FLAGS) -lpthread -o $(EXE_NAME) GArray.o GBigNumber.o GBitTable.o GBucket.o GCompress.o GDate.o GDiff.o GGrammar.o GHash.o GLList.o GMutex.o GParseTree.o GSocket.o GStream.o GStuff.o GVirtualMem.o main.o
		chmod 711 $(EXE_NAME)

# .o files
GArray.o:		GArray.cpp GashSDK.h
		$(CC) $(FLAGS) -c GArray.cpp

GBigNumber.o:	GBigNumber.cpp GashSDK.h
		$(CC) $(FLAGS) -c GBigNumber.cpp

GBitTable.o:	GBitTable.cpp GashSDK.h
		$(CC) $(FLAGS) -c GBitTable.cpp

GBucket.o:	    GBucket.cpp GashSDK.h
		$(CC) $(FLAGS) -c GBucket.cpp

GCompress.o:	GCompress.cpp GashSDK.h GCompress.h
		$(CC) $(FLAGS) -c GCompress.cpp

GDate.o:	    GDate.cpp GashSDK.h
		$(CC) $(FLAGS) -c GDate.cpp

GDiff.o:	    GDiff.cpp GashSDK.h
		$(CC) $(FLAGS) -c GDiff.cpp

GGrammar.o:	    GGrammar.cpp GashSDK.h
		$(CC) $(FLAGS) -c GGrammar.cpp

GHash.o:	    GHash.cpp GashSDK.h
		$(CC) $(FLAGS) -c GHash.cpp

GLList.o:	    GLList.cpp GashSDK.h
		$(CC) $(FLAGS) -c GLList.cpp

GMutex.o:	    GMutex.cpp GashSDK.h
		$(CC) $(FLAGS) -c GMutex.cpp

GParseTree.o:	GParseTree.cpp GashSDK.h	
		$(CC) $(FLAGS) -c GParseTree.cpp

GSocket.o:		GSocket.cpp GashSDK.h
		$(CC) $(FLAGS) -c GSocket.cpp

GStream.o:		GStream.cpp GashSDK.h
		$(CC) $(FLAGS) -c GStream.cpp

GStuff.o:		GStuff.cpp GashSDK.h
		$(CC) $(FLAGS) -c GStuff.cpp

GVirtualMem.o:	GVirtualMem.cpp GashSDK.h	
		$(CC) $(FLAGS) -c GVirtualMem.cpp

main.o:			main.cpp
		$(CC) $(FLAGS) -c main.cpp
