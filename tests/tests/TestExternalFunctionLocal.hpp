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
#include "codi.hpp"

template<typename _Case>
struct TestExternalFunctionLocal : public TestBase<4, 1, 3, TestExternalFunctionLocal<_Case>> {
  public:
    using Case = _Case;
    using Base = TestBase<4, 1, 3, TestExternalFunctionLocal<Case>>;

    template<typename T>
    static void primal(T const* x, size_t m, T* y, size_t, codi::ExternalFunctionUserData*) {
      for (size_t i = 0; i < m; ++i) {
        y[i] = sin(x[i]);
      }
    }

    template<typename T>
    static void reverse(T const* x, T* x_b, size_t m, T const*, T const* y_b, size_t, codi::ExternalFunctionUserData*) {
      for (size_t i = 0; i < m; ++i) {
        x_b[i] = cos(x[i]) * y_b[i];
      }
    }

    template<typename T>
    static void test(std::array<T, Base::nIn> const& in, std::array<T, Base::nOut>& out) {

      int const N = 1000;

      T* jobResults = new T[N];
      T* intermediate = new T[N];

      OPDI_PARALLEL()
      {
        int nThreads = omp_get_num_threads();
        int start = ((N - 1) / nThreads + 1) * omp_get_thread_num();
        int end = std::min(N, ((N - 1) / nThreads + 1) * (omp_get_thread_num() + 1));

        for (int i = start; i < end; ++i) {
          Base::job1(i, in, jobResults[i]);
        }

        codi::ExternalFunctionHelper<T>* eh = new codi::ExternalFunctionHelper<T>(true);
        for (int i = start; i < end; ++i) {
          eh->addInput(jobResults[i]);
        }

        for (int i = start; i < end; ++i) {
          eh->addOutput(intermediate[i]);
        }

        eh->callPrimalFuncWithADType(TestExternalFunctionLocal::primal<T>, &jobResults[start], end - start,
                                     &intermediate[start], end - start, nullptr);

        eh->addToTape(TestExternalFunctionLocal::reverse);

        for (int i = start; i < end; ++i) {
          jobResults[i] = cos(exp(intermediate[i]));
        }
      }
      OPDI_END_PARALLEL

      for (int i = 0; i < N; ++i) {
        out[0] += jobResults[i];
      }

      delete [] jobResults;
    }
};
