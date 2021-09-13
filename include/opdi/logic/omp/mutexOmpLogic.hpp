/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2021 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Homepage: http://www.scicomp.uni-kl.de
 * Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
 *
 * Lead developer: Johannes Blühdorn (SciComp, TU Kaiserslautern)
 *
 * This file is part of OpDiLib (http://www.scicomp.uni-kl.de/software/opdi).
 *
 * OpDiLib is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OpDiLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU
 * General Public License along with OpDiLib.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * For other licensing options please contact us.
 *
 */

#pragma once

#include <map>
#include <omp.h>
#include <set>

#include "../../helpers/macros.hpp"
#include "../../helpers/exceptions.hpp"
#include "../../config.hpp"
#include "../../misc/tapedOutput.hpp"
#include "../../tool/toolInterface.hpp"

#include "../logicInterface.hpp"

namespace opdi {

  struct MutexOmpLogic : public virtual LogicInterface {
    public:

      using LogicInterface::MutexKind;

    private:

      using Trace = std::map<std::size_t, std::size_t>;

      struct MutexTrace {
        public:
          Trace trace;
          omp_lock_t lock; // lock for internal synchronization
          std::size_t waitId; // wait id of internal lock
          std::set<std::size_t> inactive; // ids of inactive mutexes
      };

      MutexTrace criticalTrace, lockTrace, nestedLockTrace, orderedTrace, reductionTrace;

      struct State {
        public:
          Trace criticalTrace;
          Trace lockTrace;
          Trace nestedLockTrace;
          Trace orderedTrace;
          Trace reductionTrace;
      };

      static State localState;
      #pragma omp threadprivate(localState)

      static State evalState;

      struct Data {
        public:
          MutexKind kind;
          std::size_t traceValue;
          std::size_t waitId;
      };

      static void internalWaitReverseFunc(std::map<std::size_t, std::size_t>& evalTrace, void* dataPtr) {
        Data* data = (Data*) dataPtr;

        #if OPDI_LOGIC_OUT & OPDI_MUTEX_OUT
          TapedOutput::print("WAIT t", omp_get_thread_num(),
                             "k", data->kind,
                             "id", data->waitId,
                             "until", data->traceValue);
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

      static void waitCriticalReverseFunc(void* dataPtr) {
        internalWaitReverseFunc(MutexOmpLogic::evalState.criticalTrace, dataPtr);
      }

      static void waitLockReverseFunc(void* dataPtr) {
        internalWaitReverseFunc(MutexOmpLogic::evalState.lockTrace, dataPtr);
      }

      static void waitNestedLockReverseFunc(void* dataPtr) {
        internalWaitReverseFunc(MutexOmpLogic::evalState.nestedLockTrace, dataPtr);
      }

      static void waitOrderedReverseFunc(void* dataPtr) {
        internalWaitReverseFunc(MutexOmpLogic::evalState.orderedTrace, dataPtr);
      }

      static void waitReductionReverseFunc(void* dataPtr) {
        internalWaitReverseFunc(MutexOmpLogic::evalState.reductionTrace, dataPtr);
      }

      static void waitDeleteFunc(void* dataPtr) {
        Data* data = (Data*) dataPtr;
        delete data;
      }

      static void internalDecrementReverseFunc(std::map<std::size_t, std::size_t>& evalTrace, void* dataPtr) {
        Data* data = (Data*) dataPtr;

        // decrement trace value
        #pragma omp atomic update
        evalTrace[data->waitId] -= 1;

        #if OPDI_LOGIC_OUT & OPDI_MUTEX_OUT
          TapedOutput::print("DECR t", omp_get_thread_num(),
                             "k", data->kind,
                             "id", data->waitId,
                             "to", data->traceValue);
        #endif
      }

      static void decrementCriticalReverseFunc(void* dataPtr) {
        internalDecrementReverseFunc(MutexOmpLogic::evalState.criticalTrace, dataPtr);
      }

      static void decrementLockReverseFunc(void* dataPtr) {
        internalDecrementReverseFunc(MutexOmpLogic::evalState.lockTrace, dataPtr);
      }

      static void decrementNestedLockReverseFunc(void* dataPtr) {
        internalDecrementReverseFunc(MutexOmpLogic::evalState.nestedLockTrace, dataPtr);
      }

      static void decrementOrderedReverseFunc(void* dataPtr) {
        internalDecrementReverseFunc(MutexOmpLogic::evalState.orderedTrace, dataPtr);
      }

      static void decrementReductionReverseFunc(void* dataPtr) {
        internalDecrementReverseFunc(MutexOmpLogic::evalState.reductionTrace, dataPtr);
      }

      static void decrementDeleteFunc(void* dataPtr) {
        Data* data = (Data*) dataPtr;
        delete data;
      }

    public:
      MutexOmpLogic() {}

      virtual ~MutexOmpLogic() {}

    protected:
      void internalInit() {
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

      void internalFinalize() {
        omp_destroy_lock(&this->criticalTrace.lock);
        omp_destroy_lock(&this->lockTrace.lock);
        omp_destroy_lock(&this->nestedLockTrace.lock);
        omp_destroy_lock(&this->orderedTrace.lock);
        omp_destroy_lock(&this->reductionTrace.lock);
      }

    public:
      virtual void onMutexDestroyed(MutexKind kind, std::size_t waitId) {

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

        #if OPDI_LOGIC_OUT & OPDI_MUTEX_OUT
          TapedOutput::print("DESTR t", omp_get_thread_num(),
                             "k", kind,
                             "id", waitId);
        #endif
      }

    private:
      void internalOnMutexAcquired(MutexKind kind, MutexTrace& mutexTrace,
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

            #if OPDI_LOGIC_OUT & OPDI_MUTEX_OUT
              TapedOutput::print("ACQU t", omp_get_thread_num(),
                                 "k", data->kind,
                                 "id", data->waitId,
                                 "at", data->traceValue);
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

    public:

      virtual void onMutexAcquired(MutexKind kind, std::size_t waitId) {

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

    private:

      void internalOnMutexReleased(MutexKind kind, MutexTrace& mutexTrace,
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

            #if OPDI_LOGIC_OUT & OPDI_MUTEX_OUT
              TapedOutput::print("REL t", omp_get_thread_num(), "k", data->kind, "id", data->waitId, "at",
                                 data->traceValue);
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

    public:

      virtual void onMutexReleased(MutexKind kind, std::size_t waitId) {

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
      virtual void registerInactiveMutex(MutexKind kind, std::size_t waitId) {

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

      void prepareEvaluate() {
        MutexOmpLogic::evalState.criticalTrace = this->criticalTrace.trace;
        MutexOmpLogic::evalState.lockTrace = this->lockTrace.trace;
        MutexOmpLogic::evalState.nestedLockTrace = this->nestedLockTrace.trace;
        MutexOmpLogic::evalState.orderedTrace = this->orderedTrace.trace;
        MutexOmpLogic::evalState.reductionTrace = this->reductionTrace.trace;
      }

      void reset() {
        this->criticalTrace.trace.clear();
        this->lockTrace.trace.clear();
        this->nestedLockTrace.trace.clear();
        this->orderedTrace.trace.clear();
        this->reductionTrace.trace.clear();
      }

      void* exportState() {
        State* state = new State;
        state->criticalTrace = this->criticalTrace.trace;
        state->lockTrace = this->lockTrace.trace;
        state->nestedLockTrace = this->nestedLockTrace.trace;
        state->orderedTrace = this->orderedTrace.trace;
        state->reductionTrace = this->reductionTrace.trace;
        return (void*) state;
      }

      void freeState(void* statePtr) {
        State* state = (State*) statePtr;
        delete state;
      }

      void recoverState(void* statePtr) {
        State* state = (State*) statePtr;
        this->criticalTrace.trace = state->criticalTrace;
        this->lockTrace.trace = state->lockTrace;
        this->nestedLockTrace.trace = state->nestedLockTrace;
        this->orderedTrace.trace = state->orderedTrace;
        this->reductionTrace.trace = state->reductionTrace;
      }
  };
}
