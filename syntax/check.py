#  OpDiLib, an Open Multiprocessing Differentiation Library
#
#  Copyright (C) 2020-2021 Chair for Scientific Computing (SciComp), TU Kaiserslautern
#  Homepage: http://www.scicomp.uni-kl.de
#  Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
#
#  Lead developer: Johannes Blühdorn (SciComp, TU Kaiserslautern)
#
#  This file is part of OpDiLib (http://www.scicomp.uni-kl.de/software/opdi).
#
#  OpDiLib is free software: you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  OpDiLib is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty
#  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License for more details.
#  You should have received a copy of the GNU
#  General Public License along with OpDiLib.
#  If not, see <http://www.gnu.org/licenses/>.
#
#  For other licensing options please contact us.
#


import json
import sys
import os
from fnmatch import filter
from enum import Enum
from argparse import ArgumentParser

RED = "\033[1;31m"
GREEN = "\033[0;32m"
RESET = "\033[0;0m"


class State(Enum):
    CODE = 1
    COMMENT = 2
    PREPROCESSOR = 3


def is_opening(keyword, syntax):
    return keyword in syntax["pairs"].keys()


def is_closing(keyword, syntax):
    return keyword in syntax["pairs"].value()


def get_keywords(syntax):
    keys = list(syntax["pairs"].keys())
    values = list(syntax["pairs"].values())
    keys.extend(values)
    return set(keys)


def get_matches(line, syntax):
    keywords = get_keywords(syntax)

    matches = {}
    for keyword in keywords:
        index = line.find(keyword)
        if index != -1:
            if index not in matches:
                matches[index] = []
            matches[index].append(keyword)

    for index in matches:
        matches[index].sort(key=lambda x: len(x))

    unique = [match_list[-1] for match_list in matches.values()]

    return unique


def print_status(status, line_no, line, verbose):
    if verbose:
        print(F"line {line_no}, {status}: {line}")


def print_error(filename, line_no, message):
    print(F"{filename}, line {line_no}: {RED}{message}{RESET}")


def check_file(code_file, syntax_file, verbose):
    if verbose:
        print(F"Checking {code_file} using {syntax_file}.")

    syntax_handle = open(syntax_file, "r")
    syntax = json.load(syntax_handle)
    syntax_handle.close()

    code_handle = open(code_file, "r")

    line_no = 0

    stack = []

    state = State.CODE

    # parse line by line
    while True:
        line_no += 1
        line = code_handle.readline()
        if not line:
            break
        line = line.strip()

        # eliminate /* */ comments within this line
        begin = line.find("/*")
        while begin != -1 and line.find("*/", begin) != -1:
            end = line.find("*/", begin) + 2
            line = line[:begin].strip() + ' ' + line[end:].strip()
            begin = line.find("/*")

        # skip /* */ comment blocks
        if state == State.CODE and line.find("/*") != -1:  # comment begins in this line, parse the leading part
            print_status("comment", line_no, line, verbose)
            line = line[:line.find("/*")].strip()
            print_status("leading code", line_no, line, verbose)
            state = State.COMMENT
        elif state == State.COMMENT:
            if line.find("*/") != -1:  # comment ends in this line, parse the trailing part
                print_status("comment", line_no, line, verbose)
                line = line[line.find("*/") + 2:].strip()
                if line.find("/*") == -1:
                    print_status("trailing code", line_no, line, verbose)
                    state = State.CODE
                else:
                    line = line[:line.find("/*")].strip()
                    print_status("enclosed code", line_no, line, verbose)
            else:  # the whole line is part of the comment
                print_status("comment", line_no, line, verbose)
                continue

        if len(line) == 0:
            continue

        # skip preprocessor commands
        if state == State.CODE and line[0] == '#':
            state = State.PREPROCESSOR

        if state == State.PREPROCESSOR:
            print_status("preprocessor", line_no, line, verbose)
            if line[-1] != '\\':
                state = State.CODE
            continue

        # strip // comments
        if line.find("//") != -1:
            print_status("comment", line_no, line, verbose)
            line = line[:line.find("//")].strip()
            print_status("leading code", line_no, line, verbose)

        if len(line) == 0:
            continue

        matches = get_matches(line, syntax)

        for match in matches:
            if is_opening(match, syntax):
                print_status("opening macro", line_no, line, verbose)
                stack.append((match, line_no))
            else:
                print_status("closing macro", line_no, line, verbose)
                if len(stack) == 0:
                    print_error(code_file, line_no, F"lonely end macro {match}")
                    return False
                elif match != syntax["pairs"][stack[-1][0]]:
                    print_error(code_file, stack[-1][1], F"{stack[-1][0]} completed by {match}")
                    return False
                else:
                    del stack[-1]

    if len(stack) != 0:
        print_error(code_file, stack[-1][1], F"{stack[-1][0]} lacks end macro")
        return False

    return True


def check_and_report_result(filename, syntax, stop_on_error, verbose):
    result = check_file(filename, syntax, verbose)
    if result:
        print(F"{filename} {GREEN}OK{RESET}")
    if not result and stop_on_error:
        sys.exit(1)
    return result


def main():
    parser = ArgumentParser(description='OpDiLib syntax checker')

    parser.add_argument("syntax", help="syntax file")
    parser.add_argument("path", help="file or directory to check")
    parser.add_argument("-r", "--recursive", action='store_true', help="also check subdirectories")
    parser.add_argument("-p", "--patterns", default="*.hpp,*.cpp", help="files to check in directories")
    parser.add_argument("-s", "--stop-on-error", action='store_true', help="stop on first error")
    parser.add_argument("-v", "--verbose", action='store_true', help="display reasoning about code")

    args = parser.parse_args()

    patterns = args.patterns.split(",")

    all_fine = True

    if os.path.isfile(args.path):
        result = check_and_report_result(args.file, args.syntax, args.stop_on_error, args.verbose)
        all_fine = all_fine and result
    elif os.path.isdir(args.path):
        if args.recursive:
            for root, dirs, files in os.walk(args.path):
                for pattern in patterns:
                    filtered = filter(files, pattern)
                    for file in filtered:
                        result = check_and_report_result(os.path.join(root, file), args.syntax, args.stop_on_error,
                                                         args.verbose)
                        all_fine = all_fine and result
    else:
        print(F"{RED}invalid path {args.path}{RESET}")
        sys.exit(1)

    if not all_fine and not args.stop_on_error:
        print(F"{RED}There were errors. Please check the output above.{RESET}")
        sys.exit(1)


if __name__ == "__main__":
    main()
