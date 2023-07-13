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

#include <omp.h>

#include "../../config.hpp"
#include "../../tool/toolInterface.hpp"

#include "instrument/ompLogicInstrumentInterface.hpp"

#include "parallelOmpLogic.hpp"

void opdi::ParallelOmpLogic::reverseFunc(void* dataPtr) {

  Data* data = (Data*) dataPtr;

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseParallelBegin(data);
    }
  #endif

  #pragma omp parallel num_threads(data->actualThreads)
  {
    int threadNum = omp_get_thread_num();

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->reverseImplicitTaskBegin(data, threadNum);
      }
    #endif

    void* oldTape = tool->getThreadLocalTape();
    tool->setThreadLocalTape(data->tapes[threadNum]);
    // since the tapes are already set passive prior to release, there is no need to do that here

    for (size_t j = data->positions[threadNum].size() - 1; j > 0; --j) {

      #if OPDI_OMP_LOGIC_INSTRUMENT
        for (auto& instrument : ompLogicInstruments) {
          instrument->reverseImplicitTaskPart(data, threadNum, j);
        }
      #endif

      tool->evaluate(data->tapes[threadNum],
                     data->positions[threadNum][j],
                     data->positions[threadNum][j - 1],
                     data->adjointAccessModes[threadNum][j - 1] == AdjointAccessMode::Atomic);
    }

    tool->setThreadLocalTape(oldTape);

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->reverseImplicitTaskEnd(data, threadNum);
      }
    #endif
  }

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseParallelEnd(data);
    }
  #endif
}

void opdi::ParallelOmpLogic::deleteFunc(void* dataPtr) {

  Data* data = (Data*) dataPtr;

  // this triggers possibly pending implicit task end events
  #pragma omp parallel num_threads(data->actualThreads)
  {
    int threadNum = omp_get_thread_num();

    void* oldTape = tool->getThreadLocalTape();
    tool->setThreadLocalTape(data->tapes[threadNum]);

    tool->reset(data->tapes[threadNum], data->positions[threadNum][0], false);

    tool->setThreadLocalTape(oldTape);

    // delete positions
    for (auto pos : data->positions[threadNum]) {
      tool->freePosition(pos);
    }
  }

  delete [] data->tapes;
  delete data;
}

void* opdi::ParallelOmpLogic::onParallelBegin(int maxThreads) {

  if (tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

    Data* data = new Data;

    data->maxThreads = maxThreads;
    data->masterTape = tool->getThreadLocalTape();
    data->tapes = new void*[maxThreads]();
    data->positions.resize(maxThreads);
    data->outerAdjointAccessMode = AdjointAccessControl::currentMode();
    data->adjointAccessModes.resize(maxThreads);

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

  Data* data = (Data*) dataPtr;

  if (data != nullptr) {

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onParallelEnd(data);
      }
    #endif

    Handle* handle = new Handle;
    handle->data = (void*) data;
    handle->reverseFunc = ParallelOmpLogic::reverseFunc;
    handle->deleteFunc = ParallelOmpLogic::deleteFunc;

    tool->pushExternalFunction(data->masterTape, handle);

    // do not delete data, it is deleted with the handle
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
    AdjointAccessControl::currentMode() = mode;

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onSetAdjointAccessMode(mode);
      }
    #endif

    Data* data = (Data*) backend->getParallelData();
    int threadNum = omp_get_thread_num();

    if (data != nullptr) {
      data->adjointAccessModes[threadNum].push_back(mode);
      data->positions[threadNum].push_back(tool->allocPosition());
      tool->getTapePosition(tool->getThreadLocalTape(), data->positions[threadNum].back());
    }
  #endif
}

opdi::LogicInterface::AdjointAccessMode opdi::ParallelOmpLogic::getAdjointAccessMode() {
  return AdjointAccessControl::currentMode();
}

