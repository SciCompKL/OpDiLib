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

#include "../../config.hpp"
#include "../../tool/toolInterface.hpp"

#include "instrument/ompLogicInstrumentInterface.hpp"

#include "syncRegionOmpLogic.hpp"

void opdi::SyncRegionOmpLogic::reverseFunc(void* dataPtr) {

  #if OPDI_OMP_LOGIC_INSTRUMENT
    Data* data = static_cast<Data*>(dataPtr);
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseSyncRegion(data);
    }
  #else
    OPDI_UNUSED(dataPtr);
  #endif

  #pragma omp barrier
}

void opdi::SyncRegionOmpLogic::deleteFunc(void* dataPtr) {

  Data* data = static_cast<Data*>(dataPtr);
  delete data;
}

void opdi::SyncRegionOmpLogic::internalPushHandle(SyncRegionKind kind, ScopeEndpoint endpoint) {

  Data* data = new Data;
  data->kind = kind;
  data->endpoint = endpoint;

  Handle* handle = new Handle;
  handle->data = static_cast<void*>(data);
  handle->reverseFunc = SyncRegionOmpLogic::reverseFunc;
  handle->deleteFunc = SyncRegionOmpLogic::deleteFunc;

  tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
}

bool opdi::SyncRegionOmpLogic::requiresReverseBarrier(SyncRegionKind kind, ScopeEndpoint endpoint) {

  static std::size_t constexpr syncRegionBehaviour[] = {
      OPDI_SYNC_REGION_BARRIER_BEHAVIOUR,
      OPDI_SYNC_REGION_BARRIER_IMPLICIT_BEHAVIOUR,
      OPDI_SYNC_REGION_BARRIER_EXPLICIT_BEHAVIOUR,
      OPDI_SYNC_REGION_BARRIER_IMPLEMENTATION_BEHAVIOUR,
      OPDI_SYNC_REGION_BARRIER_REVERSE_BEHAVIOUR
  };

  assert(1 <= kind && kind <= 5);
  assert(1 == endpoint || 2 == endpoint);

  return syncRegionBehaviour[kind - 1] & endpoint;
}

void opdi::SyncRegionOmpLogic::onSyncRegion(SyncRegionKind kind, ScopeEndpoint endpoint) {

  if (tool != nullptr && tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

    #if OPDI_OMP_LOGIC_INSTRUMENT
        for (auto& instrument : ompLogicInstruments) {
          instrument->onSyncRegion(kind, endpoint);
        }
    #endif

    if (requiresReverseBarrier(kind, endpoint)) {
      internalPushHandle(kind, endpoint);
    }
  }
}

void opdi::SyncRegionOmpLogic::addReverseBarrier() {

  this->onSyncRegion(SyncRegionKind::BarrierReverse, ScopeEndpoint::Begin);
  this->onSyncRegion(SyncRegionKind::BarrierReverse, ScopeEndpoint::End);
}
