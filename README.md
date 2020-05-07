# Shell-Program

Welcome to the 3207Project2 shell. The current working directory will print, followed by the > key. This prompts you, the user, to enter a command that the shell will execute. The shell has some built-in commands, but also supports many external commands, and can execute files. The shell also handles I/O redirection, meaning that you may change where a program gets and puts its input and output. This is done with the characters < and >, where < is input redirection and < is output redirection. This will allow you to do things such as write the output of your command to a file, to be read later. The shell also supports taking the output of one command and using it directly for the input of another function. This mechanism is called piping, and is done with the | operator. The shell also supports commands from a batch file, where if it will procedurally work through commands from a file. To do this, run the shell program with the batch file as the first parameter. There are some functions built into the shell, but the shell can also execute external programs. The internal commands are listed below:

"Cd": changes the current directory to the word that proceeds it (cd CTest)
"clear": clears the screen
"Ls": lists all contents of current directory
"environ": lists all environment variables, such as shell name, user, and home directory
"echo": repeats the input to the screen, or other output source
"help": displays this window
"Pause": stops working until the user enters a key
"exit": terminates the program
