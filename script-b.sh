#!/bin/bash

shell=sshell
shell_source=$1
wrongCommandMessage="Shell: Incorrect command"
count=1                             #Keep Test Case Count. Printed by print_status function and increased
totalmarks=0
cleanop()
{
    so=$1 
    sed -i -e 's/$ //g' "$so" #deleting spaces after end of lines
    sed -i -e '/^$/d' "$so" #deleting blank lines
}

if [ -z "$shell_source" ]
then
    echo "Please enter the source code as command line argument."
    exit
else
    gcc $shell_source -o $shell  &>/dev/null    #$shell is name of the executable
    if [ ! -e $shell ]
    then
        echo "Compilation failed"
        exit
    fi
fi

test_serial0()
{
killall sleep &> /dev/null
echo "Testcase $count: Serial command 'sleep 2 && sleep 2':"
i=partb_test$count.txt
touch $i
echo "sleep 2 && sleep 2"  > $i
echo exit >> $i
./$shell "$i" &
val1=$(ps -aux | grep -v "grep" | grep -c "sleep")
# echo $val1
sleep 3
val2=$(ps -aux | grep -v "grep" | grep -c "sleep")
sleep 3
if [[ "$val1" -eq 1 && "$val2" -eq 1 ]]
then
    echo "Test case passed"
    rm -f $i
    totalmarks=$(($totalmarks + 1))
else
    echo "Test failed. ";
fi
echo "---------------------------------------";
sleep 2
count=$(($count + 1))
}

test_serial1()
{
killall sleep &> /dev/null
killall ls &> /dev/null
echo "Testcase $count: Serial command 'sleep 2 && garbage && sleep 3':"
i=partbtests1.txt
so=partestops1.txt
touch $i $so
echo "sleep 2 && garbage && sleep 3"  > $i
echo "sleep 2" >> $i
echo exit >> $i
./$shell "$i" > "$so" &
val1=$(ps -aux | grep -v "grep" | grep -c "sleep")
# echo $val1
sleep 3
val2=$(ps -aux | grep -v "grep" | grep -c "sleep")
# echo $val2
sleep 4
val3=$(ps -aux | grep -v "grep" | grep -c "sleep")
sleep 2
countOfFailed=`grep -c "$wrongCommandMessage" $so`
if [[ "$val1" -eq 1 && "$val2" -eq 1 && "$val3" -eq 0 && "$countOfFailed" -eq 1 ]]
then
    echo "Test case passed"
    rm -f $i $so
    totalmarks=$(($totalmarks + 1))
else
    echo "Test case failed.  ";
fi
echo "---------------------------------------"
sleep 2
count=$(($count + 1))
}


test_parallel0()
{
killall sleep &> /dev/null
echo "Testcase $count: parallel command sleep 3 &&& sleep 3 &&& sleep 3"
i=partb_test$count.txt
so=partb_testop$count.txt
touch $i $so
echo "sleep 3 &&& sleep 3 &&& sleep 3"  > $i
echo "sleep 1" >> $i
echo exit >> $i
./$shell "$i" &
val=$(ps -aux | grep -v "grep" | grep -c "sleep")
# echo $val
sleep 4
if [ $val == 3 ]
then
    echo "Test case Passed";
    rm -f $i
    totalmarks=$(($totalmarks + 1))
else
    echo "Test failed";
fi
echo "---------------------------------------";
count=$(($count + 1))
}
test_parallel1()
{
killall sleep &> /dev/null
echo "Testcase $count: parallel command: sleep 3 &&& garbage &&& sleep 3"
i=partb_test$count.txt
so=partb_testop$count.txt
touch $i $so
echo "sleep 3 &&& garbage &&& sleep 3"  > $i
echo "sleep 1" >> $i
echo exit >> $i
./$shell "$i" > $so & 
val=$(ps -aux | grep -v "grep" | grep -c "sleep")
cleanop $so
countOfFailed=`grep -c "$wrongCommandMessage" $so`
# echo $val
sleep 4
if [[ $val == 2 && $countOfFailed == 1 ]]
then
    echo "Test case Passed";
    rm -f $i $so
    totalmarks=$(($totalmarks + 1))
else
    echo "Test failed. ";
fi
echo "---------------------------------------";
count=$(($count + 1))
}

test_background0()
{
killall -9 sleep &> /dev/null
echo "Testcase $count: sleep 100 &, sleep 100"
i=partb_test$count.txt
touch $i
echo "sleep 100 & " > $i
echo "sleep 100" >> $i
# echo exit >> $i
./$shell $i &
sleep 1
val=$(ps -aux | grep -v "grep" | grep -c "sleep")
# echo $val
if [ "$val" -eq 2 ];
then
    echo "Test Passed";
    totalmarks=$(($totalmarks + 1))
    rm -f $i
else
    echo "Test failed";
fi
#kill shell process
killall -9 $shell &> /dev/null
echo "---------------------------------------";
count=$(($count + 1))
}

echo --------
echo PART B 5 marks
echo --------
echo

test_serial0
test_serial1
test_parallel0
test_parallel1
test_background0
rm *testop*.txt
echo "Total marks: $totalmarks"
