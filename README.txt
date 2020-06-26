Author: Kaiming Cui
Email: cuik@iu.edu

Description:
My own shell has the following 4 features:
• A command line parser to process your text input, print an error message if the command line is
not valid.
• Input may include file redirection: <, >, and/or >>
• Input may have many pipes: cmd1 | cmd2 | cmd3 | … | cmdN
• Input may support the background: &

It looks like the following pseudocode:
while (1) {
printf(“your prompt”); 
Read one line from the user; 
Parse the line into an array of commands; 
Finally, execute the array of parsed commands if the input is a valid command line; //System calls
}



Document:
The tar ball "Lab2_KaimingCui.tar" contains four items: a C source code "MyShell.c", a Makefile, a README.txt and a header file called "header.h".

Environment:
This project is developed in C language on Linux server by using vim.
It can run on any linux server.

Dependency:
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

Building and Testing:
1. Upload the zip file to tesla server on local (before login).
    scp Lab2_KaimingCui.tar [user_name]@tesla.cs.iupui.edu:/home/[user_name]
    * [user_name] is your username in IUPUI.
2. Login to tesla server with your private password.
    ssh [user_name]@tesla.cs.iupui.edu
3. untar the tar ball.
	tar -xvf Lab2_KaimingCui.tar
4. Using Makefile to compile the source code
	make
5. The "KaimingShell.out" executable file is produced, run this file
	./KaimingShell.out
6. Then you will see the prompt "KaimingCui>" and you can input your command now.

7. Enter "exit" to exit my Shell.

9. Enter "Enter" will do nothing but show you a new prompt and wait for your command again.

10. If you use background mark "&" to run your command, then the parent process will not call wait(), you will get zombie process after the childs done.

11. After you see the output if you do not see the new prompt, press ENTER then you can see a new prompt and start a new input line.

Caution:
1. The max length of chars you can input is 1024
2. The max number of subcommand is 20 which means you can only input at most 19 pipes in your input line.
3. the max length of char of each subcommand is 50 chars.
4. You can only have at most one input redirection file and one output redirection file in your input.

Example:
//enter exit to exit
(base) [cuik@tesla Lab2]$ ./KaimingShell.out
KaimingCui>exit

//enter ENTER to get new prompt
(base) [cuik@tesla Lab2]$ ./KaimingShell.out
KaimingCui>
KaimingCui>
KaimingCui>
KaimingCui>exit

//a example to execute command and pipes
KaimingCui>cat <test.txt | sort > result.txt &
KaimingCui>cat test.txt
KaimingCui>5
6
7
1
2
3

KaimingCui>cat result.txt
KaimingCui>1
2
3
5
6
7

KaimingCui>exit
