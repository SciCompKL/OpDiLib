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

#include <cstdlib>

namespace opdi {

  struct LogicInterface
  {
    public:

      enum MutexKind {
        Critical, Lock, NestedLock, Ordered, Reduction
      };

      enum ScopeEndpoint {
        Begin, End, BeginEnd
      };

      enum SyncRegionKind {
        Barrier, BarrierImplicit, BarrierExplicit, BarrierImplementation, BarrierReverse
      };

      enum WorksharingKind {
        Loop, Sections, Single
      };

      enum AdjointAccessMode {
        Atomic, Classical
      };

      using WaitId = std::size_t;

      virtual ~LogicInterface() {}

      virtual void* onParallelBegin(void* encounteringTaskData, int maxThreads) = 0;
      virtual void onParallelEnd(void* data) = 0;

      virtual void* onImplicitTaskBegin(bool initialImplicitTask, int actualParallelism, int index,
                                        void* parallelData) = 0;
      virtual void onImplicitTaskEnd(void* data) = 0;

      virtual void onMutexDestroyed(MutexKind kind, WaitId waitId) = 0;
      virtual void onMutexAcquired(MutexKind kind, WaitId waitId) = 0;
      virtual void onMutexReleased(MutexKind kind, WaitId waitId) = 0;
      virtual void registerInactiveMutex(MutexKind kind, WaitId waitId) = 0;

      virtual void onWork(WorksharingKind kind, ScopeEndpoint endpoint) = 0;

      virtual void onMaster(ScopeEndpoint endpoint) = 0;

      virtual void onSyncRegion(SyncRegionKind kind, ScopeEndpoint endpoint) = 0;

      virtual void init() = 0;
      virtual void finalize() = 0;
      virtual void prepareEvaluate() = 0;
      virtual void postEvaluate() = 0;
      virtual void reset() = 0;

      virtual void* exportState() = 0;
      virtual void freeState(void* state) = 0;
      virtual void recoverState(void* state) = 0;

      virtual void setAdjointAccessMode(AdjointAccessMode mode) = 0;
      virtual AdjointAccessMode getAdjointAccessMode() const = 0;

      virtual void resetTask(void* position, AdjointAccessMode mode) = 0;

      virtual void addReverseBarrier() = 0;
      virtual void addReverseFlush() = 0;
  };

  extern LogicInterface* logic;
}
