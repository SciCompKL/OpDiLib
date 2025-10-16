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
#include <omp.h>

#include "../../backend/backendInterface.hpp"
#include "../../config.hpp"
#include "../../tool/toolInterface.hpp"

#include "instrument/ompLogicInstrumentInterface.hpp"

#include "implicitTaskOmpLogic.hpp"
#include "parallelOmpLogic.hpp"

int opdi::ParallelOmpLogic::skipParallelRegion = 0;

void opdi::ParallelOmpLogic::reverseFunc(void* parallelDataPtr) {

  ParallelData* parallelData = static_cast<ParallelData*>(parallelDataPtr);

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseParallelBegin(parallelData);
    }
  #endif

  ParallelOmpLogic::internalBeginSkippedParallelRegion();

  #pragma omp parallel num_threads(parallelData->actualSizeOfTeam)
  {
    if (parallelData->actualSizeOfTeam != omp_get_num_threads()) {
      OPDI_ERROR("Parallel region in the reverse pass does not use the required number of threads.");
    }

    int threadNum = omp_get_thread_num();

    ImplicitTaskData* implicitTaskData = parallelData->childTaskData[threadNum];

    assert(implicitTaskData->indexInTeam == threadNum);

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->reverseImplicitTaskBegin(implicitTaskData);
      }
    #endif

    void* oldTape = tool->getThreadLocalTape();
    tool->setThreadLocalTape(implicitTaskData->newTape);
    // since the tapes are already set passive when forward implicit tasks finish, there is no need to do that here

    for (size_t j = implicitTaskData->positions.size() - 1; j > 0; --j) {

      #if OPDI_OMP_LOGIC_INSTRUMENT
        for (auto& instrument : ompLogicInstruments) {
          instrument->reverseImplicitTaskPart(implicitTaskData, j);
        }
      #endif

      tool->evaluate(implicitTaskData->newTape,
                     implicitTaskData->positions[j],
                     implicitTaskData->positions[j - 1],
                     implicitTaskData->adjointAccessModes[j - 1] == AdjointAccessMode::Atomic);
    }

    tool->setThreadLocalTape(oldTape);

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->reverseImplicitTaskEnd(implicitTaskData);
      }
    #endif
  }

  ParallelOmpLogic::internalEndSkippedParallelRegion();

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseParallelEnd(parallelData);
    }
  #endif
}

void opdi::ParallelOmpLogic::deleteFunc(void* parallelDataPtr) {

  ParallelData* parallelData = static_cast<ParallelData*>(parallelDataPtr);

  ParallelOmpLogic::internalBeginSkippedParallelRegion();

  // this triggers possibly pending implicit task end events
  #pragma omp parallel num_threads(parallelData->actualSizeOfTeam)
  {
    if (parallelData->actualSizeOfTeam != omp_get_num_threads()) {
      OPDI_WARNING("Parallel region during cleanup does not use the required number of threads.");
    }

    int threadNum = omp_get_thread_num();

    ImplicitTaskData* implicitTaskData = parallelData->childTaskData[threadNum];

    void* oldTape = tool->getThreadLocalTape();
    tool->setThreadLocalTape(implicitTaskData->newTape);

    tool->reset(implicitTaskData->newTape, implicitTaskData->positions[0], OPDI_OMP_LOGIC_CLEAR_ADJOINTS);

    tool->setThreadLocalTape(oldTape);

    // delete data of child tasks
    for (auto const& pos : implicitTaskData->positions) {
      tool->freePosition(pos);
    }
    delete implicitTaskData;
  }

  ParallelOmpLogic::internalEndSkippedParallelRegion();

  tool->freePosition(parallelData->encounteringTaskTapePosition);

  // delete data of the parallel region
  delete parallelData;
}

opdi::LogicInterface::AdjointAccessMode opdi::ParallelOmpLogic::internalGetAdjointAccessMode(
    ImplicitTaskData* implicitTaskData) const {
  return implicitTaskData->adjointAccessModes.back();
}

void opdi::ParallelOmpLogic::internalSetAdjointAccessMode(ImplicitTaskData* implicitTaskData, AdjointAccessMode mode) {

  if (implicitTaskData->isInitialImplicitTask) {
    implicitTaskData->adjointAccessModes.back() = mode;
  }
  else {
    void* position = tool->allocPosition();
    tool->getTapePosition(implicitTaskData->newTape, position);

    if (tool->comparePosition(implicitTaskData->positions.back(), position) == 0) {
      implicitTaskData->adjointAccessModes.back() = mode;
      tool->freePosition(position);
    }
    else {
      implicitTaskData->adjointAccessModes.push_back(mode);
      implicitTaskData->positions.push_back(position);
    }
  }
}

void* opdi::ParallelOmpLogic::onParallelBegin(void* encounteringTaskDataPtr, int maximumSizeOfTeam) {

  if (tool->getThreadLocalTape() != nullptr && ParallelOmpLogic::skipParallelRegion == 0) {

    ImplicitTaskData* encounteringTaskData = static_cast<ImplicitTaskData*>(encounteringTaskDataPtr);

    assert(encounteringTaskData != nullptr);
    assert(encounteringTaskData->isInitialImplicitTask || tool->getThreadLocalTape() == encounteringTaskData->newTape);

    ParallelData* parallelData = new ParallelData;

    parallelData->maximumSizeOfTeam = maximumSizeOfTeam;
    parallelData->isActiveParallelRegion = tool->isActive(tool->getThreadLocalTape());
    parallelData->encounteringTaskData = encounteringTaskData;
    parallelData->encounteringTaskTape = tool->getThreadLocalTape();
    parallelData->encounteringTaskTapePosition = tool->allocPosition();
    tool->getTapePosition(parallelData->encounteringTaskTape, parallelData->encounteringTaskTapePosition);
    parallelData->encounteringTaskAdjointAccessMode = internalGetAdjointAccessMode(encounteringTaskData);
    parallelData->childTaskData.resize(maximumSizeOfTeam);

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onParallelBegin(parallelData);
      }
    #endif

    return static_cast<void*>(parallelData);
  }

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->onParallelBegin(nullptr);
    }
  #endif

  return nullptr;
}

void opdi::ParallelOmpLogic::onParallelEnd(void* parallelDataPtr) {

  if (parallelDataPtr != nullptr) {

    ParallelData* parallelData = static_cast<ParallelData*>(parallelDataPtr);

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onParallelEnd(parallelData);
      }
    #endif

    if (parallelData->isActiveParallelRegion) {

      Handle* handle = new Handle;
      handle->data = static_cast<void*>(parallelData);
      handle->reverseFunc = ParallelOmpLogic::reverseFunc;
      handle->deleteFunc = ParallelOmpLogic::deleteFunc;

      tool->pushExternalFunction(parallelData->encounteringTaskTape, handle);

      // do not delete data, it is deleted with the handle
    }

    // if needed, transport adjoint access mode of thread 0 to encountering task
    ImplicitTaskData* implicitTaskData = parallelData->childTaskData[0];

    if (internalGetAdjointAccessMode(parallelData->encounteringTaskData) != implicitTaskData->adjointAccessModes.back())
      this->internalSetAdjointAccessMode(parallelData->encounteringTaskData,
                                         implicitTaskData->adjointAccessModes.back());

    if (!parallelData->isActiveParallelRegion) {
      deleteFunc(parallelData);
    }
  }
  #if OPDI_OMP_LOGIC_INSTRUMENT
  else {
    for (auto& instrument : ompLogicInstruments) {
      instrument->onParallelEnd(nullptr);
    }
  }
  #endif
}

void opdi::ParallelOmpLogic::setAdjointAccessMode(opdi::LogicInterface::AdjointAccessMode mode) {

  #if OPDI_VARIABLE_ADJOINT_ACCESS_MODE
    void* implicitTaskDataPtr = backend->getTaskData();
    if (implicitTaskDataPtr != nullptr) {  // nullptr if called during tape evaluation
      #if OPDI_OMP_LOGIC_INSTRUMENT
        for (auto& instrument : ompLogicInstruments) {
          instrument->onSetAdjointAccessMode(mode);
        }
      #endif

      internalSetAdjointAccessMode(static_cast<ImplicitTaskData*>(implicitTaskDataPtr), mode);
    }
  #endif
}

opdi::LogicInterface::AdjointAccessMode opdi::ParallelOmpLogic::getAdjointAccessMode() const {
  void* implicitTaskDataPtr = backend->getTaskData();
  if (implicitTaskDataPtr != nullptr) {  // nullptr if called during tape evaluation
    return internalGetAdjointAccessMode(static_cast<ImplicitTaskData*>(implicitTaskDataPtr));
  } else {
    return opdi::ImplicitTaskOmpLogic::defaultAdjointAccessMode;
  }
}

void opdi::ParallelOmpLogic::internalBeginSkippedParallelRegion() {
  ++ParallelOmpLogic::skipParallelRegion;
}

void opdi::ParallelOmpLogic::internalEndSkippedParallelRegion() {
  --ParallelOmpLogic::skipParallelRegion;
}

void opdi::ParallelOmpLogic::beginSkippedParallelRegion() {
  ParallelOmpLogic::internalBeginSkippedParallelRegion();
}

void opdi::ParallelOmpLogic::endSkippedParallelRegion() {
  ParallelOmpLogic::internalEndSkippedParallelRegion();
}
