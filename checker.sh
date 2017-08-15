#!/bin/bash

## CONSTANTS ##
CURRENT_DIRECTORY=`pwd`
RESOURCES_DIRECTORY=$1
CODING_STYLE_BONUS=0
ERROR_BONUS=0
ERRORS_NUMBER=0
PASSED_TESTS_GRADE=0

MESSAGE_SIZE=12
ONE=1
TWO=2
## FUNCTIONS ##
function cleanHomework
{
	rm -rf *.o test
}

function compileHomework
{
	echo ""
	# build here
}

function checkBonus
{
	printf '%*s\n' "${COLUMNS:-$(($(tput cols) - $ONE))}" '' | tr ' ' -
	echo "" > checkstyle.txt
	TargetDir=$RESOURCES_DIRECTORY
	
	echo -ne "Coding style Errors:\n"
	
	for entry in "$RESOURCES_DIRECTORY"/*
	do
		./cpplint.py "$entry" > checkstyle.txt 
		YOUR_BONUS=`cat checkstyle.txt | grep "Total errors found: "| tr -dc '0-9'`
		ERRORS_NUMBER=$(( $ERRORS_NUMBER + $YOUR_BONUS ))
	done

	if [[ $ERRORS_NUMBER != "0" ]]; then
		printf '\nBonus: %*sFAIL' "${COLUMNS:-$(($(tput cols) - $MESSAGE_SIZE))}" '' | tr ' ' -
		echo -ne "\n\t\tYou have a total of $ERRORS_NUMBER coding style errors.\n"

	else
		echo -ne "\n\tNone?\n\t\tWow, next level!"
		printf '\n\nBonus: %*sOK' "${COLUMNS:-$(($(tput cols) - $MESSAGE_SIZE + $TWO ))}" '' | tr ' ' -

		PASSED_TESTS_GRADE='10' ## /10; TODO - set here the number of passed tests; 
		CODING_STYLE_BONUS=$(($PASSED_TESTS_GRADE * 2))
		CODING_STYLE_BONUS=$(($CODING_STYLE_BONUS))
	fi
}

function printBonus
{
	if [[ $ERRORS_NUMBER -gt 0 ]]; then
	printf '\n%*s' "${COLUMNS:-$(tput cols)}" '' | tr ' ' -
		echo -ne  "\n\tAnd you almost did it!\n"
	else
		echo -ne "\n\tDamn!\n"
	fi
	echo -ne "\n\t\tYou got a bonus of 0.$CODING_STYLE_BONUS/0.2.\n\n"
}

## MAIN EXECUTION ##
cleanHomework
# compileHomework

## CHECK TESTS

## BONUS CHECK
checkBonus

## BONUS RESULTS
printBonus

# cleanHomework
