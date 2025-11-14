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

#include <array>
#include <map>
#include <omp.h>
#include <set>

#include "../logicInterface.hpp"

namespace opdi {

  struct MutexOmpLogic : public virtual LogicInterface {
    public:

      using LogicInterface::MutexKind;
      using LogicInterface::nMutexKind;
      using LogicInterface::WaitId;

      using Counter = std::size_t;

    private:

      // for one kind of mutex, associates corresponding wait ids with counters
      using Counters = std::map<WaitId, Counter>;

      // for one kind of mutex, facilities for recording corresponding mutexes
      struct Recording {
        public:
          Counters counters;
          omp_lock_t lock; // lock for internal synchronization
          WaitId waitId; // wait id of internal lock
          std::set<WaitId> inactive; // ids of inactive mutexes
      };

      std::array<Recording, nMutexKind> recordings;  // recordings for all mutex kinds

      // counters for all mutex kinds
      using AllCounters = std::array<Counters, nMutexKind>;

      // thread-local memory used during recording for data exchange between acquire and release events
      static AllCounters localCounters;
      #pragma omp threadprivate(localCounters)

      // counters used during evaluations
      static AllCounters evaluationCounters;
#ifdef __SANITIZE_THREAD__
      static AllCounters tsanDummies;
#endif

      // currently, OpDiLib's internal state corresponds to the values of all mutex counters
      using State = AllCounters;

    public:

      struct Data {
        public:
          MutexKind mutexKind;
          Counter counter;
          WaitId waitId;
      };

    private:

      void checkKind(MutexKind mutexKind);

      static void waitReverseFunc(void* dataPtr);
      static void decrementReverseFunc(void* dataPtr);
      static void deleteFunc(void* dataPtr);

    protected:

      void internalInit();
      void internalFinalize();

    public:

      virtual void onMutexDestroyed(MutexKind mutexKind, WaitId waitId);
      virtual void onMutexAcquired(MutexKind mutexKind, WaitId waitId);
      virtual void onMutexReleased(MutexKind mutexKind, WaitId waitId);

      // not thread-safe! only use outside parallel regions
      virtual void registerInactiveMutex(MutexKind mutexKind, WaitId waitId);

      void prepareEvaluate();
      void postEvaluate();
      void reset();

      void* exportState();
      void freeState(void* statePtr);
      void recoverState(void* statePtr);
  };
}
