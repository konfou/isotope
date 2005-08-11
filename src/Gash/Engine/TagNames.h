/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#ifndef __TAGNAMES_H__
#define __TAGNAMES_H__

// XML Tag names
extern const char* TAG_NAME_FILE;
extern const char* TAG_NAME_CLASS;
extern const char* TAG_NAME_INSTRUCTIONS;
extern const char* TAG_NAME_VAR;
extern const char* TAG_NAME_PROCEDURE;
extern const char* TAG_NAME_METHOD;
extern const char* TAG_NAME_INTERFACE;
extern const char* TAG_NAME_MACHINE;
extern const char* TAG_NAME_CALL;
extern const char* TAG_NAME_PARAM;
extern const char* TAG_NAME_SOURCE;
extern const char* TAG_NAME_BIN;
extern const char* TAG_NAME_SYM;
extern const char* TAG_NAME_LIBRARY;
extern const char* TAG_NAME_BLOCK;
extern const char* TAG_NAME_STRINGTABLE;
extern const char* TAG_NAME_STRING;
extern const char* TAG_NAME_FLOAT;
extern const char* TAG_NAME_CONSTANT;

// XML Attribute names
extern const char* ATTR_NAME;
extern const char* ATTR_EXP;
extern const char* ATTR_TYPE;
extern const char* ATTR_START;
extern const char* ATTR_PARENT;
extern const char* ATTR_PARENTID;
extern const char* ATTR_DATA;
extern const char* ATTR_ID;
extern const char* ATTR_CALLBACK;
extern const char* ATTR_COMMENT;
extern const char* ATTR_VAL;
extern const char* ATTR_SYNTAX;
extern const char* ATTR_THIS;
extern const char* ATTR_SOURCE;
extern const char* ATTR_IMPORTED;
extern const char* ATTR_CATCH;

// Built-in XML Attribute values
extern const char* VAL_NULL;
extern const char* VAL_THIS;
extern const char* VAL_MAIN;
extern const char* VAL_CLASSIC;
extern const char* VAL_XML;
extern const char* VAL_TRUE;
extern const char* VAL_FALSE;

// Internal classes
extern const char* CLASS_NAME_COMPARATOR;
extern const char* CLASS_NAME_MEMBER;
extern const char* CLASS_NAME_METHOD;
extern const char* CLASS_NAME_CLASS;

// Built in classes
extern const char* CLASS_NAME_OBJECT;
extern const char* CLASS_NAME_INTEGER;
extern const char* CLASS_NAME_STACKLAYER;
extern const char* CLASS_NAME_ASM;
extern const char* CLASS_NAME_STRING;
extern const char* CLASS_NAME_BOOL;
extern const char* CLASS_NAME_FLOAT;

// Built in methods
extern const char* METHOD_NAME_IF;
extern const char* METHOD_NAME_WHILE;
extern const char* METHOD_NAME_ELSE;
extern const char* METHOD_NAME_NEW;

// File extensions
extern const char* EXT_XLIB;
extern const char* EXT_GASH;

// Signatures
extern const char* SIG_STRING_GETCONSTANTSTRING;
extern const char* SIG_NEW;
extern const char* SIG_FROM_STREAM;
extern const char* SIG_EXCEPTION_NEW;

#endif // __TAGNAMES_H__
