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
struct TestStateExport : public TestBase<4, 1, 3, TestStateExport<_Case>> {
  public:
    using Case = _Case;
    using Base = TestBase<4, 1, 3, TestStateExport<Case>>;

    template<typename T>
    static void test(std::array<T, Base::nIn> const& in, std::array<T, Base::nOut>& out) {

      int const N = 1000;
      T* jobResults = new T[N];
      omp_lock_t lock1, lock2;
      INIT_LOCK(&lock1);
      INIT_LOCK(&lock2);

      OPDI_PARALLEL()
      {
        int nThreads = omp_get_num_threads();
        int start = ((N - 1) / nThreads + 1) * omp_get_thread_num();
        int end = std::min(N, ((N - 1) / nThreads + 1) * (omp_get_thread_num() + 1));

        for (int i = start; i < end; ++i) {
          Base::job1(i, in, jobResults[i]);
          SET_LOCK(&lock1);
          out[0] += jobResults[i];
          UNSET_LOCK(&lock1);
        }
      }
      OPDI_END_PARALLEL

      #ifdef _OPENMP
        auto opdiState = opdi::logic->exportState();
      #endif
      auto tapePosition = T::getTape().getPosition();

      OPDI_PARALLEL()
      {
        int nThreads = omp_get_num_threads();
        int start = ((N - 1) / nThreads + 1) * omp_get_thread_num();
        int end = std::min(N, ((N - 1) / nThreads + 1) * (omp_get_thread_num() + 1));

        for (int i = start; i < end; ++i) {
          Base::job2(i, in, jobResults[i]);
          SET_LOCK(&lock2);
          out[0] += jobResults[i];
          UNSET_LOCK(&lock2);
        }
      }
      OPDI_END_PARALLEL

      bool wasActive = T::getTape().isActive();

      if (wasActive) {
        T::getTape().setPassive();
      }

      T::getTape().resetTo(tapePosition);
      #ifdef _OPENMP
        opdi::logic->recoverState(opdiState);
        opdi::logic->freeState(opdiState);
      #endif

      if (wasActive) {
        T::getTape().setActive();
      }

      DESTROY_LOCK(&lock1);
      DESTROY_LOCK(&lock2);

      delete [] jobResults;
    }
};
