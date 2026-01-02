#!/bin/bash
#
# OpDiLib, an Open Multiprocessing Differentiation Library
#
# Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
# Copyright (C) 2023-2026 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
# Homepage: https://scicomp.rptu.de
# Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
#
# Lead developer: Johannes Blühdorn (SciComp, University of Kaiserslautern-Landau)
#
# This file is part of OpDiLib (https://scicomp.rptu.de/software/opdi).
#
# OpDiLib is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# OpDiLib is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License along with OpDiLib. If not, see
# <http://www.gnu.org/licenses/>.
#

DRIVER=$1
TEST=$2
GENFILE=$BUILD_DIR"/"$DRIVER$TEST".cpp"
MODE=$3
EXPLICIT_PREPROCESSOR=$4
STDERR_OUTPUT_IS_ERROR=$5

LAUNCH_NAME=$DRIVER$TEST
if [[ "$EXPLICIT_PREPROCESSOR" == "yes" ]];
then
  LAUNCH_NAME+="Pre";
fi

if [[ "$STDERR_OUTPUT_IS_ERROR" == "yes" ]];
then
	ERROR_FILE=$RESULT_DIR/$DRIVER$TEST.err;
else
	ERROR_FILE=/dev/null;
fi

case "$MODE" in
	"RUN")
		timeout 5m ./$BUILD_DIR/$LAUNCH_NAME 1> $RESULT_DIR/$DRIVER$TEST.out 2> $ERROR_FILE;
		ret=$?
		if [[ $ret -ne 0 ]];
		then
			echo -e $DRIVER$TEST "\e[0;31mTIMEOUT or ERROR\e[0m";
			cat $RESULT_DIR/$DRIVER$TEST.err;
			echo "1" >> testresults;
		else
			if [[ -s $RESULT_DIR/$DRIVER$TEST.err ]]
			then
				echo -e $DRIVER$TEST "\e[0;31mERROR\e[0m";
				cat $RESULT_DIR/$DRIVER$TEST.err;
				echo "1" >> testresults;
			elif cmp -s $RESULT_DIR/$DRIVER$TEST.ref $RESULT_DIR/$DRIVER$TEST.out;
			then
				echo -e $DRIVER$TEST "\e[0;32mOK\e[0m";
				echo "0" >> testresults;
			else
				echo -e $DRIVER$TEST "\e[0;31mFAILED\e[0m";
				diff $RESULT_DIR/$DRIVER$TEST.ref $RESULT_DIR/$DRIVER$TEST.out;
				echo "1" >> testresults;
			fi;
		fi;
		;;
	"REF")
		./$BUILD_DIR/$LAUNCH_NAME > $RESULT_DIR/$DRIVER$TEST.ref
		;;
esac
