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

#include <map>
#include <omp.h>
#include <set>

#include "../logicInterface.hpp"

namespace opdi {

  struct MutexOmpLogic : public virtual LogicInterface {
    public:

      using LogicInterface::MutexKind;
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

      Recording criticalRecording, lockRecording, nestedLockRecording, orderedRecording, reductionRecording;

      struct State {
        public:
          Counters criticalCounters;
          Counters lockCounters;
          Counters nestedLockCounters;
          Counters orderedCounters;
          Counters reductionCounters;
      };

      static State localState;
      #pragma omp threadprivate(localState)

      static State evalState;
#ifdef __SANITIZE_THREAD__
      static State tsanDummies;
#endif

    public:

      struct Data {
        public:
          MutexKind kind;
          Counter counter;
          WaitId waitId;
      };

    private:

      static void internalWaitReverseFunc(Counters& counters,
                                          #ifdef __SANITIZE_THREAD__
                                            Counters& tsanDummies,
                                          #endif
                                          void* dataPtr);

      static void waitCriticalReverseFunc(void* dataPtr);
      static void waitLockReverseFunc(void* dataPtr);
      static void waitNestedLockReverseFunc(void* dataPtr);
      static void waitOrderedReverseFunc(void* dataPtr);
      static void waitReductionReverseFunc(void* dataPtr);

      static void waitDeleteFunc(void* dataPtr);

      static void internalDecrementReverseFunc(Counters& counters,
                                               #ifdef __SANITIZE_THREAD__
                                                 Counters& tsanDummies,
                                               #endif
                                               void* dataPtr);

      static void decrementCriticalReverseFunc(void* dataPtr);
      static void decrementLockReverseFunc(void* dataPtr);
      static void decrementNestedLockReverseFunc(void* dataPtr) ;
      static void decrementOrderedReverseFunc(void* dataPtr);
      static void decrementReductionReverseFunc(void* dataPtr);

      static void decrementDeleteFunc(void* dataPtr);

    protected:

      void internalInit();
      void internalFinalize();

    private:

      void internalOnMutexAcquired(MutexKind kind, Recording& recording,
                                   Counters& localCounters,
                                   void (*decrementReverseFunc)(void*),
                                   WaitId waitId);
      void internalOnMutexReleased(MutexKind kind, Recording& recording,
                                   Counters& localCounters,
                                   void (*waitReverseFunc)(void*),
                                   WaitId waitId);

    public:

      virtual void onMutexDestroyed(MutexKind kind, WaitId waitId);
      virtual void onMutexAcquired(MutexKind kind, WaitId waitId);
      virtual void onMutexReleased(MutexKind kind, WaitId waitId);

      // not thread-safe! only use outside parallel regions
      virtual void registerInactiveMutex(MutexKind kind, WaitId waitId);

      void prepareEvaluate();
      void postEvaluate();
      void reset();

      void* exportState();
      void freeState(void* statePtr);
      void recoverState(void* statePtr);
  };
}
