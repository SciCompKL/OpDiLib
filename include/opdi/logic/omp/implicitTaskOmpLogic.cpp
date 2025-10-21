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

#include <cassert>

#include "../../backend/backendInterface.hpp"
#include "../../config.hpp"
#include "../../helpers/exceptions.hpp"
#include "../../tool/toolInterface.hpp"

#include "instrument/ompLogicInstrumentInterface.hpp"

#include "implicitTaskOmpLogic.hpp"
#include "parallelOmpLogic.hpp"

void opdi::ImplicitTaskOmpLogic::internalInit() {
  this->tapePool.init();
}

void opdi::ImplicitTaskOmpLogic::internalFinalize() {
  this->tapePool.finalize();
}

void* opdi::ImplicitTaskOmpLogic::onImplicitTaskBegin(bool isInitialImplicitTask, int actualSizeOfTeam, int indexInTeam,
                                                      void* parallelDataPtr) {

  ParallelData* parallelData = static_cast<ParallelData*>(parallelDataPtr);

  // check if the handling of the parallel region was skipped
  if (parallelData != nullptr || isInitialImplicitTask) {

    ImplicitTaskData* implicitTaskData = new ImplicitTaskData;
    implicitTaskData->isInitialImplicitTask = isInitialImplicitTask;
    implicitTaskData->level = omp_get_level();
    implicitTaskData->indexInTeam = indexInTeam;

    // OpDiLib does not interfere with the initial implicit task AD-wise, e.g., does not track its tape / does not
    // assume that the tape does not change. OpDiLib uses the initial implicit task's data primarily to track its
    // adjoint access mode.
    if (!isInitialImplicitTask) {

      assert(tool != nullptr);

      if (indexInTeam == 0) {
        if (parallelData->maximumSizeOfTeam < actualSizeOfTeam) {
          OPDI_ERROR("Actual number of threads exceeds maximum number of threads.");
        }
        parallelData->actualSizeOfTeam = actualSizeOfTeam;
      }

      implicitTaskData->oldTape = tool->getThreadLocalTape();
      assert(implicitTaskData->oldTape != nullptr);
      implicitTaskData->parallelData = parallelData;

      void* newTape = this->tapePool.getTape(parallelData->encounteringTaskTape, indexInTeam);

      if (parallelData->isActiveParallelRegion) {
        // most recent tape activity change *per thread* reflects the current activity
        if (indexInTeam == 0) {
          tool->setActive(implicitTaskData->oldTape, false);  // suspend recording on encountering task's tape
        }
        tool->setActive(newTape, true);
      }

      implicitTaskData->newTape = newTape;

      implicitTaskData->positions.push_back(tool->allocPosition());
      tool->getTapePosition(newTape, implicitTaskData->positions.back());

      tool->setThreadLocalTape(newTape);

      implicitTaskData->adjointAccessModes.push_back(parallelData->encounteringTaskAdjointAccessMode);

      parallelData->childTaskData[indexInTeam] = implicitTaskData;

      // check for copies due to firstprivate/copyin that were recorded on the wrong tapes
      // move them to the correct tapes if needed

      void* oldTapePosition = tool->allocPosition();
      tool->getTapePosition(implicitTaskData->oldTape, oldTapePosition);

      void* referencePosition = tool->allocPosition();
      if (indexInTeam == 0) {
        tool->copyPosition(referencePosition, parallelData->encounteringTaskTapePosition);
      }
      else {
        tool->getZeroPosition(implicitTaskData->oldTape, referencePosition);
      }

      if (tool->comparePosition(oldTapePosition, referencePosition) > 0) {
        // users should ensure that activity of default tapes and encountering task's tape match
        assert(parallelData->isActiveParallelRegion);

        tool->append(newTape, implicitTaskData->oldTape, referencePosition, oldTapePosition);
        tool->erase(implicitTaskData->oldTape, referencePosition, oldTapePosition);
      }

      tool->freePosition(referencePosition);
      tool->freePosition(oldTapePosition);
    }
    else {
      implicitTaskData->oldTape = nullptr;
      implicitTaskData->newTape = nullptr;
      implicitTaskData->parallelData = nullptr;

      implicitTaskData->adjointAccessModes.push_back(ImplicitTaskOmpLogic::defaultAdjointAccessMode);
    }

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onImplicitTaskBegin(implicitTaskData);
      }
    #endif

    return static_cast<void*>(implicitTaskData);
  }

  return nullptr;
}

void opdi::ImplicitTaskOmpLogic::onImplicitTaskEnd(void* implicitTaskDataPtr) {

  if (implicitTaskDataPtr != nullptr) {

    ImplicitTaskData* implicitTaskData = static_cast<ImplicitTaskData*>(implicitTaskDataPtr);

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onImplicitTaskEnd(implicitTaskData);
      }
    #endif

    if (!implicitTaskData->isInitialImplicitTask) {
      assert(tool != nullptr);

      tool->setThreadLocalTape(implicitTaskData->oldTape);

      implicitTaskData->positions.push_back(tool->allocPosition());
      tool->getTapePosition(implicitTaskData->newTape, implicitTaskData->positions.back());

      if (!implicitTaskData->parallelData->isActiveParallelRegion) {
        if (tool->comparePosition(implicitTaskData->positions.front(), implicitTaskData->positions.back()) != 0) {
          OPDI_ERROR("Something became active during a passive parallel region. This is not supported and will not be",
                     "differentiated correctly.");
        }
      }
      else {
        // most recent tape activity change *per thread* reflects the current activity
        tool->setActive(implicitTaskData->newTape, false);
        if (implicitTaskData->indexInTeam == 0) {
          tool->setActive(implicitTaskData->oldTape, true);  // resume recording on encountering task's tape
        }
      }

      // do not delete data, it is deleted as part of parallel regions
    }
    else {
      // delete task data, there is no parallel region to do so
      delete implicitTaskData;
    }
  }
}

void opdi::ImplicitTaskOmpLogic::resetImplicitTask(void* position, opdi::LogicInterface::AdjointAccessMode mode) {

  void* implicitTaskDataPtr = backend->getImplicitTaskData();

  if (implicitTaskDataPtr != nullptr) {
    ImplicitTaskData* implicitTaskData = static_cast<ImplicitTaskData*>(implicitTaskDataPtr);

    if (!implicitTaskData->isInitialImplicitTask) {
      assert(tool != nullptr);

      assert(tool->comparePosition(implicitTaskData->positions.front(), position) <= 0);

      while (tool->comparePosition(implicitTaskData->positions.back(), position) > 0) {
        implicitTaskData->positions.pop_back();
        implicitTaskData->adjointAccessModes.pop_back();
      }
    }

    implicitTaskData->adjointAccessModes.back() = mode;
  }
}
