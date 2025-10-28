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

#ifdef __SANITIZE_THREAD__
  #define ANNOTATE_RWLOCK_CREATE(lock) AnnotateRWLockCreate(__FILE__, __LINE__, (void*)lock)
  #define ANNOTATE_RWLOCK_DESTROY(lock) AnnotateRWLockDestroy(__FILE__, __LINE__, (void*)lock)
  #define ANNOTATE_RWLOCK_ACQUIRED(lock, isWrite) AnnotateRWLockAcquired(__FILE__, __LINE__, (void*)lock, isWrite)
  #define ANNOTATE_RWLOCK_RELEASED(lock, isWrite) AnnotateRWLockReleased(__FILE__, __LINE__, (void*)lock, isWrite)

extern "C" void AnnotateRWLockCreate(const char* f, int l, void* addr);
extern "C" void AnnotateRWLockDestroy(const char* f, int l, void* addr);
extern "C" void AnnotateRWLockAcquired(const char* f, int l, void* addr, size_t isWrite);
extern "C" void AnnotateRWLockReleased(const char* f, int l, void* addr, size_t isWrite);
#endif

template<typename _Case>
struct TestCustomMutex : public TestBase<4, 1, 3, TestCustomMutex<_Case>> {
  public:
    using Case = _Case;
    using Base = TestBase<4, 1, 3, TestCustomMutex<Case>>;

    struct CustomMutex {
      private:
        int status;

        #ifdef __SANITIZE_THREAD__
          int dummy;
        #endif

      public:
        CustomMutex() : status(0)
          #ifdef __SANITIZE_THREAD__
                      , dummy(0)
          #endif
        {

          #ifdef __SANITIZE_THREAD__
            ANNOTATE_RWLOCK_CREATE(&dummy);
          #endif
        }

        ~CustomMutex() {
          #ifdef __SANITIZE_THREAD__
            ANNOTATE_RWLOCK_DESTROY(&dummy);
          #endif
        }

        void lock() {
          int localStatus;

          while (true) {
            // wait until unlocked
            do {
              #ifdef _OPENMP
                #pragma omp atomic read
              #endif
              localStatus = status;
            } while (localStatus != 0);

            // try to lock
            #ifdef _OPENMP
              #pragma omp atomic capture
            #endif
            localStatus = ++status;

            // success
            if (localStatus == 1) {
              break;
            }

            // already acquired, try again
            #ifdef _OPENMP
              #pragma omp atomic update
            #endif
            --status;
          }

          #ifdef __SANITIZE_THREAD__
            ANNOTATE_RWLOCK_ACQUIRED(&dummy, true);
          #endif

          #ifdef _OPENMP
            opdi::logic->onMutexAcquired(opdi::LogicInterface::MutexKind::Custom, reinterpret_cast<opdi::LogicInterface::WaitId>(&status));
          #endif
        }

        void unlock() {
          #ifdef __SANITIZE_THREAD__
            ANNOTATE_RWLOCK_RELEASED(&dummy, true);
          #endif

          #ifdef _OPENMP
            #pragma omp atomic update
          #endif
          --status;

          #ifdef _OPENMP
            opdi::logic->onMutexReleased(opdi::LogicInterface::MutexKind::Custom, reinterpret_cast<opdi::LogicInterface::WaitId>(&status));
          #endif
        }
    };

    template<typename T>
    static void test(std::array<T, Base::nIn> const& in, std::array<T, Base::nOut>& out) {

      int const N = 100;
      T* jobResults = new T[N];
      T out1 = 0.0;
      T out2 = 0.0;

      CustomMutex mutex1, mutex2;

      OPDI_PARALLEL()
      {
        int nThreads = omp_get_num_threads();
        int start = ((N - 1) / nThreads + 1) * omp_get_thread_num();
        int end = std::min(N, ((N - 1) / nThreads + 1) * (omp_get_thread_num() + 1));

        for (int i = start; i < end; ++i) {
          Base::job1(i, in, jobResults[i]);

          mutex1.lock();
          out1 += jobResults[i];
          mutex1.unlock();

          Base::job2(i, in, jobResults[i]);

          mutex2.lock();
          out2 *= jobResults[i];
          mutex2.unlock();
        }
      }
      OPDI_END_PARALLEL

      out[0] = out1 + out2;

      delete [] jobResults;
    }
};
