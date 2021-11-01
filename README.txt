CS 344 Assignment 3 - Smallsh
Created by Nicole Yarbrough

This project was created to solve the prompt provided for assignment 3 in the Oregon State 
University Operating Systems I course.

To compile the code, unzip all files and run the command "make". If you would like to delete
the executable, use the command "make clean"

This program is replicating a shell that handles commands such as "ls", "wc", and more.
If you would like to start the shell, type "./smallsh" into the terminal. This will bring up 
a prompt to enter commands that you would normally enter into the Linux terminal, such as 
"ls -a" and "cd ..". The smallsh terminal can be used much like the Lunux terminal. The comment 
character is #. Blank lines of terminal input will be ignored. Add a '&' character at the end of 
the line to run the command in the background.

The executable can also be run with a bash script. To do so, save the script in the same folder as
smallsh, then chmod it by typing "chmod +x ./p3testscript" into the Linux shell. You can run the 
test script by "./p3testscript", "./p3testscript 2>&1", "./p3testscript 2>&1 | more", or 
"./p3testscript > mytestresults 2>&1". The p3testscript has been included in this zipped folder as 
an example.