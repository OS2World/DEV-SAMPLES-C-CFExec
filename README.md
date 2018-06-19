# DEV-SAMPLES-C-CFExec
Sample of how processes work. Lines of text are read from stdin until EOF.  Lines whose first word is "CL" are assumed to be attempts to compile C source files. A child process is started with the CL.EXE command, the rest of the line containing "CL" being the arguments.
