/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2024 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
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
struct TestAdjointAccessControlNested2 : public TestBase<4, 1, 3, TestAdjointAccessControlNested2<_Case>> {
  public:
    using Case = _Case;
    using Base = TestBase<4, 1, 3, TestAdjointAccessControlNested2<Case>>;

    template<typename T>
    static void test(std::array<T, Base::nIn> const& in, std::array<T, Base::nOut>& out) {

      int const N = 100;
      T* a = new T[N];
      T* b = new T[N];
      T* c = new T[N];

      // note: these assertions are disabled for Primal and FirstOrderForward
      assertAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Atomic);

      OPDI_PARALLEL()
      {
        assertAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Atomic);

        int outerNThreads = omp_get_num_threads();
        int outerStart = ((N - 1) / outerNThreads + 1) * omp_get_thread_num();
        int outerEnd = std::min(N, ((N - 1) / outerNThreads + 1) * (omp_get_thread_num() + 1));

        // shared reading of in
        for (int i = outerStart; i < outerEnd; ++i) {
          Base::job1(i, in, a[i]);
        }

        #if _OPENMP
          opdi::logic->setAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Classical);
        #endif

        assertAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Classical);

        // no shared reading
        for (int i = outerStart; i < outerEnd; ++i) {
          b[i] = sin(exp(a[i]));
        }

        OPDI_BARRIER()

        OPDI_PARALLEL()
        {
          assertAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Classical);

          int innerNThreads = omp_get_num_threads();
          int innerStart = outerStart + (((outerEnd - outerStart) - 1) / innerNThreads + 1) * omp_get_thread_num();
          int innerEnd = std::min(outerEnd, outerStart + (((outerEnd - outerStart) - 1) / innerNThreads + 1)
                                                                                     * (omp_get_thread_num() + 1));

          #if _OPENMP
            opdi::logic->setAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Atomic);
          #endif

          assertAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Atomic);

          // shared reading on a
          for (int j = innerStart; j < innerEnd; ++j) {
            T arg = b[j];
            for (int k = 0; k < 10; ++k) {
              arg += a[(j + k) % N];
            }
            c[j] = cos(arg);
          }

          // atomic adjoint access mode to be transported out of the nested parallel region
        }
        OPDI_END_PARALLEL

        assertAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Atomic);

        // shared reading on a
        for (int i = outerStart; i < outerEnd; ++i) {
          T arg = c[i];
          for (int k = 0; k < 10; ++k) {
            arg += sin(exp(a[(i + k) % N]));
          }
          c[i] += arg;
        }

        #if _OPENMP
          opdi::logic->setAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Classical);
          opdi::logic->addReverseBarrier();
        #endif

        assertAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Classical);

        // no shared reading
        for (int i = outerStart; i < outerEnd; ++i) {
          c[i] += sin(exp(c[i]));
        }

        #if _OPENMP
          opdi::logic->setAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Atomic);
          opdi::logic->addReverseBarrier();
        #endif

        assertAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Atomic);

        // shared reading on a
        for (int i = outerStart; i < outerEnd; ++i) {
          T arg = c[i];
          for (int j = 0; j < 10; ++j) {
            arg += a[(i + j) % N];
          }
          c[i] = cos(arg);
        }

      }
      OPDI_END_PARALLEL

      assertAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode::Atomic);

      for (int i = 0; i < N; ++i) {
        out[0] += c[i];
      }

      delete [] a;
      delete [] b;
      delete [] c;
    }
};
