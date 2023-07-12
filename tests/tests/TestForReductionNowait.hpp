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
struct TestForReductionNowait : public TestBase<4, 1, 3, TestForReductionNowait<_Case>> {
  public:
    using Case = _Case;
    using Base = TestBase<4, 1, 3, TestForReductionNowait<Case>>;

    template<typename T>
    static void test(std::array<T, Base::nIn> const& in, std::array<T, Base::nOut>& out) {

      int const N = 1000;
      T* jobResults1 = new T[N];
      T* jobResults2 = new T[N];

      T output1 = 0.0;
      T output2 = 0.0;

      OPDI_PARALLEL()
      {
        OPDI_FOR(OPDI_REDUCTION reduction(plus: output1) OPDI_NOWAIT)
        for (int i = 0; i < N; ++i) {
          Base::job1(i, in, jobResults1[i]);
          output1 += jobResults1[i];
        }
        OPDI_END_FOR

        OPDI_FOR(OPDI_REDUCTION reduction(plus: output2))
        for (int i = 0; i < N; ++i) {
          Base::job2(i, in, jobResults2[i]);
          output2 += jobResults2[i];
        }
        OPDI_END_FOR
      }
      OPDI_END_PARALLEL

      out[0] = output1 + output2;

      delete [] jobResults1;
      delete [] jobResults2;
    }
};
