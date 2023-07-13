/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
 * Homepage: http://www.scicomp.uni-kl.de
 * Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
 *
 * Lead developer: Johannes Blühdorn (SciComp, University of Kaiserslautern-Landau)
 *
 * This file is part of OpDiLib (http://www.scicomp.uni-kl.de/software/opdi).
 *
 * OpDiLib is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * OpDiLib is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with OpDiLib. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <iostream>
#include <omp.h>

namespace opdi {

  struct Output {
    private:

      // print functionality

      template<typename First, typename... Args>
      static void printRec(First const& first, Args const&... args) {
        std::cerr << first << " ";
        Output::printRec(args...);
      }

      template<typename Last>
      static void printRec(Last const& last) {
        std::cerr << last << std::endl;
      }

    public:
      static omp_lock_t lock;

      Output() {}
      virtual ~Output() {}

      static void init() {
        omp_init_lock(&Output::lock);
      }

      static void finalize() {
        omp_destroy_lock(&Output::lock);
      }

      template<typename... Args>
      static void print(Args const&... args) {
        omp_set_lock(&Output::lock);
        Output::printRec(args...);
        omp_unset_lock(&Output::lock);
      }

      template<typename... Args>
      static void printThread(Args const&... args) {
        omp_set_lock(&Output::lock);
        Output::printRec("thread", omp_get_thread_num(), args...);
        omp_unset_lock(&Output::lock);
      }
  };
}
