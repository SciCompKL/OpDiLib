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
#include <omp.h>
#include <omp-tools.h>

#include "../../helpers/exceptions.hpp"
#include "../../helpers/macros.hpp"

#include "callbacksBase.hpp"

namespace opdi {

  // not thread safe, do not use inside parallel code!
  struct WaitIdExtractor : public virtual CallbacksBase {
    private:
      static ompt_wait_id_t* waitId;

      static ompt_callback_t onAcquire;
      static ompt_callback_t onAcquired;
      static ompt_callback_t onReleased;
      
      static void extractWaitId(
          ompt_mutex_t kind,
          unsigned int hint,
          unsigned int impl,
          ompt_wait_id_t waitId,
          void const* codeptr) {
      
        OPDI_UNUSED(kind);
        OPDI_UNUSED(hint);
        OPDI_UNUSED(impl);
        OPDI_UNUSED(codeptr);

        *(WaitIdExtractor::waitId) = waitId;
      }
      
    public:
      static void begin(ompt_wait_id_t* waitId) {
        WaitIdExtractor::waitId = waitId;
        
        // store currently set mutex callbacks

        CallbacksBase::queryCallback(ompt_callback_mutex_acquire, &WaitIdExtractor::onAcquire);
        CallbacksBase::queryCallback(ompt_callback_mutex_acquired, &WaitIdExtractor::onAcquired);
        CallbacksBase::queryCallback(ompt_callback_mutex_released, &WaitIdExtractor::onReleased);

        // replace or disable callbacks, respectively

        OPDI_CHECK_ERROR(CallbacksBase::registerCallback(ompt_callback_mutex_acquire,
                                                         (ompt_callback_t) WaitIdExtractor::extractWaitId));
        OPDI_CHECK_ERROR(CallbacksBase::clearCallback(ompt_callback_mutex_acquired));
        OPDI_CHECK_ERROR(CallbacksBase::clearCallback(ompt_callback_mutex_released));
      }
      
      static void end() {

        // recover previous callbacks

        OPDI_CHECK_ERROR(CallbacksBase::registerCallback(ompt_callback_mutex_acquire, WaitIdExtractor::onAcquire));
        OPDI_CHECK_ERROR(CallbacksBase::registerCallback(ompt_callback_mutex_acquired, WaitIdExtractor::onAcquired));
        OPDI_CHECK_ERROR(CallbacksBase::registerCallback(ompt_callback_mutex_released, WaitIdExtractor::onReleased));
      }
  };
  
}
