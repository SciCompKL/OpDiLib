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

#include <list>
#include <memory>

#include "../../logicInterface.hpp"
#include "../implicitTaskOmpLogic.hpp"
#include "../maskedOmpLogic.hpp"
#include "../mutexOmpLogic.hpp"
#include "../parallelOmpLogic.hpp"
#include "../syncRegionOmpLogic.hpp"
#include "../workOmpLogic.hpp"

namespace opdi {

  struct OmpLogicInstrumentInterface {
    public:

      virtual ~OmpLogicInstrumentInterface() {}

      /* instrumentation of forward actions */

      virtual void onParallelBegin(ParallelData* /*data*/) {}
      virtual void onParallelEnd(ParallelData* /*data*/) {}
      virtual void onImplicitTaskBegin(ImplicitTaskData* /*data*/) {}
      virtual void onImplicitTaskEnd(ImplicitTaskData* /*data*/) {}

      virtual void onMutexAcquired(MutexOmpLogic::Data* /*data*/) {}
      virtual void onMutexReleased(MutexOmpLogic::Data* /*data*/) {}
      virtual void onMutexDestroyed(LogicInterface::MutexKind /*kind*/, std::size_t /*waitId*/) {}

      virtual void onSyncRegion(LogicInterface::SyncRegionKind /*kind*/, LogicInterface::ScopeEndpoint /*endpoint*/) {}

      virtual void onMasked(LogicInterface::ScopeEndpoint /*endpoint*/) {}

      virtual void onWork(LogicInterface::WorksharingKind /*kind*/, LogicInterface::ScopeEndpoint /*endpoint*/) {}

      /* instrumentation of reverse actions */

      virtual void reverseParallelBegin(ParallelData* /*data*/) {}
      virtual void reverseParallelEnd(ParallelData* /*data*/) {}
      virtual void reverseImplicitTaskBegin(ImplicitTaskData* /*data*/) {}
      virtual void reverseImplicitTaskEnd(ImplicitTaskData* /*data*/) {}
      virtual void reverseImplicitTaskPart(ImplicitTaskData* /*data*/, std::size_t /*part*/) {}

      virtual void reverseMutexWait(MutexOmpLogic::Data* /*data*/) {}
      virtual void reverseMutexDecrement(MutexOmpLogic::Data* /*data*/) {}

      virtual void reverseSyncRegion(SyncRegionOmpLogic::Data* /*data*/) {}

      virtual void reverseMasked(MaskedOmpLogic::Data* /*data*/) {}

      virtual void reverseWork(WorkOmpLogic::Data* /*data*/) {}

      virtual void reverseFlush() {}

      /* instrumentation of other functionality */

      virtual void onSetAdjointAccessMode(LogicInterface::AdjointAccessMode /*adjointAccess*/) {}
  };

  extern std::list<OmpLogicInstrumentInterface*> ompLogicInstruments;
}
