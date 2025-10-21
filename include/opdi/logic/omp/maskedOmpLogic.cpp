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

#include <omp.h>

#include "../../helpers/macros.hpp"
#include "../../config.hpp"
#include "../../tool/toolInterface.hpp"

#include "instrument/ompLogicInstrumentInterface.hpp"

#include "maskedOmpLogic.hpp"

void opdi::MaskedOmpLogic::reverseFunc(void *dataPtr) {

  #if OPDI_OMP_LOGIC_INSTRUMENT
    Data* data = static_cast<Data*>(dataPtr);
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseMasked(data);
    }
  #else
    OPDI_UNUSED(dataPtr);
  #endif
}

void opdi::MaskedOmpLogic::deleteFunc(void* dataPtr) {

  Data* data = static_cast<Data*>(dataPtr);
  delete data;
}

void opdi::MaskedOmpLogic::onMasked(ScopeEndpoint endpoint) {

  #if OPDI_OMP_LOGIC_INSTRUMENT
    if (tool != nullptr && tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

      for (auto& instrument : ompLogicInstruments) {
        instrument->onMasked(endpoint);
      }

      Data* data = new Data;
      data->endpoint = endpoint;

      Handle* handle = new Handle;
      handle->data = static_cast<void*>(data);
      handle->reverseFunc = MaskedOmpLogic::reverseFunc;
      handle->deleteFunc = MaskedOmpLogic::deleteFunc;
      tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
    }
  #else
    OPDI_UNUSED(endpoint);
  #endif
}
