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

#pragma once

#include <iostream>
#include <map>
#include <omp.h>
#include <omp-tools.h>
#include <set>
#include <stack>

#include "../../helpers/exceptions.hpp"
#include "../../helpers/macros.hpp"
#include "../../logic/logicInterface.hpp"

#include "callbacksBase.hpp"
#include "waitIdExtractor.hpp"

namespace opdi {

  struct OmptBackend;

  struct MutexCallbacks : public virtual CallbacksBase {
    private:
      
      static void onMutexDestroyed(ompt_mutex_t kind,
                                   ompt_wait_id_t waitId,
                                   void const* codeptr) {
        OPDI_UNUSED(codeptr);

        switch (kind) {
          case ompt_mutex_lock:
          case ompt_mutex_test_lock:
            logic->onMutexDestroyed(LogicInterface::MutexKind::Lock, waitId);
            break;
          case ompt_mutex_nest_lock:
          case ompt_mutex_test_nest_lock:
            logic->onMutexDestroyed(LogicInterface::MutexKind::NestedLock, waitId);
            break;
          case ompt_mutex_critical:
            logic->onMutexDestroyed(LogicInterface::MutexKind::Critical, waitId);
            break;
          case ompt_mutex_ordered:
            logic->onMutexDestroyed(LogicInterface::MutexKind::Ordered, waitId);
            break;
          case ompt_mutex_atomic: // not supported, no AD handling
            break;
          default:
            OPDI_WARNING("Unknown kind argument.");
            break;
        }
      }

      static void onMutexAcquired(ompt_mutex_t kind,
                                  ompt_wait_id_t waitId,
                                  void const* codeptr) {
        OPDI_UNUSED(codeptr);

        switch (kind) {
          case ompt_mutex_lock:
          case ompt_mutex_test_lock:
            logic->onMutexAcquired(LogicInterface::MutexKind::Lock, waitId);
            break;
          case ompt_mutex_nest_lock:
          case ompt_mutex_test_nest_lock:
            logic->onMutexAcquired(LogicInterface::MutexKind::NestedLock, waitId);
            break;
          case ompt_mutex_critical:
            logic->onMutexAcquired(LogicInterface::MutexKind::Critical, waitId);
            break;
          case ompt_mutex_ordered:
            logic->onMutexAcquired(LogicInterface::MutexKind::Ordered, waitId);
            break;
          case ompt_mutex_atomic: // not supported, no AD handling
            break;
          default:
            OPDI_WARNING("Unknown kind argument.");
            break;
        }
      }

      static void onMutexReleased(ompt_mutex_t kind,
                                  ompt_wait_id_t waitId,
                                  void const* codeptr) {
        OPDI_UNUSED(codeptr);

        switch (kind) {
          case ompt_mutex_lock:
          case ompt_mutex_test_lock:
            logic->onMutexReleased(LogicInterface::MutexKind::Lock, waitId);
            break;
          case ompt_mutex_nest_lock:
          case ompt_mutex_test_nest_lock:
            logic->onMutexReleased(LogicInterface::MutexKind::NestedLock, waitId);
            break;
          case ompt_mutex_critical:
            logic->onMutexReleased(LogicInterface::MutexKind::Critical, waitId);
            break;
          case ompt_mutex_ordered:
            logic->onMutexReleased(LogicInterface::MutexKind::Ordered, waitId);
            break;
          case ompt_mutex_atomic: // not supported, no AD handling
            break;
          default:
            OPDI_WARNING("Unknown kind argument.");
            break;
        }
      }
      
    protected:

      static void init() {
        
        OPDI_CHECK_ERROR(CallbacksBase::registerCallback(ompt_callback_lock_destroy,
                                                         (ompt_callback_t) MutexCallbacks::onMutexDestroyed));
        OPDI_CHECK_ERROR(CallbacksBase::registerCallback(ompt_callback_mutex_acquired,
                                                         (ompt_callback_t) MutexCallbacks::onMutexAcquired));
        OPDI_CHECK_ERROR(CallbacksBase::registerCallback(ompt_callback_mutex_released,
                                                         (ompt_callback_t) MutexCallbacks::onMutexReleased));
      }
      
      static void finalize() {

        OPDI_CHECK_ERROR(CallbacksBase::clearCallback(ompt_callback_lock_destroy));
        OPDI_CHECK_ERROR(CallbacksBase::clearCallback(ompt_callback_mutex_acquired));
        OPDI_CHECK_ERROR(CallbacksBase::clearCallback(ompt_callback_mutex_released));
      }
  };
}
