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

#include "runtime.hpp"

namespace opdi {

  /* execution environment functions */

  void opdi_set_num_threads(int numThreads) {
    omp_set_num_threads(numThreads);
  }

  int opdi_get_num_threads() {
    return omp_get_num_threads();
  }

  int opdi_get_max_threads() {
    return omp_get_max_threads();
  }

  int opdi_get_thread_num() {
    return omp_get_thread_num();
  }

  int opdi_get_num_procs() {
    return omp_get_num_procs();
  }

  int opdi_in_parallel() {
    return omp_in_parallel();
  }

  void opdi_set_dynamic(int dynamicThreads) {
    omp_set_dynamic(dynamicThreads);
  }

  int opdi_get_dynamic() {
    return omp_get_dynamic();
  }

  void opdi_set_nested(int nested) {
    omp_set_nested(nested);
  }

  int opdi_get_nested() {
    return omp_get_nested();
  }

  /* lock routines */

  void opdi_init_lock(omp_lock_t* lock) {
    omp_init_lock(lock);
  }

  void opdi_init_nest_lock(omp_nest_lock_t* lock) {
    omp_init_nest_lock(lock);
  }

#if _OPENMP >= 201511 && __clang__
  void opdi_init_lock_with_hint(omp_lock_t *lock, omp_sync_hint_t hint) {
    omp_init_lock_with_hint(lock, hint);
  }

  void opdi_init_nest_lock_with_hint(omp_nest_lock_t* lock, omp_sync_hint_t hint) {
    omp_init_nest_lock_with_hint(lock, hint);
  }
#endif

  /* timing routines */

  double opdi_get_wtime() {
    return omp_get_wtime();
  }

  double opdi_get_wtick() {
    return omp_get_wtick();
  }
}

