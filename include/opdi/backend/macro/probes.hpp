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

#include "../../helpers/exceptions.hpp"
#include "../../logic/logicInterface.hpp"
#include "../../tool/toolInterface.hpp"

#include "dataTools.hpp"
#include "implicitBarrierTools.hpp"
#include "probeTools.hpp"
#include "reductionTools.hpp"

namespace opdi {

  struct TaskProbe {
    public:

      void* parallelData;
      void* taskData;
      void* masterPosition;
      bool needsAction;

      TaskProbe() : parallelData(nullptr), taskData(nullptr), needsAction(false) {
        this->masterPosition = tool->allocPosition();
        opdi::tool->getTapePosition(tool->getThreadLocalTape(), this->masterPosition);
      }

      TaskProbe(void* parallelData) : parallelData(parallelData), taskData(nullptr), needsAction(false) {
        this->masterPosition = tool->allocPosition();
        tool->getTapePosition(tool->getThreadLocalTape(), this->masterPosition);
      }

      TaskProbe(TaskProbe const& other) : parallelData(other.parallelData), needsAction(true) {

        this->masterPosition = tool->allocPosition();
        if (omp_get_thread_num() == 0) {
          tool->copyPosition(this->masterPosition, other.masterPosition);
        }
        else {
          tool->getZeroPosition(tool->getThreadLocalTape(), this->masterPosition);
        }

        void* oldTape = tool->getThreadLocalTape();

        void* currentPosition = tool->allocPosition();
        tool->getTapePosition(oldTape, currentPosition);

        DataTools::pushParallelData(this->parallelData);
        this->taskData = logic->onImplicitTaskBegin(omp_get_num_threads(), omp_get_thread_num(), this->parallelData);

        // check if copy statements have been recorded before the correct tape was set
        // if so, move them to the correct tape
        if (tool->comparePosition(currentPosition, masterPosition) > 0) {
          tool->append(tool->getThreadLocalTape(), oldTape, masterPosition, currentPosition);
          tool->erase(oldTape, masterPosition, currentPosition);
        }

        tool->freePosition(currentPosition);

        ProbeScopeStatus::beginImplicitTaskProbeScope();
      }

      ~TaskProbe() {
        if (needsAction) {
          ProbeScopeStatus::endImplicitTaskProbeScope();
          logic->onImplicitTaskEnd(this->taskData);
          DataTools::popParallelData();
        }

        tool->freePosition(this->masterPosition);
      }
  };

  template<LogicInterface::WorksharingKind _kind>
  struct WorkProbe {
    public:

      static LogicInterface::WorksharingKind constexpr kind = _kind;

      bool needsAction;

      WorkProbe(int) : needsAction(false) {}

      WorkProbe() : needsAction(true) {
        logic->onWork(kind, LogicInterface::ScopeEndpoint::Begin);
      }

      ~WorkProbe() {
        if (needsAction) {
          logic->onWork(kind, LogicInterface::ScopeEndpoint::End);
        }
      }
  };

  using LoopProbe = WorkProbe<LogicInterface::WorksharingKind::Loop>;
  using SectionsProbe = WorkProbe<LogicInterface::WorksharingKind::Sections>;
  using SingleProbe = WorkProbe<LogicInterface::WorksharingKind::Single>;

  extern LoopProbe internalLoopProbe;
  extern SectionsProbe internalSectionsProbe;
  extern SingleProbe internalSingleProbe;

  struct ReductionProbe {
    public:

      bool needsAction;

      ReductionProbe(int) : needsAction(false) {}

      ReductionProbe() : needsAction(true) {
        ProbeScopeStatus::beginReductionProbeScope();
        ReductionTools::beginRegionWithReduction();
      }

      ~ReductionProbe() {
        if (needsAction) {
          ReductionTools::addBarrierIfNeeded();
          ReductionTools::endRegionWithReduction();
          ProbeScopeStatus::endReductionProbeScope();
        }
      }
  };

  extern ReductionProbe internalReductionProbe;

  struct NowaitProbe {
    public:

      NowaitProbe(int) {}

      NowaitProbe() {
        ImplicitBarrierTools::nowait();
      }
  };

  extern NowaitProbe internalNowaitProbe;
}
