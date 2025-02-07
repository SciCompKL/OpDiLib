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

int opdi::ParallelOmpLogic::skipParallelHandling = 0;

void opdi::ParallelOmpLogic::reverseFunc(void* dataPtr) {

  Data* data = (Data*) dataPtr;

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseParallelBegin(data);
    }
  #endif

  ++ParallelOmpLogic::skipParallelHandling;

  #pragma omp parallel num_threads(data->actualThreads)
  {
    int threadNum = omp_get_thread_num();

    ImplicitTaskOmpLogic::Data* taskData = reinterpret_cast<ImplicitTaskOmpLogic::Data*>(data->childTasks[threadNum]);

    assert(taskData->index == threadNum);

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->reverseImplicitTaskBegin(taskData);
      }
    #endif

    void* oldTape = tool->getThreadLocalTape();
    tool->setThreadLocalTape(taskData->tape);
    // since the tapes are already set passive when forward implicit tasks finish, there is no need to do that here

    for (size_t j = taskData->positions.size() - 1; j > 0; --j) {

      #if OPDI_OMP_LOGIC_INSTRUMENT
        for (auto& instrument : ompLogicInstruments) {
          instrument->reverseImplicitTaskPart(taskData, j);
        }
      #endif

      tool->evaluate(taskData->tape,
                     taskData->positions[j],
                     taskData->positions[j - 1],
                     taskData->adjointAccessModes[j - 1] == AdjointAccessMode::Atomic);
    }

    tool->setThreadLocalTape(oldTape);

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->reverseImplicitTaskEnd(taskData);
      }
    #endif
  }

  --ParallelOmpLogic::skipParallelHandling;

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseParallelEnd(data);
    }
  #endif
}

void opdi::ParallelOmpLogic::deleteFunc(void* dataPtr) {

  Data* data = (Data*) dataPtr;

  ++ParallelOmpLogic::skipParallelHandling;

  // this triggers possibly pending implicit task end events
  #pragma omp parallel num_threads(data->actualThreads)
  {
    int threadNum = omp_get_thread_num();

    ImplicitTaskOmpLogic::Data* taskData = reinterpret_cast<ImplicitTaskOmpLogic::Data*>(data->childTasks[threadNum]);

    void* oldTape = tool->getThreadLocalTape();
    tool->setThreadLocalTape(taskData->tape);

    tool->reset(taskData->tape, taskData->positions[0], false);

    tool->setThreadLocalTape(oldTape);

    // delete data of child tasks
    for (auto const& pos : taskData->positions) {
      tool->freePosition(pos);
    }
    delete taskData;
  }

  --ParallelOmpLogic::skipParallelHandling;

  // delete data of the parallel region
  delete data;
}

opdi::LogicInterface::AdjointAccessMode opdi::ParallelOmpLogic::internalGetAdjointAccessMode(void* taskDataPtr) const {
  ImplicitTaskOmpLogic::Data* taskData = reinterpret_cast<ImplicitTaskOmpLogic::Data*>(taskDataPtr);
  return taskData->adjointAccessModes.back();
}

void opdi::ParallelOmpLogic::internalSetAdjointAccessMode(void* taskDataPtr, AdjointAccessMode mode) {
  ImplicitTaskOmpLogic::Data* taskData = reinterpret_cast<ImplicitTaskOmpLogic::Data*>(taskDataPtr);

  if (taskData->initialImplicitTask) {
    taskData->adjointAccessModes.back() = mode;
  }
  else {
    void* position = tool->allocPosition();
    tool->getTapePosition(taskData->tape, position);

    if (tool->comparePosition(taskData->positions.back(), position) == 0) {
      taskData->adjointAccessModes.back() = mode;
      tool->freePosition(position);
    }
    else {
      taskData->adjointAccessModes.push_back(mode);
      taskData->positions.push_back(position);
    }
  }
}

void* opdi::ParallelOmpLogic::onParallelBegin(void* encounteringTask, int maxThreads) {

  if (tool->getThreadLocalTape() != nullptr && ParallelOmpLogic::skipParallelHandling == 0) {

    assert(encounteringTask != nullptr);

    ImplicitTaskOmpLogic::Data* encounteringTaskData = reinterpret_cast<ImplicitTaskOmpLogic::Data*>(encounteringTask);

    assert(encounteringTaskData->initialImplicitTask || tool->getThreadLocalTape() == encounteringTaskData->tape);

    Data* data = new Data;

    data->maxThreads = maxThreads;
    data->activeParallelRegion = tool->isActive(tool->getThreadLocalTape());
    data->parentTask = encounteringTask;
    data->parentTape = tool->getThreadLocalTape();
    data->parentAdjointAccessMode = internalGetAdjointAccessMode(encounteringTask);
    data->childTasks.resize(maxThreads);

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onParallelBegin(data);
      }
    #endif

    return (void*) data;
  }

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->onParallelBegin(nullptr);
    }
  #endif

  return nullptr;
}

void opdi::ParallelOmpLogic::onParallelEnd(void* dataPtr) {

  Data* parallelData = (Data*) dataPtr;

  if (parallelData != nullptr) {

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onParallelEnd(parallelData);
      }
    #endif

    if (parallelData->activeParallelRegion) {

      Handle* handle = new Handle;
      handle->data = (void*) parallelData;
      handle->reverseFunc = ParallelOmpLogic::reverseFunc;
      handle->deleteFunc = ParallelOmpLogic::deleteFunc;

      tool->pushExternalFunction(parallelData->parentTape, handle);

      // do not delete data, it is deleted with the handle
    }

    // if needed, transport adjoint access mode of thread 0 to parent task
    ImplicitTaskOmpLogic::Data* taskData = reinterpret_cast<ImplicitTaskOmpLogic::Data*>(parallelData->childTasks[0]);

    if (internalGetAdjointAccessMode(parallelData->parentTask) != taskData->adjointAccessModes.back())
      this->internalSetAdjointAccessMode(parallelData->parentTask, taskData->adjointAccessModes.back());

    if (!parallelData->activeParallelRegion) {
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
    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onSetAdjointAccessMode(mode);
      }
    #endif

    void* taskDataPtr = backend->getTaskData();
    assert(taskDataPtr != nullptr);
    internalSetAdjointAccessMode(taskDataPtr, mode);
  #endif
}

opdi::LogicInterface::AdjointAccessMode opdi::ParallelOmpLogic::getAdjointAccessMode() const {
  void* taskDataPtr = backend->getTaskData();
  assert(taskDataPtr != nullptr);
  return internalGetAdjointAccessMode(taskDataPtr);
}

