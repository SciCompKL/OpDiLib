/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2024 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
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

#include "../../helpers/macros.hpp"
#include "../../config.hpp"
#include "../../tool/toolInterface.hpp"

#include "instrument/ompLogicInstrumentInterface.hpp"

#include "masterOmpLogic.hpp"

void opdi::MasterOmpLogic::reverseFunc(void *dataPtr) {

  #if OPDI_OMP_LOGIC_INSTRUMENT
    Data* data = (Data*) dataPtr;
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseMaster(data);
    }
  #else
    OPDI_UNUSED(dataPtr);
  #endif
}

void opdi::MasterOmpLogic::deleteFunc(void* dataPtr) {

  Data* data = (Data*) dataPtr;
  delete data;
}

void opdi::MasterOmpLogic::onMaster(ScopeEndpoint endpoint) {

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->onMaster(endpoint);
    }

    if (tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

        Data* data = new Data;
        data->endpoint = endpoint;

        Handle* handle = new Handle;
        handle->data = (void*) data;
        handle->reverseFunc = MasterOmpLogic::reverseFunc;
        handle->deleteFunc = MasterOmpLogic::deleteFunc;
        tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
    }
  #else
    OPDI_UNUSED(endpoint);
  #endif
}
