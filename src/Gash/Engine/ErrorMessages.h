/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

// *** If you get a compiler-error on this line, the error is really
// *** on the line that declares the ERROR_MACRO macro just before
// *** it #include's this file.
ERROR_MACRO(ERROR_NONE, L"There is no error.  The operation completed successfully.", 0)
ERROR_MACRO(INTERNAL_ERROR, L"There was an internal error.  This is not supposed to ever happen.  This bug brings ultimate shame and dishonor upon the heads of the programmers who wrote such shoddy code", 0)

// XML errors
ERROR_MACRO(BAD_XML, L"Couldn't parse the XML file.  It is probably not a valid XML file.", 0)
ERROR_MACRO(EXPECTED_VAR_TAG, L"Expected a \"Var\" tag.", 0)
ERROR_MACRO(EXPECTED_PARAM_TAG, L"Expected a \"Param\" tag.", 0)
ERROR_MACRO(EXPECTED_CLASS_MEMBER_TAG, L"Expected a \"Var\", \"Constant\", \"Procedure\", \"Method\", or \"Interface\" tag.", 0)
ERROR_MACRO(EXPECTED_COMMANDS_CHILD_TAG, L"Expected a \"Commands\" tag.", 0)
ERROR_MACRO(EXPECTED_CLASS_OR_INTERFACE_TAG, L"Expected a \"Class\" or \"Interface\" tag.", 0)
ERROR_MACRO(EXPECTED_METHODSIG, L"Expected a \"MethodSig\" tag.", 0)
ERROR_MACRO(EXPECTED_ASM_CALL_OR_CALLBACK_TAG, L"Expected a \"Asm\", \"Call\", or \"CB\" tag.", 0)
ERROR_MACRO(EXPECTED_SOURCE_TAG, L"Expected a \"Source\" tag.", 0)
ERROR_MACRO(EXPECTED_SOURCES_TAG, L"Expected a \"Sources\" tag.", 0)
ERROR_MACRO(EXPECTED_FILE_TAG, L"Expected a \"File\" tag.", 0)
ERROR_MACRO(EXPECTED_FILE_OR_LIBRARY_TAG, L"Expected a \"File\" or \"Library\" tag.", 0)
ERROR_MACRO(EXPECTED_NAME_ATTRIBUTE, L"This tag should have a \"Name\" attribute.", 0)
ERROR_MACRO(EXPECTED_PARENT_ATTRIBUTE, L"This tag should have a \"Parent\" attribute.", 0)
ERROR_MACRO(EXPECTED_VALUE_ATTRIBUTE, L"This tag should have a \"Value\" attribute.", 0)
ERROR_MACRO(EXPECTED_TYPE_ATTRIBUTE, L"This tag should have a \"Class\" or an \"Interface\" attribute.", 0)
ERROR_MACRO(EXPECTED_EXP_ATTRIBUTE, L"This tag should have a \"Exp\" attribute.", 0)
ERROR_MACRO(EXPECTED_SYNTAX_ATTRIBUTE, L"This tag should have a \"Syntax\" attribute.", 0)
ERROR_MACRO(EMPTY_EXPRESSION, L"This is an empty expression.", 0)

// Bad Attribute Values
ERROR_MACRO(EXPECTED_NONE_OBJ_OR_ALL, L"Expected the value to be \"None\", \"Obj\", or \"All\".", 0)

// Not found errors
ERROR_MACRO(TYPE_NOT_FOUND, L"There is no type named \"%s\".", 1)
ERROR_MACRO(METHOD_NOT_FOUND, L"There is no method named \"%s\".", 1)
ERROR_MACRO(INTERFACE_NOT_FOUND, L"There is no interface with that name.", 0)
ERROR_MACRO(MEMBER_NOT_FOUND, L"This class has no member with that name.", 0)
ERROR_MACRO(NAMED_CONSTANT_NOT_FOUND, L"There is no named constant with that name.", 0)
ERROR_MACRO(VARIABLE_NOT_FOUND, L"There is no variable named \"%s\" defined in this scope.", 1)

// Variables
ERROR_MACRO(VARIABLE_ALREADY_DECLARED, L"A variable named \"%s\" is already defined in this scope.", 1)

// Parameters
ERROR_MACRO(TOO_MANY_PARAMETERS, L"Too many parameters.", 0)
ERROR_MACRO(NOT_ENOUGH_PARAMETERS, L"Not enough parameters.", 0)

// Expressions
ERROR_MACRO(BAD_EXPRESSION, L"Bad expression.  I can't parse it.", 0)
ERROR_MACRO(EXPRESSION_CANT_EVALUATE_TO_MODIFIABLE_VALUE, L"That expression can only be evaluated to a constant value, but you're trying to call a method that would modify it.", 0)
ERROR_MACRO(PROCS_HAVE_NO_THIS_VAR_TO_MODIFY, L"Procedures have no \"This\" variable to modify with \"&\" or \"!\".", 0)

// Classic Syntax
ERROR_MACRO(UNEXPECTED_EOF, L"Unexpected end of file.", 0)
ERROR_MACRO(EXPECTED_CLASS_INTERFACE_OR_MACHINE, L"Expected \"class\", \"interface\", or \"machine\".", 0)
ERROR_MACRO(EXPECTED_OPEN_SQUIGGLY_BRACE, L"Expected a \"{\" token.", 0)
ERROR_MACRO(EXPECTED_CLOSE_SQUIGGLY_BRACE, L"Expected a \"}\" token.", 0)
ERROR_MACRO(EXPECTED_OPEN_PAREN, L"Expected a \"(\" token.", 0)
ERROR_MACRO(EXPECTED_CLOSE_PAREN, L"Expected a \")\" token.", 0)
ERROR_MACRO(EXPECTED_CLOSE_ANGLE_BRACE, L"Expected a \">\" token.", 0)
ERROR_MACRO(EXPECTED_COMMA_TOKEN, L"Expected a \",\" or a \")\" token.", 0)
ERROR_MACRO(EXPECTED_PERIOD_TOKEN, L"Expected a \".\" token.", 0)
ERROR_MACRO(EXPECTED_METHOD_TOKEN, L"Expected a \"Method\" token.", 0)
ERROR_MACRO(EXPECTED_INSTRUCTION, L"Expected a \"Method\", \"Proc\", \"Const\", \"Interface\", or \"[\" token.", 0)
ERROR_MACRO(EXPECTED_COLON, L"Expected a \":\".", 0)
ERROR_MACRO(EXPECTED_BANG_MODIFIER, L"Expected a \"!\" in front of this expression because the variable \"%s\" will be modified.", 1);
ERROR_MACRO(EXPECTED_AMPERSAND_MODIFIER, L"Expected an \"&\" in front of this expression because the object \"%s\" will be modified.", 1);

// File IO Errors
ERROR_MACRO(ERROR_OPENING_FILE_TO_READ, L"Unable to open the file for reading.  (This usually happens when the file doesn't exist.  If it does exist, then some other program probably has the file locked from sharing.)", 0)
ERROR_MACRO(ERROR_READING_FILE, L"There was an error reading from the file.", 0)
ERROR_MACRO(FILE_NOT_FOUND, L"The file \"%s\" was not found.", 1)
ERROR_MACRO(ERROR_SAVING, L"Unable to save/create  the file \"%s\".", 1)
ERROR_MACRO(UNRECOGNIZED_FILE_FORMAT, L"Unable to recognize the format of the file \"%s\".", 1)

// Other
ERROR_MACRO(SOURCE_ALREADY_LOADED, L"There is already source loaded.  You must unload it before you can load other source.", 0)
ERROR_MACRO(UNKNOWN_OPERATOR, L"Bad operator.", 0)
ERROR_MACRO(TYPES_MUST_BE_THE_SAME, L"Both variables must be for the same class to do this operation.", 0)
ERROR_MACRO(UNTERMINATED_STRING_CONSTANT, L"A string constant must begin and end with quotes.  This expression begins but does not end with one.", 0)
ERROR_MACRO(INVALID_CONSTANT_CHARACTER, L"Invalid constant character.  A constant must be an integer number, or a string enclosed in quotes.", 0)
ERROR_MACRO(INVALID_LIBRARIES_FOLDER, L"Bad libraries folder.  Can't change to that directory.", 0)
ERROR_MACRO(BAD_LIBRARY, L"There's a library in the Libraries folder that has problems and can't be loaded.", 0)
ERROR_MACRO(UNBALANCED_PARENS, L"Unbalanced parens.  There is a '(' without a matching ')', or something like that.", 0)
ERROR_MACRO(MISSING_OPERATOR, L"There should be an operator between two expressions.", 0)
ERROR_MACRO(INTEGER_TOO_BIG, L"An integer must be a 32-bit value.", 0)
ERROR_MACRO(ELSE_MUST_BE_CHILD_OF_IF, L"An \"Else\" instruction must be a child of an \"If\" instruction.", 0)
ERROR_MACRO(BAD_CAST, L"Can not cast object to that class because neither class inherrits from the other.", 0)
ERROR_MACRO(INVALID_SYNTAX_TYPE, L"Invalid syntax type.  Valid values are: \"Classic\", and \"XML\".", 0)
ERROR_MACRO(COULDNT_FIND_GETCONSTANTSTRING, L"Couldn't find the machine method \"!String.getConstantString(Integer)\".", 0)
ERROR_MACRO(COULDNT_FIND_IMPLICIT_CALL_TARGET, L"Couldn't find the target of an implicit call: %s", 1)
ERROR_MACRO(WRONG_SOURCE, L"This source code does not compile to that compiled code.", 0)
ERROR_MACRO(ALLOCATE_MUST_BE_CALLED_IN_SAME_CLASS_PROC, L"\"allocate\" may only be called from the class you are constructing.  (This rule elliminates the need for constructors.)  Use the \"new\" method of that class instead (if it has one).", 0)
ERROR_MACRO(CANT_MAKE_NEW_INTERFACE, L"You can't call \"New\" on an interface.  (But you can call \"New\" on a class that implements the interface.)", 0)
ERROR_MACRO(CLASS_DOESNT_IMPLEMENT_INTERFACE, L"This class doesn't implement that interface.", 0)
ERROR_MACRO(CANT_SET_DIFFERENT_INTERFACES, L"Those are two different interfaces.", 0)
ERROR_MACRO(INTERFACES_CANT_HAVE_MEMBERS, L"Interfaces can't have members.", 0)
ERROR_MACRO(UNRECOGNIZED_SYNTAX_TYPE, L"Unrecognized syntax type.", 0)
ERROR_MACRO(USING_CLASS_TO_CALL_SIGNATURE, L"You are attempting to use a class-object to call a signature.  You should call the method that implements that signature.", 0)
ERROR_MACRO(ELSE_WITHOUT_CORRESPONDING_IF, L"This \"else\" does not have a corresponding \"if\".", 0)
ERROR_MACRO(INSTRUCTION_CANT_HAVE_CHILDREN, L"This instruction can not have child instructions.", 0)
ERROR_MACRO(AMBIGUOUS_CALL, L"Ambiguous call.  There is more than one method with the same name and a matching signature.", 0)
ERROR_MACRO(NO_METHOD_MATCHES_SIGNATURE, L"None of the methods with that name have a signature that is compatible with those parameters.", 0)
ERROR_MACRO(UNHANDLED_EXCEPTION, L"An unexpected exception was thrown.", 0)
ERROR_MACRO(OUT_OF_MEMORY, L"Out of memory", 0)
ERROR_MACRO(NO_MAIN_PROC, L"There is no procedure named main.", 0)
ERROR_MACRO(CANT_BUILD_MULTIPLE_PROJECTS, L"Only one project may be built at a time.", 0)
ERROR_MACRO(CANT_MODIFY_VARIABLE, L"You don't have permission to modify the variable \"%s\" because it wasn't declared with a \"!\".", 1)
ERROR_MACRO(CANT_MODIFY_OBJECT, L"You don't have permission to modify that object because the variable \"%s\" wasn't declared with a \"&\" or a \"!\".", 1)
