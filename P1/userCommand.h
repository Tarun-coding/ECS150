#include <stdbool.h>
struct command{
       	char* fileName;
	char* args[20];	
	bool tooManyArguments;
	bool missingCommand;
	bool noOutputFile;
	bool mislocatedOutputRedirection;
};
