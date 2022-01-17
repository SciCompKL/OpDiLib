/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Homepage: http://www.scicomp.uni-kl.de
 * Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
 *
 * Lead developer: Johannes Blühdorn (SciComp, TU Kaiserslautern)
 *
 * This file is part of OpDiLib (http://www.scicomp.uni-kl.de/software/opdi).
 *
 * OpDiLib is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OpDiLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU
 * General Public License along with OpDiLib.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * For other licensing options please contact us.
 *
 */

#pragma once

#include <list>
#include <memory>

#include "../../logicInterface.hpp"
#include "../implicitTaskOmpLogic.hpp"
#include "../mutexOmpLogic.hpp"
#include "../parallelOmpLogic.hpp"
#include "../syncRegionOmpLogic.hpp"
#include "../workOmpLogic.hpp"

namespace opdi {

  struct OmpLogicInstrumentInterface {
    public:

      virtual ~OmpLogicInstrumentInterface() {}

      virtual void reverseFlush() {}

      virtual void onImplicitTaskBegin(ImplicitTaskOmpLogic::Data* /*data*/) {}
      virtual void onImplicitTaskEnd(ImplicitTaskOmpLogic::Data* /*data*/) {}

      virtual void reverseMutexWait(MutexOmpLogic::Data* /*data*/) {}
      virtual void reverseMutexDecrement(MutexOmpLogic::Data* /*data*/) {}
      virtual void onMutexDestroyed(LogicInterface::MutexKind /*kind*/, std::size_t /*waitId*/) {}
      virtual void onMutexAcquired(MutexOmpLogic::Data* /*data*/) {}
      virtual void onMutexReleased(MutexOmpLogic::Data* /*data*/) {}

      virtual void reverseParallel(ParallelOmpLogic::Data* /*data*/) {}
      virtual void reverseParallelPart(ParallelOmpLogic::Data* /*data*/, std::size_t /*j*/) {}
      virtual void onParallelBegin(ParallelOmpLogic::Data* /*data*/) {}
      virtual void onParallelEnd(ParallelOmpLogic::Data* /*data*/) {}

      virtual void reverseSyncRegion(SyncRegionOmpLogic::Data* /*data*/) {}
      virtual void onSyncRegion(LogicInterface::SyncRegionKind /*kind*/, LogicInterface::ScopeEndpoint /*endpoint*/) {}

      virtual void reverseWork(WorkOmpLogic::Data* /*data*/) {}
      virtual void onWork(LogicInterface::WorksharingKind /*kind*/, LogicInterface::ScopeEndpoint /*endpoint*/) {}
  };

  extern std::list<OmpLogicInstrumentInterface*> ompLogicInstruments;
}
