/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2026 Chair for Scientific Computing (SciComp), RPTU University Kaiserslautern-Landau
 * Homepage: https://scicomp.rptu.de
 * Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
 *
 * Lead developer: Johannes Blühdorn (SciComp, RPTU University Kaiserslautern-Landau)
 *
 * This file is part of OpDiLib (https://scicomp.rptu.de/software/opdi).
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

namespace opdi {

  /* lock routines */

  void opdi_destroy_lock(omp_lock_t* lock) {
    omp_destroy_lock(lock);
  }

  void opdi_destroy_nest_lock(omp_nest_lock_t* lock) {
    omp_destroy_nest_lock(lock);
  }

  void opdi_set_lock(omp_lock_t* lock) {
    omp_set_lock(lock);
  }

  void opdi_set_nest_lock(omp_nest_lock_t* lock) {
    omp_set_nest_lock(lock);
  }

  void opdi_unset_lock(omp_lock_t* lock) {
    omp_unset_lock(lock);
  }

  void opdi_unset_nest_lock(omp_nest_lock_t* lock) {
    omp_unset_nest_lock(lock);
  }

  int opdi_test_lock(omp_lock_t* lock) {
    return omp_test_lock(lock);
  }

  int opdi_test_nest_lock(omp_nest_lock_t* lock) {
    return omp_test_nest_lock(lock);
  }
}
