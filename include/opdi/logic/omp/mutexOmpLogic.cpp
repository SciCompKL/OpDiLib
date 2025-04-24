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

void opdi::MutexOmpLogic::internalWaitReverseFunc(MutexOmpLogic::Counters& counters,
                                                  #ifdef __SANITIZE_THREAD__
                                                    MutexOmpLogic::Counters& tsanDummies,
                                                  #endif
                                                  void* dataPtr) {
  Data* data = (Data*) dataPtr;

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseMutexWait(data);
    }
  #endif

  // busy wait until counter is matched
  while (true) {
    MutexOmpLogic::Counter currentValue;

    #pragma omp atomic read
    currentValue = counters[data->waitId];

    if (currentValue == data->counter) {
      break;
    }
  }

  #ifdef __SANITIZE_THREAD__
    ANNOTATE_RWLOCK_ACQUIRED(&tsanDummies[data->waitId], true);
  #endif
}

void opdi::MutexOmpLogic::waitCriticalReverseFunc(void* dataPtr) {
  internalWaitReverseFunc(MutexOmpLogic::evaluationCounters.criticalCounters,
                          #ifdef __SANITIZE_THREAD__
                            MutexOmpLogic::tsanDummies.criticalCounters,
                          #endif
                          dataPtr);
}

void opdi::MutexOmpLogic::waitLockReverseFunc(void* dataPtr) {
  internalWaitReverseFunc(MutexOmpLogic::evaluationCounters.lockCounters,
                          #ifdef __SANITIZE_THREAD__
                            MutexOmpLogic::tsanDummies.lockCounters,
                          #endif
                          dataPtr);
}

void opdi::MutexOmpLogic::waitNestedLockReverseFunc(void* dataPtr) {
  internalWaitReverseFunc(MutexOmpLogic::evaluationCounters.nestedLockCounters,
                          #ifdef __SANITIZE_THREAD__
                            MutexOmpLogic::tsanDummies.nestedLockCounters,
                          #endif
                          dataPtr);
}

void opdi::MutexOmpLogic::waitOrderedReverseFunc(void* dataPtr) {
  internalWaitReverseFunc(MutexOmpLogic::evaluationCounters.orderedCounters,
                          #ifdef __SANITIZE_THREAD__
                            MutexOmpLogic::tsanDummies.orderedCounters,
                          #endif
                          dataPtr);
}

void opdi::MutexOmpLogic::waitReductionReverseFunc(void* dataPtr) {
  internalWaitReverseFunc(MutexOmpLogic::evaluationCounters.reductionCounters,
                          #ifdef __SANITIZE_THREAD__
                            MutexOmpLogic::tsanDummies.reductionCounters,
                          #endif
                          dataPtr);
}

void opdi::MutexOmpLogic::waitDeleteFunc(void* dataPtr) {
  Data* data = (Data*) dataPtr;
  delete data;
}

void opdi::MutexOmpLogic::internalDecrementReverseFunc(MutexOmpLogic::Counters& counters,
                                                       #ifdef __SANITIZE_THREAD__
                                                         MutexOmpLogic::Counters& tsanDummies,
                                                       #endif
                                                       void* dataPtr) {
  Data* data = (Data*) dataPtr;

  #ifdef __SANITIZE_THREAD__
    ANNOTATE_RWLOCK_RELEASED(&tsanDummies[data->waitId], true);
  #endif

  // decrement counter
  #pragma omp atomic update
  counters[data->waitId] -= 1;

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseMutexDecrement(data);
    }
  #endif
}

void opdi::MutexOmpLogic::decrementCriticalReverseFunc(void* dataPtr) {
  internalDecrementReverseFunc(MutexOmpLogic::evaluationCounters.criticalCounters,
                               #ifdef __SANITIZE_THREAD__
                                 MutexOmpLogic::tsanDummies.criticalCounters,
                               #endif
                               dataPtr);
}

void opdi::MutexOmpLogic::decrementLockReverseFunc(void* dataPtr) {
  internalDecrementReverseFunc(MutexOmpLogic::evaluationCounters.lockCounters,
                               #ifdef __SANITIZE_THREAD__
                                 MutexOmpLogic::tsanDummies.lockCounters,
                               #endif
                               dataPtr);
}

void opdi::MutexOmpLogic::decrementNestedLockReverseFunc(void* dataPtr) {
  internalDecrementReverseFunc(MutexOmpLogic::evaluationCounters.nestedLockCounters,
                               #ifdef __SANITIZE_THREAD__
                                 MutexOmpLogic::tsanDummies.nestedLockCounters,
                               #endif
                               dataPtr);
}

void opdi::MutexOmpLogic::decrementOrderedReverseFunc(void* dataPtr) {
  internalDecrementReverseFunc(MutexOmpLogic::evaluationCounters.orderedCounters,
                               #ifdef __SANITIZE_THREAD__
                                 MutexOmpLogic::tsanDummies.orderedCounters,
                               #endif
                               dataPtr);
}

void opdi::MutexOmpLogic::decrementReductionReverseFunc(void* dataPtr) {
  internalDecrementReverseFunc(MutexOmpLogic::evaluationCounters.reductionCounters,
                               #ifdef __SANITIZE_THREAD__
                                 MutexOmpLogic::tsanDummies.reductionCounters,
                               #endif
                               dataPtr);
}

void opdi::MutexOmpLogic::decrementDeleteFunc(void* dataPtr) {
  Data* data = (Data*) dataPtr;
  delete data;
}

void opdi::MutexOmpLogic::internalInit() {
  omp_init_lock(&this->criticalRecording.lock);
  omp_init_lock(&this->lockRecording.lock);
  omp_init_lock(&this->nestedLockRecording.lock);
  omp_init_lock(&this->orderedRecording.lock);
  omp_init_lock(&this->reductionRecording.lock);

  this->criticalRecording.waitId = backend->getLockIdentifier(&this->criticalRecording.lock);
  this->lockRecording.waitId = backend->getLockIdentifier(&this->lockRecording.lock);
  this->nestedLockRecording.waitId = backend->getLockIdentifier(&this->nestedLockRecording.lock);
  this->orderedRecording.waitId = backend->getLockIdentifier(&this->orderedRecording.lock);
  this->reductionRecording.waitId = backend->getLockIdentifier(&this->reductionRecording.lock);
}

void opdi::MutexOmpLogic::internalFinalize() {
  omp_destroy_lock(&this->criticalRecording.lock);
  omp_destroy_lock(&this->lockRecording.lock);
  omp_destroy_lock(&this->nestedLockRecording.lock);
  omp_destroy_lock(&this->orderedRecording.lock);
  omp_destroy_lock(&this->reductionRecording.lock);
}

void opdi::MutexOmpLogic::onMutexDestroyed(MutexKind kind, WaitId waitId) {

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->onMutexDestroyed(kind, waitId);
    }
  #endif

  switch (kind) {
    case MutexKind::Critical:
      this->criticalRecording.inactive.erase(waitId);
      break;
    case MutexKind::Lock:
      this->lockRecording.inactive.erase(waitId);
      break;
    case MutexKind::NestedLock:
      this->nestedLockRecording.inactive.erase(waitId);
      break;
    case MutexKind::Ordered:
      this->orderedRecording.inactive.erase(waitId);
      break;
    case MutexKind::Reduction:
      this->reductionRecording.inactive.erase(waitId);
      break;
    default:
      OPDI_ERROR("Invalid kind argument.");
      break;
  }
}

void opdi::MutexOmpLogic::internalOnMutexAcquired(MutexKind kind, MutexOmpLogic::Recording& recording,
                                                  MutexOmpLogic::Counters& localCounters,
                                                  void (*decrementReverseFunc)(void*),
                                                  WaitId waitId) {

  if (tool != nullptr && tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

    // skip inactive mutexes
    if (recording.inactive.count(waitId) == 0) {

      Data* data = new Data;
      data->kind = kind;
      data->waitId = waitId;

      omp_set_lock(&recording.lock);
      data->counter = recording.counters[waitId]++;
      localCounters[waitId] = recording.counters[waitId]; // remember incremented counter value for the release event
      omp_unset_lock(&recording.lock);

      #if OPDI_OMP_LOGIC_INSTRUMENT
        for (auto& instrument : ompLogicInstruments) {
          instrument->onMutexAcquired(data);
        }
      #endif

      // push decrement handle
      Handle* handle = new Handle;
      handle->data = (void*) data;
      handle->reverseFunc = decrementReverseFunc;
      handle->deleteFunc = MutexOmpLogic::decrementDeleteFunc;

      tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
    }
  }
}

void opdi::MutexOmpLogic::onMutexAcquired(MutexKind kind, WaitId waitId) {

  switch (kind) {
    case MutexKind::Critical:
      this->internalOnMutexAcquired(kind, this->criticalRecording, MutexOmpLogic::localCounters.criticalCounters,
                                    MutexOmpLogic::decrementCriticalReverseFunc, waitId);
      break;
    case MutexKind::Lock:
      // always skip internal locks
      if (waitId == this->criticalRecording.waitId ||
          waitId == this->lockRecording.waitId ||
          waitId == this->nestedLockRecording.waitId ||
          waitId == this->orderedRecording.waitId ||
          waitId == this->reductionRecording.waitId) {
        return;
      }
      this->internalOnMutexAcquired(kind, this->lockRecording, MutexOmpLogic::localCounters.lockCounters,
                                    MutexOmpLogic::decrementLockReverseFunc, waitId);
      break;
    case MutexKind::NestedLock:
      this->internalOnMutexAcquired(kind, this->nestedLockRecording, MutexOmpLogic::localCounters.nestedLockCounters,
                                    MutexOmpLogic::decrementNestedLockReverseFunc, waitId);
      break;
    case MutexKind::Ordered:
      this->internalOnMutexAcquired(kind, this->orderedRecording, MutexOmpLogic::localCounters.orderedCounters,
                                    MutexOmpLogic::decrementOrderedReverseFunc, waitId);
      break;
    case MutexKind::Reduction:
      this->internalOnMutexAcquired(kind, this->reductionRecording, MutexOmpLogic::localCounters.reductionCounters,
                                    MutexOmpLogic::decrementReductionReverseFunc, waitId);
      break;
    default:
      OPDI_ERROR("Invalid kind argument.");
      break;
  }
}

void opdi::MutexOmpLogic::internalOnMutexReleased(MutexKind kind, MutexOmpLogic::Recording& recording,
                                                  MutexOmpLogic::Counters& localCounters,
                                                  void (*waitReverseFunc)(void*), WaitId waitId) {

  if (tool != nullptr && tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

    // skip inactive mutexes
    if (recording.inactive.count(waitId) == 0) {

      Data* data = new Data;
      data->kind = kind;
      data->waitId = waitId;

      omp_set_lock(&recording.lock);
      data->counter = localCounters[waitId];
      omp_unset_lock(&recording.lock);

      #if OPDI_OMP_LOGIC_INSTRUMENT
        for (auto& instrument : ompLogicInstruments) {
          instrument->onMutexReleased(data);
        }
      #endif

      // push wait handle
      Handle* handle = new Handle;
      handle->data = (void*) data;
      handle->reverseFunc = waitReverseFunc;
      handle->deleteFunc = MutexOmpLogic::waitDeleteFunc;

      tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
    }
  }
}

void opdi::MutexOmpLogic::onMutexReleased(MutexKind kind, WaitId waitId) {

  switch (kind) {
    case MutexKind::Critical:
      this->internalOnMutexReleased(kind, this->criticalRecording, MutexOmpLogic::localCounters.criticalCounters,
                                    MutexOmpLogic::waitCriticalReverseFunc, waitId);
      break;
    case MutexKind::Lock:
      // always skip internal locks
      if (waitId == this->criticalRecording.waitId ||
          waitId == this->lockRecording.waitId ||
          waitId == this->nestedLockRecording.waitId ||
          waitId == this->orderedRecording.waitId ||
          waitId == this->reductionRecording.waitId) {
        return;
      }
      this->internalOnMutexReleased(kind, this->lockRecording, MutexOmpLogic::localCounters.lockCounters,
                                    MutexOmpLogic::waitLockReverseFunc, waitId);
      break;
    case MutexKind::NestedLock:
      this->internalOnMutexReleased(kind, this->nestedLockRecording, MutexOmpLogic::localCounters.nestedLockCounters,
                                    MutexOmpLogic::waitNestedLockReverseFunc, waitId);
      break;
    case MutexKind::Ordered:
      this->internalOnMutexReleased(kind, this->orderedRecording, MutexOmpLogic::localCounters.orderedCounters,
                                    MutexOmpLogic::waitOrderedReverseFunc, waitId);
      break;
    case MutexKind::Reduction:
      this->internalOnMutexReleased(kind, this->reductionRecording, MutexOmpLogic::localCounters.reductionCounters,
                                    MutexOmpLogic::waitReductionReverseFunc, waitId);
      break;
    default:
      OPDI_ERROR("Invalid kind argument.");
      break;
  }
}

// not thread safe! only use outside parallel regions
void opdi::MutexOmpLogic::registerInactiveMutex(MutexKind kind, WaitId waitId) {

  switch (kind) {
    case MutexKind::Critical:
      this->criticalRecording.inactive.insert(waitId);
      break;
    case MutexKind::Lock:
      this->lockRecording.inactive.insert(waitId);
      break;
    case MutexKind::NestedLock:
      this->lockRecording.inactive.insert(waitId);
      break;
    case MutexKind::Ordered:
      this->orderedRecording.inactive.insert(waitId);
      break;
    case MutexKind::Reduction:
      this->reductionRecording.inactive.insert(waitId);
      break;
    default:
      OPDI_ERROR("Invalid kind argument.");
      break;
  }
}

void opdi::MutexOmpLogic::prepareEvaluate() {
  MutexOmpLogic::evaluationCounters.criticalCounters = this->criticalRecording.counters;
  MutexOmpLogic::evaluationCounters.lockCounters = this->lockRecording.counters;
  MutexOmpLogic::evaluationCounters.nestedLockCounters = this->nestedLockRecording.counters;
  MutexOmpLogic::evaluationCounters.orderedCounters = this->orderedRecording.counters;
  MutexOmpLogic::evaluationCounters.reductionCounters = this->reductionRecording.counters;

#ifdef __SANITIZE_THREAD__
  /* create lock annotations for the reverse pass */

  auto createReverseLocks=[](MutexOmpLogic::Counters const& counters, MutexOmpLogic::Counters& tsanDummies) {
    assert(tsanDummies.empty());
    for (auto const& pair : counters) {
      tsanDummies[pair.first] = 0;
    }

    for (auto& pair : tsanDummies) {
      ANNOTATE_RWLOCK_CREATE(&pair.second);
    }
  };

  createReverseLocks(MutexOmpLogic::evaluationCounters.criticalCounters, MutexOmpLogic::tsanDummies.criticalCounters);
  createReverseLocks(MutexOmpLogic::evaluationCounters.lockCounters, MutexOmpLogic::tsanDummies.lockCounters);
  createReverseLocks(MutexOmpLogic::evaluationCounters.nestedLockCounters, MutexOmpLogic::tsanDummies.nestedLockCounters);
  createReverseLocks(MutexOmpLogic::evaluationCounters.orderedCounters, MutexOmpLogic::tsanDummies.orderedCounters);
  createReverseLocks(MutexOmpLogic::evaluationCounters.reductionCounters, MutexOmpLogic::tsanDummies.reductionCounters);
#endif
}

void opdi::MutexOmpLogic::postEvaluate() {
#ifdef __SANITIZE_THREAD__
  /* destroy lock annotations */

  auto destroyReverseLocks=[](MutexOmpLogic::Counters& tsanDummies) {
    for (auto& pair : tsanDummies) {
      ANNOTATE_RWLOCK_DESTROY(&pair.second);
    }

    tsanDummies.clear();
  };

  destroyReverseLocks(MutexOmpLogic::tsanDummies.criticalCounters);
  destroyReverseLocks(MutexOmpLogic::tsanDummies.lockCounters);
  destroyReverseLocks(MutexOmpLogic::tsanDummies.nestedLockCounters);
  destroyReverseLocks(MutexOmpLogic::tsanDummies.orderedCounters);
  destroyReverseLocks(MutexOmpLogic::tsanDummies.reductionCounters);
#endif
}

void opdi::MutexOmpLogic::reset() {
  this->criticalRecording.counters.clear();
  this->lockRecording.counters.clear();
  this->nestedLockRecording.counters.clear();
  this->orderedRecording.counters.clear();
  this->reductionRecording.counters.clear();
}

void* opdi::MutexOmpLogic::exportState() {
  State* state = new State;
  state->criticalCounters = this->criticalRecording.counters;
  state->lockCounters = this->lockRecording.counters;
  state->nestedLockCounters = this->nestedLockRecording.counters;
  state->orderedCounters = this->orderedRecording.counters;
  state->reductionCounters = this->reductionRecording.counters;
  return (void*) state;
}

void opdi::MutexOmpLogic::freeState(void* statePtr) {
  State* state = (State*) statePtr;
  delete state;
}

void opdi::MutexOmpLogic::recoverState(void* statePtr) {
  State* state = (State*) statePtr;
  this->criticalRecording.counters = state->criticalCounters;
  this->lockRecording.counters = state->lockCounters;
  this->nestedLockRecording.counters = state->nestedLockCounters;
  this->orderedRecording.counters = state->orderedCounters;
  this->reductionRecording.counters = state->reductionCounters;
}
