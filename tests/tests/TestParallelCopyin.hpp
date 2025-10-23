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
struct TestParallelCopyin : public TestBase<4, 1, 3, TestParallelCopyin<_Case>> {
  public:
    using Case = _Case;
    using Base = TestBase<4, 1, 3, TestParallelCopyin<Case>>;

    template<typename T>
    static void test(std::array<T, Base::nIn> const& in, std::array<T, Base::nOut>& out) {

      /* gcc refuses to compile this, so copyin is actually not tested with gcc right now */
      static T helper;
      #if !defined(__GNUC__) || defined(__clang__)
        #if _OPENMP
          #pragma omp threadprivate(helper)
        #endif
      #endif

      int const N = 1000;
      T* jobResults = new T[N];

      #ifndef BUILD_REFERENCE
        /* Set activity of default tapes. They might record copy operations due to firstprivate. OpDiLib's AD approach
         * for parallel regions moves these recordings to the correct tapes. */
        if (opdi::tool->isActive(opdi::tool->getThreadLocalTape())) {
          opdi::logic->beginSkippedParallelRegion();
          OPDI_PARALLEL()
          {
            /* due to skipping, OpDiLib has not exchanged the tapes */
            if (omp_get_thread_num() != 0) {
              opdi::tool->setActive(opdi::tool->getThreadLocalTape(), true);
            }
          }
          OPDI_END_PARALLEL
          opdi::logic->endSkippedParallelRegion();
        }
      #endif

      helper = in[0] * in[1] * in[2] * in[3];

      #if !defined(__GNUC__) || defined(__clang__)
        OPDI_PARALLEL(copyin(helper))
      #else
        // opdi-syntax-off
        OPDI_PARALLEL(firstprivate(helper))
        // opdi-syntax-on
      #endif
      {
        int nThreads = omp_get_num_threads();
        int start = ((N - 1) / nThreads + 1) * omp_get_thread_num();
        int end = std::min(N, ((N - 1) / nThreads + 1) * (omp_get_thread_num() + 1));

        for (int i = start; i < end; ++i) {
          Base::job1(i, in, jobResults[i]);
          jobResults[i] = sin(jobResults[i] * helper);
        }
      }
      OPDI_END_PARALLEL

      for (int i = 0; i < N; ++i) {
        out[0] += jobResults[i];
      }

      delete [] jobResults;
    }
};
