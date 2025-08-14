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

void* opdi::ImplicitTaskOmpLogic::onImplicitTaskBegin(bool initialImplicitTask, int actualParallelism, int index,
                                                      void* parallelDataPtr) {

  ParallelData* parallelData = (ParallelData*) parallelDataPtr;

  // check if the handling of the parallel region was skipped
  if (parallelData != nullptr || initialImplicitTask) {

    Data* data = new Data;
    data->initialImplicitTask = initialImplicitTask;
    data->level = omp_get_level();
    data->index = index;

    // OpDiLib does not interfere with the initial implicit task AD-wise, e.g., does not track its tape / does not
    // assume that the tape does not change. OpDiLib uses the initial implicit task's data primarily to track its
    // adjoint access mode.
    if (!initialImplicitTask) {
      if (index == 0) {
        if (parallelData->maxThreads < actualParallelism) {
          OPDI_ERROR("Actual number of threads exceeds maximum number of threads.");
        }
        parallelData->actualThreads = actualParallelism;
      }

      data->oldTape = tool->getThreadLocalTape();
      data->parallelData = parallelData;

      void* newTape = this->tapePool.getTape(parallelData->parentTape, index);

      if (parallelData->activeParallelRegion) {
        //most recent tape activity change *per thread* reflects the current activity
        if (index == 0) {
          tool->setActive(data->oldTape, false);  // suspend recording on encountering task's tape
        }
        tool->setActive(newTape, true);
      }

      data->tape = newTape;

      data->positions.push_back(tool->allocPosition());
      tool->getTapePosition(newTape, data->positions.back());

      tool->setThreadLocalTape(newTape);

      data->adjointAccessModes.push_back(parallelData->parentAdjointAccessMode);

      parallelData->childTasks[index] = data;
    }
    else {
      data->oldTape = nullptr;
      data->tape = nullptr;
      data->parallelData = nullptr;

      data->adjointAccessModes.push_back(ImplicitTaskOmpLogic::defaultAdjointAccessMode);
    }

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onImplicitTaskBegin(data);
      }
    #endif

    return data;
  }

  return nullptr;
}

void opdi::ImplicitTaskOmpLogic::onImplicitTaskEnd(void* dataPtr) {

  if (dataPtr != nullptr) {
    Data* data = (Data*) dataPtr;

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onImplicitTaskEnd(data);
      }
    #endif

    if (!data->initialImplicitTask) {
      tool->setThreadLocalTape(data->oldTape);

      data->positions.push_back(tool->allocPosition());
      tool->getTapePosition(data->tape, data->positions.back());

      if (!data->parallelData->activeParallelRegion) {
        if (tool->comparePosition(data->positions.front(), data->positions.back()) != 0) {
          OPDI_ERROR("Something became active during a passive parallel region. This is not supported and will not be",
                     "differentiated correctly.");
        }
      }
      else {
        //most recent tape activity change *per thread* reflects the current activity
        tool->setActive(data->tape, false);
        if (data->index == 0) {
          tool->setActive(data->oldTape, true);  // resume recording on encountering task's tape
        }
      }

      // do not delete data, it is deleted as part of parallel regions
    }
    else {
      // delete task data, there is no parallel region to do so
      delete data;
    }
  }
}

void opdi::ImplicitTaskOmpLogic::resetTask(void* position, opdi::LogicInterface::AdjointAccessMode mode) {

  void* taskDataPtr = backend->getTaskData();

  if (taskDataPtr != nullptr) {
    opdi::ImplicitTaskOmpLogic::Data* taskData = reinterpret_cast<opdi::ImplicitTaskOmpLogic::Data*>(taskDataPtr);

    if (!taskData->initialImplicitTask) {
      assert(tool->comparePosition(taskData->positions.front(), position) <= 0);

      while (tool->comparePosition(taskData->positions.back(), position) > 0) {
        taskData->positions.pop_back();
        taskData->adjointAccessModes.pop_back();
      }
    }

    taskData->adjointAccessModes.back() = mode;
  }
}
