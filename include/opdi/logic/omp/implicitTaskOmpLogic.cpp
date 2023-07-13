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

#include "../../config.hpp"
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

void* opdi::ImplicitTaskOmpLogic::onImplicitTaskBegin(int actualParallelism, int index, void* parallelDataPtr) {

  ParallelData* parallelData = (ParallelData*) parallelDataPtr;

  if (parallelData != nullptr) {
    if (index == 0) {
      parallelData->actualThreads = actualParallelism;
    }

    Data* data = new Data;
    data->level = omp_get_level();
    data->index = index;
    data->oldTape = tool->getThreadLocalTape();
    data->parallelData = parallelData;

    void* newTape = this->tapePool.getTape(parallelData->masterTape, index);
    tool->setActive(newTape, true);

    data->parallelData->tapes[index] = newTape;

    data->parallelData->positions[index].push_back(tool->allocPosition());
    tool->getTapePosition(newTape, data->parallelData->positions[index].back());

    tool->setThreadLocalTape(newTape);

    AdjointAccessControl::pushMode(data->parallelData->outerAdjointAccessMode);
    data->parallelData->adjointAccessModes[index].push_back(data->parallelData->outerAdjointAccessMode);

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

    AdjointAccessMode lastAccessMode = AdjointAccessControl::currentMode();
    AdjointAccessControl::popMode();
    AdjointAccessControl::currentMode() = lastAccessMode;

    tool->setThreadLocalTape(data->oldTape);

    data->parallelData->positions[data->index].push_back(tool->allocPosition());
    tool->getTapePosition(data->parallelData->tapes[data->index],
                          data->parallelData->positions[data->index].back());

    #if OPDI_OMP_LOGIC_INSTRUMENT
      for (auto& instrument : ompLogicInstruments) {
        instrument->onImplicitTaskEnd(data);
      }
    #endif

    tool->setActive(data->parallelData->tapes[data->index], false);

    if (data->oldTape == data->parallelData->masterTape) {
      tool->setActive(data->oldTape, true);
    }

    delete data;
  }
}
