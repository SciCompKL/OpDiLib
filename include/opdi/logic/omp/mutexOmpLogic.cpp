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

#include "../../helpers/macros.hpp"
#include "../../helpers/exceptions.hpp"
#include "../../config.hpp"
#include "../../tool/toolInterface.hpp"

#include "instrument/ompLogicInstrumentInterface.hpp"

#include "mutexOmpLogic.hpp"

opdi::MutexOmpLogic::State opdi::MutexOmpLogic::localState;
opdi::MutexOmpLogic::State opdi::MutexOmpLogic::evalState;

void opdi::MutexOmpLogic::internalWaitReverseFunc(std::map<std::size_t, std::size_t>& evalTrace, void* dataPtr) {
  Data* data = (Data*) dataPtr;

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseMutexWait(data);
    }
  #endif

  // busy wait until trace value is matched
  while (true) {
    std::size_t currentId;

    #pragma omp atomic read
    currentId = evalTrace[data->waitId];

    if (currentId == data->traceValue) {
      break;
    }
  }
}

void opdi::MutexOmpLogic::waitCriticalReverseFunc(void* dataPtr) {
  internalWaitReverseFunc(MutexOmpLogic::evalState.criticalTrace, dataPtr);
}

void opdi::MutexOmpLogic::waitLockReverseFunc(void* dataPtr) {
  internalWaitReverseFunc(MutexOmpLogic::evalState.lockTrace, dataPtr);
}

void opdi::MutexOmpLogic::waitNestedLockReverseFunc(void* dataPtr) {
  internalWaitReverseFunc(MutexOmpLogic::evalState.nestedLockTrace, dataPtr);
}

void opdi::MutexOmpLogic::waitOrderedReverseFunc(void* dataPtr) {
  internalWaitReverseFunc(MutexOmpLogic::evalState.orderedTrace, dataPtr);
}

void opdi::MutexOmpLogic::waitReductionReverseFunc(void* dataPtr) {
  internalWaitReverseFunc(MutexOmpLogic::evalState.reductionTrace, dataPtr);
}

void opdi::MutexOmpLogic::waitDeleteFunc(void* dataPtr) {
  Data* data = (Data*) dataPtr;
  delete data;
}

void opdi::MutexOmpLogic::internalDecrementReverseFunc(std::map<std::size_t, std::size_t>& evalTrace, void* dataPtr) {
  Data* data = (Data*) dataPtr;

  // decrement trace value
  #pragma omp atomic update
  evalTrace[data->waitId] -= 1;

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->reverseMutexDecrement(data);
    }
  #endif
}

void opdi::MutexOmpLogic::decrementCriticalReverseFunc(void* dataPtr) {
  internalDecrementReverseFunc(MutexOmpLogic::evalState.criticalTrace, dataPtr);
}

void opdi::MutexOmpLogic::decrementLockReverseFunc(void* dataPtr) {
  internalDecrementReverseFunc(MutexOmpLogic::evalState.lockTrace, dataPtr);
}

void opdi::MutexOmpLogic::decrementNestedLockReverseFunc(void* dataPtr) {
  internalDecrementReverseFunc(MutexOmpLogic::evalState.nestedLockTrace, dataPtr);
}

void opdi::MutexOmpLogic::decrementOrderedReverseFunc(void* dataPtr) {
  internalDecrementReverseFunc(MutexOmpLogic::evalState.orderedTrace, dataPtr);
}

void opdi::MutexOmpLogic::decrementReductionReverseFunc(void* dataPtr) {
  internalDecrementReverseFunc(MutexOmpLogic::evalState.reductionTrace, dataPtr);
}

void opdi::MutexOmpLogic::decrementDeleteFunc(void* dataPtr) {
  Data* data = (Data*) dataPtr;
  delete data;
}

void opdi::MutexOmpLogic::internalInit() {
  omp_init_lock(&this->criticalTrace.lock);
  omp_init_lock(&this->lockTrace.lock);
  omp_init_lock(&this->nestedLockTrace.lock);
  omp_init_lock(&this->orderedTrace.lock);
  omp_init_lock(&this->reductionTrace.lock);

  this->criticalTrace.waitId = backend->getLockIdentifier(&this->criticalTrace.lock);
  this->lockTrace.waitId = backend->getLockIdentifier(&this->lockTrace.lock);
  this->nestedLockTrace.waitId = backend->getLockIdentifier(&this->nestedLockTrace.lock);
  this->orderedTrace.waitId = backend->getLockIdentifier(&this->orderedTrace.lock);
  this->reductionTrace.waitId = backend->getLockIdentifier(&this->reductionTrace.lock);
}

void opdi::MutexOmpLogic::internalFinalize() {
  omp_destroy_lock(&this->criticalTrace.lock);
  omp_destroy_lock(&this->lockTrace.lock);
  omp_destroy_lock(&this->nestedLockTrace.lock);
  omp_destroy_lock(&this->orderedTrace.lock);
  omp_destroy_lock(&this->reductionTrace.lock);
}

void opdi::MutexOmpLogic::onMutexDestroyed(MutexKind kind, std::size_t waitId) {

  #if OPDI_OMP_LOGIC_INSTRUMENT
    for (auto& instrument : ompLogicInstruments) {
      instrument->onMutexDestroyed(kind, waitId);
    }
  #endif

  switch (kind) {
    case MutexKind::Critical:
      this->criticalTrace.inactive.erase(waitId);
      break;
    case MutexKind::Lock:
      this->lockTrace.inactive.erase(waitId);
      break;
    case MutexKind::NestedLock:
      this->nestedLockTrace.inactive.erase(waitId);
      break;
    case MutexKind::Ordered:
      this->orderedTrace.inactive.erase(waitId);
      break;
    case MutexKind::Reduction:
      this->reductionTrace.inactive.erase(waitId);
      break;
    default:
      OPDI_ERROR("Invalid kind argument.");
      break;
  }
}

void opdi::MutexOmpLogic::internalOnMutexAcquired(MutexKind kind, MutexTrace& mutexTrace,
                                                  std::map<std::size_t, std::size_t>& localTrace,
                                                  void (*decrementReverseFunc)(void*),
                                                  std::size_t waitId) {

  if (tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

    // skip inactive mutexes
    if (mutexTrace.inactive.count(waitId) == 0) {

      Data* data = new Data;
      data->kind = kind;
      data->waitId = waitId;

      omp_set_lock(&mutexTrace.lock);
      data->traceValue = mutexTrace.trace[waitId]++;
      localTrace[waitId] = mutexTrace.trace[waitId]; // remember incremented trace value for the release event
      omp_unset_lock(&mutexTrace.lock);

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

void opdi::MutexOmpLogic::onMutexAcquired(MutexKind kind, std::size_t waitId) {

  switch (kind) {
    case MutexKind::Critical:
      this->internalOnMutexAcquired(kind, this->criticalTrace, MutexOmpLogic::localState.criticalTrace,
                                    MutexOmpLogic::decrementCriticalReverseFunc, waitId);
      break;
    case MutexKind::Lock:
      // always skip internal locks
      if (waitId == this->criticalTrace.waitId ||
          waitId == this->lockTrace.waitId ||
          waitId == this->nestedLockTrace.waitId ||
          waitId == this->orderedTrace.waitId ||
          waitId == this->reductionTrace.waitId) {
        return;
      }
      this->internalOnMutexAcquired(kind, this->lockTrace, MutexOmpLogic::localState.lockTrace,
                                    MutexOmpLogic::decrementLockReverseFunc, waitId);
      break;
    case MutexKind::NestedLock:
      this->internalOnMutexAcquired(kind, this->nestedLockTrace, MutexOmpLogic::localState.nestedLockTrace,
                                    MutexOmpLogic::decrementNestedLockReverseFunc, waitId);
      break;
    case MutexKind::Ordered:
      this->internalOnMutexAcquired(kind, this->orderedTrace, MutexOmpLogic::localState.orderedTrace,
                                    MutexOmpLogic::decrementOrderedReverseFunc, waitId);
      break;
    case MutexKind::Reduction:
      this->internalOnMutexAcquired(kind, this->reductionTrace, MutexOmpLogic::localState.reductionTrace,
                                    MutexOmpLogic::decrementReductionReverseFunc, waitId);
      break;
    default:
      OPDI_ERROR("Invalid kind argument.");
      break;
  }
}

void opdi::MutexOmpLogic::internalOnMutexReleased(MutexKind kind, MutexTrace& mutexTrace,
                                                  std::map<std::size_t, std::size_t>& localTrace,
                                                  void (*waitReverseFunc)(void*), std::size_t waitId) {

  if (tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

    // skip inactive mutexes
    if (mutexTrace.inactive.count(waitId) == 0) {

      Data* data = new Data;
      data->kind = kind;
      data->waitId = waitId;

      omp_set_lock(&mutexTrace.lock);
      data->traceValue = localTrace[waitId];
      omp_unset_lock(&mutexTrace.lock);

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

void opdi::MutexOmpLogic::onMutexReleased(MutexKind kind, std::size_t waitId) {

  switch (kind) {
    case MutexKind::Critical:
      this->internalOnMutexReleased(kind, this->criticalTrace, MutexOmpLogic::localState.criticalTrace,
                                    MutexOmpLogic::waitCriticalReverseFunc, waitId);
      break;
    case MutexKind::Lock:
      // always skip internal locks
      if (waitId == this->criticalTrace.waitId ||
          waitId == this->lockTrace.waitId ||
          waitId == this->nestedLockTrace.waitId ||
          waitId == this->orderedTrace.waitId ||
          waitId == this->reductionTrace.waitId) {
        return;
      }
      this->internalOnMutexReleased(kind, this->lockTrace, MutexOmpLogic::localState.lockTrace,
                                    MutexOmpLogic::waitLockReverseFunc, waitId);
      break;
    case MutexKind::NestedLock:
      this->internalOnMutexReleased(kind, this->nestedLockTrace, MutexOmpLogic::localState.nestedLockTrace,
                                    MutexOmpLogic::waitNestedLockReverseFunc, waitId);
      break;
    case MutexKind::Ordered:
      this->internalOnMutexReleased(kind, this->orderedTrace, MutexOmpLogic::localState.orderedTrace,
                                    MutexOmpLogic::waitOrderedReverseFunc, waitId);
      break;
    case MutexKind::Reduction:
      this->internalOnMutexReleased(kind, this->reductionTrace, MutexOmpLogic::localState.reductionTrace,
                                    MutexOmpLogic::waitReductionReverseFunc, waitId);
      break;
    default:
      OPDI_ERROR("Invalid kind argument.");
      break;
  }
}

// not thread safe! only use outside parallel regions
void opdi::MutexOmpLogic::registerInactiveMutex(MutexKind kind, std::size_t waitId) {

  switch (kind) {
    case MutexKind::Critical:
      this->criticalTrace.inactive.insert(waitId);
      break;
    case MutexKind::Lock:
      this->lockTrace.inactive.insert(waitId);
      break;
    case MutexKind::NestedLock:
      this->lockTrace.inactive.insert(waitId);
      break;
    case MutexKind::Ordered:
      this->orderedTrace.inactive.insert(waitId);
      break;
    case MutexKind::Reduction:
      this->reductionTrace.inactive.insert(waitId);
      break;
    default:
      OPDI_ERROR("Invalid kind argument.");
      break;
  }
}

void opdi::MutexOmpLogic::prepareEvaluate() {
  MutexOmpLogic::evalState.criticalTrace = this->criticalTrace.trace;
  MutexOmpLogic::evalState.lockTrace = this->lockTrace.trace;
  MutexOmpLogic::evalState.nestedLockTrace = this->nestedLockTrace.trace;
  MutexOmpLogic::evalState.orderedTrace = this->orderedTrace.trace;
  MutexOmpLogic::evalState.reductionTrace = this->reductionTrace.trace;
}

void opdi::MutexOmpLogic::reset() {
  this->criticalTrace.trace.clear();
  this->lockTrace.trace.clear();
  this->nestedLockTrace.trace.clear();
  this->orderedTrace.trace.clear();
  this->reductionTrace.trace.clear();
}

void* opdi::MutexOmpLogic::exportState() {
  State* state = new State;
  state->criticalTrace = this->criticalTrace.trace;
  state->lockTrace = this->lockTrace.trace;
  state->nestedLockTrace = this->nestedLockTrace.trace;
  state->orderedTrace = this->orderedTrace.trace;
  state->reductionTrace = this->reductionTrace.trace;
  return (void*) state;
}

void opdi::MutexOmpLogic::freeState(void* statePtr) {
  State* state = (State*) statePtr;
  delete state;
}

void opdi::MutexOmpLogic::recoverState(void* statePtr) {
  State* state = (State*) statePtr;
  this->criticalTrace.trace = state->criticalTrace;
  this->lockTrace.trace = state->lockTrace;
  this->nestedLockTrace.trace = state->nestedLockTrace;
  this->orderedTrace.trace = state->orderedTrace;
  this->reductionTrace.trace = state->reductionTrace;
}
