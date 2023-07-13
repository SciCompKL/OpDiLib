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

#include <map>
#include <omp.h>
#include <set>

#include "../logicInterface.hpp"

namespace opdi {

  struct MutexOmpLogic : public virtual LogicInterface {
    public:

      using LogicInterface::MutexKind;

    private:

      using Trace = std::map<std::size_t, std::size_t>;

      struct MutexTrace {
        public:
          Trace trace;
          omp_lock_t lock; // lock for internal synchronization
          std::size_t waitId; // wait id of internal lock
          std::set<std::size_t> inactive; // ids of inactive mutexes
      };

      MutexTrace criticalTrace, lockTrace, nestedLockTrace, orderedTrace, reductionTrace;

      struct State {
        public:
          Trace criticalTrace;
          Trace lockTrace;
          Trace nestedLockTrace;
          Trace orderedTrace;
          Trace reductionTrace;
      };

      static State localState;
      #pragma omp threadprivate(localState)

      static State evalState;

    public:

      struct Data {
        public:
          MutexKind kind;
          std::size_t traceValue;
          std::size_t waitId;
      };

    private:

      static void internalWaitReverseFunc(std::map<std::size_t, std::size_t>& evalTrace, void* dataPtr);

      static void waitCriticalReverseFunc(void* dataPtr);
      static void waitLockReverseFunc(void* dataPtr);
      static void waitNestedLockReverseFunc(void* dataPtr);
      static void waitOrderedReverseFunc(void* dataPtr);
      static void waitReductionReverseFunc(void* dataPtr);

      static void waitDeleteFunc(void* dataPtr);

      static void internalDecrementReverseFunc(std::map<std::size_t, std::size_t>& evalTrace, void* dataPtr);

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

      void internalOnMutexAcquired(MutexKind kind, MutexTrace& mutexTrace,
                                   std::map<std::size_t, std::size_t>& localTrace,
                                   void (*decrementReverseFunc)(void*),
                                   std::size_t waitId);
      void internalOnMutexReleased(MutexKind kind, MutexTrace& mutexTrace,
                                   std::map<std::size_t, std::size_t>& localTrace,
                                   void (*waitReverseFunc)(void*),
                                   std::size_t waitId);

    public:

      virtual void onMutexDestroyed(MutexKind kind, std::size_t waitId);
      virtual void onMutexAcquired(MutexKind kind, std::size_t waitId);
      virtual void onMutexReleased(MutexKind kind, std::size_t waitId);

      // not thread-safe! only use outside parallel regions
      virtual void registerInactiveMutex(MutexKind kind, std::size_t waitId);

      void prepareEvaluate();
      void reset();

      void* exportState();
      void freeState(void* statePtr);
      void recoverState(void* statePtr);
  };
}
