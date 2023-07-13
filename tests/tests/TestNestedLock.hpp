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

#include "testBase.hpp"

template<typename _Case>
struct TestNestedLock : public TestBase<4, 1, 3, TestNestedLock<_Case>> {
  public:
    using Case = _Case;
    using Base = TestBase<4, 1, 3, TestNestedLock<Case>>;

    template<typename T>
    static void job(int i, std::array<T, Base::nIn> const& in, T& result1, T& result2,
                    omp_nest_lock_t* lock1, omp_nest_lock_t* lock2) {

      T n1 = sin(in[i % 4] * in[(i + 1) % 4] + i);
      T n2 = cos(in[(i + 2) % 4] * in[(i + 3) % 4] * i);
      T n3 = n1 + n1 * n2 - in[i % 4];

      if (i % 4 == 0) {
        SET_NEST_LOCK(lock1);
        result1 += sin(n3);
        UNSET_NEST_LOCK(lock1);

        SET_NEST_LOCK(lock2);
        result2 += cos(n3);
        UNSET_NEST_LOCK(lock2);
      }
      else if (i % 4 == 1) {
        while (!TEST_NEST_LOCK(lock1));
        result1 += sin(n3);
        UNSET_NEST_LOCK(lock1);

        while (!TEST_NEST_LOCK(lock2));
        result2 += cos(n3);
        UNSET_NEST_LOCK(lock2);
      }
      else if (i % 4 == 2) {
        SET_NEST_LOCK(lock1);
        SET_NEST_LOCK(lock1);
        result1 += sin(n3);
        UNSET_NEST_LOCK(lock1);
        UNSET_NEST_LOCK(lock1);

        SET_NEST_LOCK(lock2);
        SET_NEST_LOCK(lock2);
        result2 += cos(n3);
        UNSET_NEST_LOCK(lock2);
        UNSET_NEST_LOCK(lock2);
      }
      else {
        SET_NEST_LOCK(lock2);
        SET_NEST_LOCK(lock2);
        result2 += cos(n3);
        UNSET_NEST_LOCK(lock2);
        UNSET_NEST_LOCK(lock2);

        SET_NEST_LOCK(lock1);
        SET_NEST_LOCK(lock1);
        result1 += sin(n3);
        UNSET_NEST_LOCK(lock1);
        UNSET_NEST_LOCK(lock1);
      }
    }

    template<typename T>
    static void test(std::array<T, Base::nIn> const& in, std::array<T, Base::nOut>& out) {

      int const N = 1000;
      T* jobResults = new T[N];
      T out1 = 0.0;
      T out2 = 0.0;
      omp_nest_lock_t lock1, lock2;
      INIT_NEST_LOCK(&lock1);
      INIT_NEST_LOCK(&lock2);

      OPDI_PARALLEL()
      {
        int nThreads = omp_get_num_threads();
        int start = ((N - 1) / nThreads + 1) * omp_get_thread_num();
        int end = std::min(N, ((N - 1) / nThreads + 1) * (omp_get_thread_num() + 1));

        for (int i = start; i < end; ++i) {
          TestNestedLock::job(i, in, out1, out2, &lock1, &lock2);
        }
      }
      OPDI_END_PARALLEL

      out[0] = out1 + out2;

      DESTROY_NEST_LOCK(&lock1);
      DESTROY_NEST_LOCK(&lock2);

      delete [] jobResults;
    }
};
