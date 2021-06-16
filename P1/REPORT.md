# simple shell

## implementation

### preliminary information
Other than the file sshell.c the only other file being used is
userCommand.h. It contains a *struct command* that has essentially
two main fields: one to hold the first argument of an input(the command).
The second is an array that holds all the arguments of the input.
It also contains other boolean values that will be useful for error management.

I have divided the implementation into four stages:
1.parsing input
2.executing command
3.piping
4.error management

## stage 1:parsing input
parsing input is done in three distinct steps
1.provide spaces for characters '>' and '|' in the input string(cmd)
2.Split separates the arguments from the string.
3.Replace '$alphabet' characters with string contained in the arrayOfStrings[].

### Spacing
the void returnString(char* str, char* character) function takes the string 
(cmd in our case) and adds a blank space before and after the characters '|' and '>'. 
### Split
The split function is used to divide the input string into separate 
strings by spaces " ". It then returns a struct command with new
 command and arguments fields(let's call this struct command as 
input for our case).  
### Replace '$alphabet' characters with strings from arrayOfStrings
Now we iterate through every argument in our input until we find '$' character.
 The argument that has this character will be replaced by one of the strings in the arrayOfStrings.

## Stage 2: executing command
### Built in commands
Our input is tested to see if the command is "exit", "cd", "pwd", or "set". For exit 
we simply exit the program by exit(0). For pwd we use the function getcwd(). For cd we use the 
function chdir().

For set, we are using an arrayOfStrings[26] that holds a string corresponding
 to every possible character. To find out what position in the array the given 
string must go to, we subtract 97 from the ascii value of the given character that
we're setting our string to. 97 is the ascii value of a, thus the resultant number
must give you a value from 0 to 25 with each position corresponding to a single position in the array. 

###  Normal commands(mySystem())
If our command is none of the in built in commands it is executed by mySystem(). mySystem() uses fork()
 and then execvp() in the child process and returns the exit status through 
the parent process.
 In the child process, we check for the redirection character '>'.
 This is done by the function redirection(input) which takes the input
 and returns the argument position which contains '>'.
 The very next argument(the file name) is used to open the file 
and get its file descriptor. It's descriptor is then used to duplicate it to STDOUT. 

## Stage 3:piping
Piping is implemented in a separate function of its own called 
pipingFunction(). The input string is first tested for
 the piping character "|". If it exists, spaces will be added 
before and after it, and then it is split based on "|". 
This will always give us one more argument than the number of pipes. 
In this case argument refers to individual strings before and after piping symbols.

In pipingFunction(), we first fork and then depending on the number of 
arguments given, let the function do different things with 
the help of if statements. Depending on the number of arguments 
the children of the initial parent process will fork individually.
 They will each wait for the completion of their children 
before they perform execvp on their own. In order to 
account for redirection, I had used the function mySystem() 
to perform the command instead of execvp. After mySystem()
 had returned an exit status I use exit() to end the process 
returning my desired exit status. This way as each of the child 
process resolves, the parent will be the only process remaining.
 We must keep in mind that the initial fork was made only in order
 to escape the modifications of any file descriptors. Finally
 the function will return the exit status and print out the completion message.

## Stage 4:Error management

Errors such as "too many Arguments", "missing command", and "no output file"
are boolean values in  struct command. Thus, whether these errors occur, is 
decided during the split function while the function breaks down the input
string by spaces. 

Error for "cannot open output file" is handled 
in mySystem(). It is during the function mySystem() we try to open the 
given file. If the file descriptor returns -1 then we report the error. 
The error for "command not found" is also implemented in mySystem(). If execvp
 is unsuccessful the child process would not end right away allowing us the 
chance for printing the error message. 

Error for "mislocated output redirection" is also one of the field of struct 
command. Since it is a piping specific error, once split is done on the input
 string on the character "|", we simply iterate through each of them to make
 sure that none of the other instructions but the last one has a ">" in them.

For "cannot cd into directory" we simply take the return integer of the function
 chdir(). If the function returns the value -1 then this error must be displayed.

 For "invalid variable name" we must check for an invalid variable while setting 
or accessing environment variables.  If the next argument after the command 
set is NULL, or if the strlen() of the variable is greater than one, 
or if the ascii of the character-97 does not fall within the range of 0 and 25 
it is an invalid variable. Similar checks are performed in environmentVariables()
 function while accessing the string corresponding to the given characters. 
