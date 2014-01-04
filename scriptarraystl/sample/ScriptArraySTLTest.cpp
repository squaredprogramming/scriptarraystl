#include "stdafx.h"
#include <assert.h>
#include <conio.h>

// Include the definitions of the script library and the add-ons we'll use.
// The project settings may need to be configured to let the compiler where
// to find these headers. Don't forget to add the source modules for the
// add-ons to your project as well so that they will be compiled into the 
// application.
#include <angelscript.h>
#include "scriptstdstring/scriptstdstring.h"
#include "scriptbuilder/scriptbuilder.h"

#include "ScriptArraySTL/ScriptArraySTL.h"

// Implement a simple message callback function
void MessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERR ";
	if( msg->type == asMSGTYPE_WARNING ) type = "WARN";
	else if( msg->type == asMSGTYPE_INFORMATION ) type = "INFO";

	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

// Print the script string to the standard output stream
void print(std::string &msg)
{
	printf("%s", msg.c_str());
}


int _tmain(int argc, _TCHAR* argv[])
{
	// Create the script engine
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	// Set the message callback to receive information on errors in human readable form.
	int r = engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL); assert( r >= 0 );

	// AngelScript doesn't have a built-in string type, as there is no definite standard 
	// string type for C++ applications. Every developer is free to register it's own string type.
	// The SDK do however provide a standard add-on for registering a string type, so it's not
	// necessary to implement the registration yourself if you don't want to.
	RegisterStdString(engine);
	RegisterScriptArray(engine, true);

	// setup our string array
	CScriptArraySTL <std::string> string_array;
	string_array.InitArray(engine, "array<string>");

	// add some data to a vector of strings
	std::vector<std::string> string_std_vector;
	for(int i = 0; i < 10; ++i)
	{
		char buffer[64];
		sprintf(buffer, "\nTest from std::vector: %i", i);

		// this works because of some C++ compiler magic (it can make a std::string object because std:string has a constructor that takes char *)
		string_std_vector.push_back(buffer); 
	}

	// test assign from iterators **************************************************************
	// now assign the data from the other vector
	string_array.assign(string_std_vector.begin(), string_std_vector.end());

	// Register the function that we want the scripts to call 
	r = engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL); assert( r >= 0 );

	// register our array as a global variable in the script
	r = engine->RegisterGlobalProperty("array<string> string_array", string_array.GetRef()); assert( r >= 0 );

	// The CScriptBuilder helper is an add-on that loads the file,
	// performs a pre-processing pass if necessary, and then tells
	// the engine to build a script module.
	CScriptBuilder builder;
	r = builder.StartNewModule(engine, "MyModule"); 
	if( r < 0 ) 
	{
		// If the code fails here it is usually because there
		// is no more memory to allocate the module
		printf("Unrecoverable error while starting a new module.\n");
		return 0;
	}
	r = builder.AddSectionFromFile("test.as");
	if( r < 0 )
	{
		// The builder wasn't able to load the file. Maybe the file
		// has been removed, or the wrong name was given, or some
		// preprocessing commands are incorrectly written.
		printf("Please correct the errors in the script and try again.\n");
		return 0;
	}
	r = builder.BuildModule();
	if( r < 0 )
	{
		// An error occurred. Instruct the script writer to fix the 
		// compilation errors that were listed in the output stream.
		printf("Please correct the errors in the script and try again.\n");
		return 0;
	}

	// Find the function that is to be called. 
	asIScriptModule *mod = engine->GetModule("MyModule");
	asIScriptFunction *func = mod->GetFunctionByDecl("void main()");
	if( func == 0 )
	{
		// The function couldn't be found. Instruct the script writer
		// to include the expected function in the script.
		printf("The script must have the function 'void main()'. Please add it and try again.\n");
		return 0;
	}
	// Create our context, prepare it, and then execute
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(func);
	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
	{
		// The execution didn't complete as expected. Determine what happened.
		if( r == asEXECUTION_EXCEPTION )
		{
			// An exception occurred, let the script writer know what happened so it can be corrected.
			printf("An exception '%s' occurred. Please correct the code and try again.\n", ctx->GetExceptionString());
		}
	}

	// test iterators **************************************************************************

	// print changes made in the script
	printf("\n\n\nThis is C++\n");
	for(size_t i = 0; i < string_array.size(); ++i)
	{
		printf(string_array[i].c_str());
	}

	printf("\n\n\nThis is C++(with iterator)\n");
	for(CScriptArraySTL <std::string>::iterator it = string_array.begin(); it < string_array.end(); ++it)
	{
		printf((*it).c_str());
	}

	printf("\n\n\nThis is C++(with const iterator)\n");
	for(CScriptArraySTL <std::string>::const_iterator it = string_array.cbegin(); it < string_array.cend(); ++it)
	{
		printf((*it).c_str());
	}

	printf("\n\n\nThis is C++(with reverse iterator)\n");
	for(CScriptArraySTL <std::string>::reverse_iterator it = string_array.rbegin(); it < string_array.rend(); ++it)
	{
		printf((*it).c_str());
	}

	// test back *******************************************************************************
	string_array.back() = "\nThis is the back. (Old Data Erased)";

	// test front ******************************************************************************
	string_array.front() = "\nThis is the front. (Old Data Erased)";

	for(CScriptArraySTL <std::string>::iterator it = string_array.begin(); it < string_array.end(); ++it)
	{
		printf((*it).c_str());
	}

	// test fill assign ************************************************************************
	string_array.assign(10, "\ntesting fill");
	for(CScriptArraySTL <std::string>::iterator it = string_array.begin(); it < string_array.end(); ++it)
	{
		printf((*it).c_str());
	}

	// test iterators 2 ************************************************************************
	// add some data
	string_array.resize(0);
	for(int i = 0; i < 10; ++i)
	{
		char buffer[64];
		sprintf(buffer, "\nTesting iterators: %i", i);

		// this works because of some C++ compiler magic (it can make a std::string object because std:string has a constructor that takes char *)
		string_array.push_back(buffer); 
	}
	// write using iterators
	for(CScriptArraySTL <std::string>::iterator it = string_array.begin(); it < string_array.end(); it+=2)
	{
		printf((*it).c_str());
	}
	size_t size = string_array.end() - string_array.begin();
	printf("\n\nSize of current String = %u", size);

	//----------------------------------------------------
	string_array.Release();

	// Clean up
	ctx->Release();
	engine->Release();

	_getch();

	return 1;
}

