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

#include "../../helpers/macros.hpp"
#include "../../helpers/exceptions.hpp"
#include "../../config.hpp"
#include "../../tool/toolInterface.hpp"

#include "instrument/ompLogicInstrumentInterface.hpp"

#include "mutexOmpLogic.hpp"

opdi::MutexOmpLogic::AllCounters opdi::MutexOmpLogic::localCounters;
opdi::MutexOmpLogic::AllCounters opdi::MutexOmpLogic::evaluationCounters;
#ifdef __SANITIZE_THREAD__
  opdi::MutexOmpLogic::AllCounters opdi::MutexOmpLogic::tsanDummies;
#endif

void opdi::MutexOmpLogic::checkKind(MutexKind mutexKind) {
  if (mutexKind >= nMutexKind) {
    OPDI_ERROR("Invalid mutex kind.");
  }
}

void opdi::MutexOmpLogic::waitReverseFunc(void* dataPtr) {

  Data* data = static_cast<Data*>(dataPtr);

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseMutexWait(data);
    }
  #endif

  // busy wait until counter is matched
  while (true) {
    MutexOmpLogic::Counter currentValue;

    #pragma omp atomic read
    currentValue = MutexOmpLogic::evaluationCounters[data->mutexKind][data->waitId];

    if (currentValue == data->counter) {
      break;
    }
  }

  #ifdef __SANITIZE_THREAD__
    ANNOTATE_RWLOCK_ACQUIRED(&MutexOmpLogic::tsanDummies[data->mutexKind][data->waitId], true);
  #endif
}

void opdi::MutexOmpLogic::decrementReverseFunc(void* dataPtr) {

  Data* data = static_cast<Data*>(dataPtr);

  #ifdef __SANITIZE_THREAD__
    ANNOTATE_RWLOCK_RELEASED(&MutexOmpLogic::tsanDummies[data->mutexKind][data->waitId], true);
  #endif

  // decrement counter
  #ifdef NDEBUG
    #pragma omp atomic update
    MutexOmpLogic::evaluationCounters[data->mutexKind][data->waitId] -= 1;
  #else
    Counter newValue;
    #pragma omp atomic capture
    {
      MutexOmpLogic::evaluationCounters[data->mutexKind][data->waitId] -= 1;
      newValue = MutexOmpLogic::evaluationCounters[data->mutexKind][data->waitId];
    }
    assert(newValue == data->counter);
  #endif

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseMutexDecrement(data);
    }
  #endif
}

void opdi::MutexOmpLogic::deleteFunc(void* dataPtr) {
  Data* data = static_cast<Data*>(dataPtr);
  delete data;
}

void opdi::MutexOmpLogic::internalInit() {
  for (std::size_t mutexKind = 0; mutexKind < nMutexKind; ++mutexKind) {
    omp_init_lock(&this->recordings[mutexKind].lock);
    this->recordings[mutexKind].waitId = backend->getLockIdentifier(&this->recordings[mutexKind].lock);
  }
}

void opdi::MutexOmpLogic::internalFinalize() {
  for (std::size_t mutexKind = 0; mutexKind < nMutexKind; ++mutexKind) {
    omp_destroy_lock(&this->recordings[mutexKind].lock);
  }
}

void opdi::MutexOmpLogic::onMutexDestroyed(MutexKind mutexKind, WaitId waitId) {

  #if OPDI_OMP_LOGIC_INSTRUMENT
    Data data = {mutexKind, waitId, 0};
    for (auto& instrument : ompLogicInstruments) {
      instrument->onMutexDestroyed(&data);
    }
  #endif

  checkKind(mutexKind);
  this->recordings[mutexKind].inactive.erase(waitId);
}

void opdi::MutexOmpLogic::onMutexAcquired(MutexKind mutexKind, WaitId waitId) {

  checkKind(mutexKind);

  // always skip internal locks
  if (MutexKind::Lock == mutexKind) {
    for (std::size_t mutexKind = 0; mutexKind < nMutexKind; ++mutexKind) {
      if (waitId == this->recordings[mutexKind].waitId) {
        return;
      }
    }
  }

  if (tool != nullptr && tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

    // skip inactive mutexes
    if (recordings[mutexKind].inactive.count(waitId) == 0) {

      Data* data = new Data;
      data->mutexKind = mutexKind;
      data->waitId = waitId;

      omp_set_lock(&recordings[mutexKind].lock);
      data->counter = recordings[mutexKind].counters[waitId]++;  // store value prior to increment
      localCounters[mutexKind][waitId] = recordings[mutexKind].counters[waitId];  // remember incremented counter value for the release event
      omp_unset_lock(&recordings[mutexKind].lock);

      #if OPDI_OMP_LOGIC_INSTRUMENT
        for (auto& instrument : ompLogicInstruments) {
          instrument->onMutexAcquired(data);
        }
      #endif

      // push decrement handle
      Handle* handle = new Handle;
      handle->data = static_cast<void*>(data);
      handle->reverseFunc = MutexOmpLogic::decrementReverseFunc;
      handle->deleteFunc = MutexOmpLogic::deleteFunc;

      tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
    }
  }
}

void opdi::MutexOmpLogic::onMutexReleased(MutexKind mutexKind, WaitId waitId) {

  checkKind(mutexKind);

  // always skip internal locks
  if (MutexKind::Lock == mutexKind) {
    for (std::size_t mutexKind = 0; mutexKind < nMutexKind; ++mutexKind) {
      if (waitId == this->recordings[mutexKind].waitId) {
        return;
      }
    }
  }

  if (tool != nullptr && tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

    // skip inactive mutexes
    if (recordings[mutexKind].inactive.count(waitId) == 0) {

      Data* data = new Data;
      data->mutexKind = mutexKind;
      data->waitId = waitId;

      data->counter = localCounters[mutexKind][waitId];

      #if OPDI_OMP_LOGIC_INSTRUMENT
        for (auto& instrument : ompLogicInstruments) {
          instrument->onMutexReleased(data);
        }
      #endif

      // push wait handle
      Handle* handle = new Handle;
      handle->data = static_cast<void*>(data);
      handle->reverseFunc = MutexOmpLogic::waitReverseFunc;
      handle->deleteFunc = MutexOmpLogic::deleteFunc;

      tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
    }
  }
}

// not thread-safe! only use outside of parallel regions
void opdi::MutexOmpLogic::registerInactiveMutex(MutexKind mutexKind, WaitId waitId) {
  checkKind(mutexKind);
  this->recordings[mutexKind].inactive.insert(waitId);
}

// not thread-safe! only use outside of parallel regions
void opdi::MutexOmpLogic::prepareEvaluate() {
  for (std::size_t mutexKind = 0; mutexKind < nMutexKind; ++mutexKind) {
    MutexOmpLogic::evaluationCounters[mutexKind] = this->recordings[mutexKind].counters;
  }

#ifdef __SANITIZE_THREAD__
  /* create lock annotations for the reverse pass */

  for (std::size_t mutexKind = 0; mutexKind < nMutexKind; ++mutexKind) {
    assert(tsanDummies[mutexKind].empty());
    for (auto const& pair : evaluationCounters[mutexKind]) {
      tsanDummies[mutexKind][pair.first] = 0;
    }

    for (auto& pair : tsanDummies[mutexKind]) {
      ANNOTATE_RWLOCK_CREATE(&pair.second);
    }
  }
#endif
}

// not thread-safe! only use outside of parallel regions
void opdi::MutexOmpLogic::postEvaluate() {
#ifdef __SANITIZE_THREAD__
  /* destroy lock annotations */

  for (std::size_t mutexKind = 0; mutexKind < nMutexKind; ++mutexKind) {
    for (auto& pair : tsanDummies[mutexKind]) {
      ANNOTATE_RWLOCK_DESTROY(&pair.second);
    }

    tsanDummies[mutexKind].clear();
  }
#endif
}

// not thread-safe! only use outside of parallel regions
void opdi::MutexOmpLogic::reset() {
  for (std::size_t mutexKind = 0; mutexKind < nMutexKind; ++mutexKind) {
    this->recordings[mutexKind].counters.clear();
  }
}

// not thread-safe! only use outside of parallel regions
void* opdi::MutexOmpLogic::exportState() {
  State* state = new State;
  for (std::size_t mutexKind = 0; mutexKind < nMutexKind; ++mutexKind) {
    (*state)[mutexKind] = this->recordings[mutexKind].counters;
  }
  return static_cast<void*>(state);
}

void opdi::MutexOmpLogic::freeState(void* statePtr) {
  State* state = static_cast<State*>(statePtr);
  delete state;
}

// not thread safe! only use outside parallel regions
void opdi::MutexOmpLogic::recoverState(void* statePtr) {
  State* state = static_cast<State*>(statePtr);
  for (std::size_t mutexKind = 0; mutexKind < nMutexKind; ++mutexKind) {
    this->recordings[mutexKind].counters = (*state)[mutexKind];
  }
}
