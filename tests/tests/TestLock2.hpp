/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2025 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
 * Homepage: https://scicomp.rptu.de
 * Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
 *
 * Lead developer: Johannes Blühdorn (SciComp, University of Kaiserslautern-Landau)
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

#pragma once

#include "testBase.hpp"

template<typename _Case>
struct TestLock2 : public TestBase<4, 1, 3, TestLock2<_Case>> {
  public:
    using Case = _Case;
    using Base = TestBase<4, 1, 3, TestLock2<Case>>;

    template<typename T>
    static void test(std::array<T, Base::nIn> const& in, std::array<T, Base::nOut>& out) {

      int const N = 100;
      T* jobResults = new T[N];
      omp_lock_t lock1, lock2;
      INIT_LOCK(&lock1);
      INIT_LOCK(&lock2);

      SET_LOCK(&lock1);

      OPDI_PARALLEL()
      {
        int nThreads = omp_get_num_threads();
        int start = ((N - 1) / nThreads + 1) * omp_get_thread_num();
        int end = std::min(N, ((N - 1) / nThreads + 1) * (omp_get_thread_num() + 1));

        if (omp_get_thread_num() != 0) {
          SET_LOCK(&lock1);
        }

        for (int i = start; i < end; ++i) {
          Base::job1(i, in, jobResults[i]);
          out[0] += jobResults[i];
        }
        UNSET_LOCK(&lock1);

        OPDI_BARRIER()

        if (omp_get_thread_num() == 0) {
          SET_LOCK(&lock1);
          SET_LOCK(&lock2);
        }
      }
      OPDI_END_PARALLEL

      UNSET_LOCK(&lock2);

      OPDI_PARALLEL()
      {
        int nThreads = omp_get_num_threads();
        int start = ((N - 1) / nThreads + 1) * omp_get_thread_num();
        int end = std::min(N, ((N - 1) / nThreads + 1) * (omp_get_thread_num() + 1));

        if (omp_get_thread_num() != 0) {
          SET_LOCK(&lock1);
        }

        for (int i = start; i < end; ++i) {
          Base::job2(i, in, jobResults[i]);
          out[0] += jobResults[i];
        }
        UNSET_LOCK(&lock1);
      }
      OPDI_END_PARALLEL

      DESTROY_LOCK(&lock1);
      DESTROY_LOCK(&lock2);

      delete [] jobResults;
    }
};
