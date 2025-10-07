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

#include "../../helpers/exceptions.hpp"
#include "../../logic/logicInterface.hpp"
#include "../../tool/toolInterface.hpp"

#include "dataTools.hpp"
#include "implicitBarrierTools.hpp"
#include "reductionTools.hpp"

namespace opdi {

  struct TaskProbe {
    public:

      void* parallelData;
      void* taskData;
      bool needsAction;

      TaskProbe() : parallelData(nullptr), taskData(nullptr), needsAction(false) {}

      TaskProbe(void* parallelData) : parallelData(parallelData), taskData(nullptr), needsAction(false) {}

      TaskProbe(TaskProbe const& other) : parallelData(other.parallelData), needsAction(true) {

        DataTools::pushParallelData(this->parallelData);
        this->taskData = logic->onImplicitTaskBegin(false, omp_get_num_threads(), omp_get_thread_num(),
                                                    this->parallelData);
        DataTools::pushTaskData(this->taskData);

        assert(ReductionTools::implicitTaskNestingDepth <= omp_get_level());

        if (ReductionTools::implicitTaskNestingDepth != omp_get_level()) {
          /* TaskProbe constructor before ReductionProbe constructor (if any) */
          do {
            ++ReductionTools::implicitTaskNestingDepth;
          } while (ReductionTools::implicitTaskNestingDepth != omp_get_level());

          ReductionTools::beginRegionThatSupportsReductions(false);
        }
      }

      ~TaskProbe() {
        if (needsAction) {
          assert(ReductionTools::implicitTaskNestingDepth == omp_get_level());

          ReductionTools::endRegionThatSupportsReductions();
          --ReductionTools::implicitTaskNestingDepth;

          logic->onImplicitTaskEnd(this->taskData);
          DataTools::popTaskData();
          DataTools::popParallelData();
        }
      }
  };

  template<LogicInterface::WorksharingKind _kind>
  struct WorkProbe {
    public:

      static LogicInterface::WorksharingKind constexpr kind = _kind;

      bool needsAction;

      WorkProbe(int) : needsAction(false) {}

      WorkProbe() : needsAction(true) {
        #if OPDI_BACKEND_GENERATE_WORK_EVENTS
          logic->onWork(kind, LogicInterface::ScopeEndpoint::Begin);
        #endif
      }

      ~WorkProbe() {
        #if OPDI_BACKEND_GENERATE_WORK_EVENTS
          if (needsAction) {
            logic->onWork(kind, LogicInterface::ScopeEndpoint::End);
          }
        #endif
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

      ReductionProbe(int)  {}

      ReductionProbe() {
        assert(ReductionTools::implicitTaskNestingDepth <= omp_get_level());

        if (ReductionTools::implicitTaskNestingDepth != omp_get_level()) {
          /* this condition can only be satisfied by probes on a parallel construct */
          /* ReductionProbe constructor before TaskProbe constructor */
          do {
            ++ReductionTools::implicitTaskNestingDepth;
          } while (ReductionTools::implicitTaskNestingDepth != omp_get_level());
          ReductionTools::beginRegionThatSupportsReductions(false);
        }

        ReductionTools::regionHasReductions();
      }

      ~ReductionProbe() {}
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
