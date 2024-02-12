﻿/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2024 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
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
struct TestPreaccumulationGlobal : public TestBase<4, 1, 3, TestPreaccumulationGlobal<_Case>> {
  public:
    using Case = _Case;
    using Base = TestBase<4, 1, 3, TestPreaccumulationGlobal<Case>>;

    template<typename T>
    static void test(std::array<T, Base::nIn> const& in, std::array<T, Base::nOut>& out) {

      int const N = 1000;
      T* jobResults = new T[N];

      std::array<T, Base::nIn> prep, post;
      codi::PreaccumulationHelper<T> ph;

      for (int i = 0; i < Base::nIn; ++i) {
        prep[i] = sin(in[i]) + cos(in[i]);
      }

      ph.start();
      for (int i = 0; i < Base::nIn; ++i) {
        ph.addInput(prep[i]);
      }

      OPDI_PARALLEL()
      {
        int nThread = omp_get_thread_num();
        int nThreads = omp_get_num_threads();
        int start = ((N - 1) / nThreads + 1) * nThread;
        int end = std::min(N, ((N - 1) / nThreads + 1) * (nThread + 1));

        for (int j = start; j < end; ++j) {
          Base::job1(j, prep, jobResults[j]);
        }
      }
      OPDI_END_PARALLEL

      for (int i = 0; i < Base::nIn; ++i) {
        post[i] = 0.0;
        for (int j = 0; j < N; ++j) {
          post[i] += cos(i * jobResults[j]);
        }
      }

      for (int i = 0; i < Base::nIn; ++i) {
        ph.addOutput(post[i]);
      }

      ph.finish(false);

      for (int i = 0; i < Base::nIn; ++i) {
        out[0] += post[i];
      }

      delete [] jobResults;
    }
};
