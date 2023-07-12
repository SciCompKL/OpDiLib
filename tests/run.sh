#!/bin/bash
#
# OpDiLib, an Open Multiprocessing Differentiation Library
#
# Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
# Copyright (C) 2023 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
# Homepage: http://www.scicomp.uni-kl.de
# Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
#
# Lead developer: Johannes Blühdorn (SciComp, University of Kaiserslautern-Landau)
#
# This file is part of OpDiLib (http://www.scicomp.uni-kl.de/software/opdi).
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

LAUNCH_NAME=$DRIVER$TEST
if [[ "$EXPLICIT_PREPROCESSOR" == "yes" ]];
then
  LAUNCH_NAME+="Pre";
fi 

case "$MODE" in
	"RUN")
		./$BUILD_DIR/$LAUNCH_NAME 1> $RESULT_DIR/$DRIVER$TEST.out
		if cmp -s $RESULT_DIR/$DRIVER$TEST.ref $RESULT_DIR/$DRIVER$TEST.out;
		then
			echo -e $DRIVER$TEST "\e[0;32mOK\e[0m";
		else
			echo -e $DRIVER$TEST "\e[0;31mFAILED\e[0m";
			diff $RESULT_DIR/$DRIVER$TEST.ref $RESULT_DIR/$DRIVER$TEST.out;
		fi;
		;;
	"REF")
		./$BUILD_DIR/$LAUNCH_NAME > $RESULT_DIR/$DRIVER$TEST.ref
		;;
esac
