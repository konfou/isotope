/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

// XML Tag names
const char* TAG_NAME_FILE = "File";
const char* TAG_NAME_CLASS = "Class";
const char* TAG_NAME_INSTRUCTIONS = "Instructions";
const char* TAG_NAME_VAR = "Var";
const char* TAG_NAME_PROCEDURE = "Procedure";
const char* TAG_NAME_METHOD = "Method";
const char* TAG_NAME_INTERFACE = "Interface";
const char* TAG_NAME_MACHINE = "Machine";
const char* TAG_NAME_CALL = "Call";
const char* TAG_NAME_PARAM = "Param";
const char* TAG_NAME_SOURCE = "Source";
const char* TAG_NAME_BIN = "Bin";
const char* TAG_NAME_LIBRARY = "Library";
const char* TAG_NAME_BLOCK = "Block";
const char* TAG_NAME_STRINGTABLE = "StringTable";
const char* TAG_NAME_STRING = "String";
const char* TAG_NAME_FLOAT = "Float";
const char* TAG_NAME_CONSTANT = "Const";

// XML Attribute names
const char* ATTR_NAME = "Name";
const char* ATTR_EXP = "Exp";
const char* ATTR_TYPE = "Type";
const char* ATTR_START = "Start";
const char* ATTR_PARENT = "Parent";
const char* ATTR_PARENTID = "ParID";
const char* ATTR_DATA = "Data";
const char* ATTR_ID = "ID";
const char* ATTR_CALLBACK = "CB";
const char* ATTR_COMMENT = "Comment";
const char* ATTR_VAL = "Val";
const char* ATTR_SYNTAX = "Syntax";
const char* ATTR_THIS = "This";
const char* ATTR_SOURCE = "Source";
const char* ATTR_IMPORTED = "Imported";
const char* ATTR_CATCH = "Catch";

// Built-in XML Attribute values
const char* VAL_NULL = "null";
const char* VAL_THIS = "this";
const char* VAL_MAIN = "Main";
const char* VAL_CLASSIC = "Classic";
const char* VAL_XML = "XML";
const char* VAL_TRUE = "True";
const char* VAL_FALSE = "False";

// Internal classes
const char* CLASS_NAME_COMPARATOR = "Comparator";
const char* CLASS_NAME_MEMBER = "Member";
const char* CLASS_NAME_METHOD = "Method";
const char* CLASS_NAME_CLASS = "Class";

// Built in classes
const char* CLASS_NAME_OBJECT = "Object";
const char* CLASS_NAME_INTEGER = "Integer";
const char* CLASS_NAME_STACKLAYER = "StackLayer";
const char* CLASS_NAME_ASM = "Asm";
const char* CLASS_NAME_STRING = "String";
const char* CLASS_NAME_BOOL = "Bool";
const char* CLASS_NAME_FLOAT = "Float";

// Built in methods
const char* METHOD_NAME_IF = "if";
const char* METHOD_NAME_WHILE = "while";
const char* METHOD_NAME_ELSE = "else";
const char* METHOD_NAME_NEW = "new";

// File extensions
const char* EXT_XLIB = ".xlib";
const char* EXT_GASH = ".gash";

// Signatures
const char* SIG_STRING_GETCONSTANTSTRING = "method !getConstantString(Integer)";
const char* SIG_NEW = "method !new()";
const char* SIG_FROM_STREAM = "method !fromStream(&Stream)";
const char* SIG_EXCEPTION_NEW = "method !new(String)";
