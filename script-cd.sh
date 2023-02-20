#!/bin/bash
# Execute this script with source code of the shell as command line argument
count=1								#Keep Test Case Count. Printed by print_status function and increased
shell=myshell						#sshell and not simply shell to keep gnome-shell alive while pkill
shell_source=$1
totalmarks=0

if [ -z "$shell_source" ]
then
	echo "Please enter the source code as command line argument."
	exit
else
	gcc $shell_source -o $shell  &>/dev/null	#$shell is name of the executable
	if [ ! -e $shell ]
	then
		echo "Compilation failed"
		exit
	fi
fi

echo --------
echo PART CD 5 marks
echo --------
echo

command=bash_test_part_c.txt	#No need to change this

# Given a string as only argument this function checks whether any matching process running
# It performs regular expression matching
check_status() {
	if pgrep $1 > /dev/null
	then
		return 1	#Running
	else
		return 0	#Not running
	fi
}

# This prints if a test case passed or failed. It takes 2 arguments. 1st an Integer, 2nd a string.
# The function prints the string. Then appends "WORKING" or "NOT Working" based on 1st arg.
print_status() {
	if [ "$1" -eq 1 ]
	then
		echo "Testcase $count : \"$2\" WORKING"
		totalmarks=$(($totalmarks + 1))
	else
		echo "Testcase $count : \"$2\" NOT WORKING"
	fi
	count=$(($count + 1))
}

# Generating some test program
echo "#include <stdio.h>
#include <unistd.h>
int main () {
	int val = 0;
	while (1){
		
	}
}" > ./never_ending.c

gcc ./never_ending.c -o never_ending1
gcc ./never_ending.c -o never_ending2
gcc ./never_ending.c -o never_ending3
gcc ./never_ending.c -o never_ending4

###---Checking Whether shell is not killed by SIGINT---###
echo "sleep 100" > ./$command
(./$shell ./$command &) &>/dev/null
check_status $shell 					#Shell Should run now
shell_status1=$?
pkill -2 $shell 						#Sending SIGINT to shell
check_status $shell
shell_status2=$?						#If SIGINT Handled this should also be 1
status=$(( $shell_status1 && $shell_status2))
print_status $status "SIGINT NOT killing the shell"
pkill -9 sleep
pkill -9 $shell

# Start Checking for exit implementation
# Put the child shell in sleep for 0.2 sec
# During That master script checks whether the child is running
# Then Master script wait for 0.3 second
# During that child will execute wait and exit
# Then Master script again checks child is running or not
# If child was running during 1st check and not running during 2nd check
# Then exit must work. Else Exit NOT working.
# Kill the child shell with SIGKILL

rm -rf ./$command
echo "sleep 0.2" > ./$command
echo "exit" >> ./$command

(./$shell ./$command &) &>/dev/null

check_status $shell
ret_stat1=$?
sleep 0.3
check_status $shell
ret_stat2=$?
ret_stat=$(($ret_stat1 && !$ret_stat2))
print_status $ret_stat "Exit"
pkill -9 shell

# Checking whether Ctrl + C is killing foreground process or nor
# Forking two while(1) type never ending process 1st in BG second in FG
# Sending SIGINT to all process of same group ID
# FG should stop and BC continue. Shell sleep for 0.2 sec when master script check
rm -rf ./$command
echo "./never_ending1 &" > ./$command		#Child shall run this in background
echo "./never_ending2" >> ./$command		#Child shall run this in foreground
echo "sleep 0.2" >> ./$command 				#During this we'll check 1 running and 2 not running
echo "exit" >> ./$command
(./$shell ./$command &) &>/dev/null
shell_pid=$(pgrep $shell)					#Getting the shell's PID
#echo $shell_pid
sleep 1										#VERY IMP. MY SHELL SLEEP 1 SEC after each BG fork. Student's shell should not
# echo $(ps -e -o pgid,pid | grep $shell_pid | awk '{print $1}')
pkill -2 -g $(ps -e -o pgid,pid | grep $shell_pid | awk '{print $1}')		# Sending Signal to all forground process
check_status $shell
shell_status=$?
#echo $shell_status
check_status "never_ending1"
ne1_status=$?
#echo $ne1_status
check_status "never_ending2"
ne2_status=$?
#echo $ne2_status
status=$(($shell_status && $ne1_status && !$ne2_status))
print_status $status "Ctrl + C killed foreground but background process and Shell running"
pkill -9 $shell
pkill -9 "never"

###----Start Checking A SINGLE Background process are exiting on exit -----###
rm -rf ./$command
echo "./never_ending1 &" > ./$command
echo "exit" >> ./$command
(./$shell ./$command &) &>/dev/null
shell_pid=$(pgrep $shell)			#Getting Shell's PID
sleep 1.1								# Give some time for shell to work
check_status $shell
shell_status=$?						# Shell should exit now
check_status "never_ending1"
ne1_status=$?						#ne1_status = 0 here for correct implementation
status=$((!$shell_status && !$ne1_status))	#Both should be 0 now
print_status $status "Shell Killed single background process while exit"
pkill -9 $shell
pkill -9 "never"

###---Start Checking whether shell killed ALL background process on EXIT ----####
rm -rf ./$command
echo "./never_ending1 &" > ./$command
echo "./never_ending2 &" >> ./$command
echo "./never_ending3 &" >> ./$command
echo "./never_ending4 &" >> ./$command
echo "exit" >> ./$command
(./$shell ./$command &) &>/dev/null
sleep 4.2
check_status $shell
shell_status=$?
check_status "never_ending1"
ne1_status=$?
check_status "never_ending2"
ne2_status=$?
check_status "never_ending3"
ne3_status=$?
check_status "never_ending4"
ne4_status=$?
status=$((!$shell_status && !$ne1_status && !$ne2_status && !$ne3_status && !$ne4_status)) #All exited
print_status $status "Shell Killed ALL Background process upon exit"
pkill -9 $shell
pkill -9 "never"

rm -rf never*		#Remove all NEVER Ending process
rm -rf $shell
rm -rf $command
echo "Total marks: $totalmarks"

