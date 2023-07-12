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

#include <array>

#ifndef _OPENMP

int omp_get_num_threads() {
  return 1;
}

int omp_get_thread_num() {
  return 0;
}

using omp_lock_t = int;
using omp_nest_lock_t = int;

void omp_init_lock(int*) {
}

void omp_init_nest_lock(int*) {
}

void omp_destroy_lock(int*) {
}

void omp_destroy_nest_lock(int*) {
}

void omp_set_lock(int*) {
}

void omp_set_nest_lock(int*) {
}

void omp_unset_lock(int*) {
}

void omp_unset_nest_lock(int*) {
}

int omp_test_lock(int*) {
  return 1;
}

int omp_test_nest_lock(int*) {
  return 1;
}

#define INIT_LOCK(lock) omp_init_lock(lock)
#define INIT_NEST_LOCK(lock) omp_init_nest_lock(lock)
#define DESTROY_LOCK(lock) omp_destroy_lock(lock)
#define DESTROY_NEST_LOCK(lock) omp_destroy_nest_lock(lock)
#define SET_LOCK(lock) omp_set_lock(lock)
#define SET_NEST_LOCK(lock) omp_set_nest_lock(lock)
#define UNSET_LOCK(lock) omp_unset_lock(lock)
#define UNSET_NEST_LOCK(lock) omp_unset_nest_lock(lock)
#define TEST_LOCK(lock) omp_test_lock(lock)
#define TEST_NEST_LOCK(lock) omp_test_nest_lock(lock)

#else

#define INIT_LOCK(lock) opdi::opdi_init_lock(lock)
#define INIT_NEST_LOCK(lock) opdi::opdi_init_nest_lock(lock)
#define DESTROY_LOCK(lock) opdi::opdi_destroy_lock(lock)
#define DESTROY_NEST_LOCK(lock) opdi::opdi_destroy_nest_lock(lock)
#define SET_LOCK(lock) opdi::opdi_set_lock(lock)
#define SET_NEST_LOCK(lock) opdi::opdi_set_nest_lock(lock)
#define UNSET_LOCK(lock) opdi::opdi_unset_lock(lock)
#define UNSET_NEST_LOCK(lock) opdi::opdi_unset_nest_lock(lock)
#define TEST_LOCK(lock) opdi::opdi_test_lock(lock)
#define TEST_NEST_LOCK(lock) opdi::opdi_test_nest_lock(lock)

#endif

template<int _nIn, int _nOut, int _nPoints, typename _Test>
struct TestBase {
  public:
    static int const nIn = _nIn;
    static int const nOut = _nOut;
    static int const nPoints = _nPoints;
    using Test = _Test;

    template<typename T>
    static void test(std::array<T, TestBase::nIn> const& in, std::array<T, TestBase::nOut>& out) {
      Test::test(in, out);
    }
};

// specialization that is used in most tests
template<typename _Test>
struct TestBase<4, 1, 3, _Test> {
  public:
    static int const nIn = 4;
    static int const nOut = 1;
    static int const nPoints = 3;
    using Test = _Test;

    template<typename T>
    static std::array<std::array<T, nIn>, nPoints> genPoints() {
      return {{{T(1.1), T(-3.4), T(2.3), T(9.3)}, {T(6.3), T(9.2), T(7.3), T(2.1)}, {T(5.2), T(6.5), T(8.8), T(0.3)}}};
    }

    template<typename T>
    static void job1(int i, std::array<T, nIn> const& in, T& result) {
      T n1 = sin(in[i % 4] * in[(i + 1) % 4] + i);
      T n2 = cos(in[(i + 2) % 4] * in[(i + 3) % 4] * i);
      T n3 = n1 + n1 * n2 - in[i % 4];
      result = sin(n3);
    }

    template<typename T>
    static void job2(int i, std::array<T, nIn> const& in, T& result) {
      T n1 = cos(in[i % 4] * in[(i + 3) % 4] + i);
      T n2 = sin(in[(i + 1) % 4] * in[(i + 2) % 4] * i);
      T n3 = n1 - n1 * n2 + in[(i + 2) % 4];
      result = cos(n3);
    }

    template<typename T>
    static void test(std::array<T, TestBase::nIn> const& in, std::array<T, TestBase::nOut>& out) {
      Test::test(in, out);
    }
};
