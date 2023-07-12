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
#include "../../logic/logicInterface.hpp"

#include "callbacksBase.hpp"

namespace opdi {

  struct ImplicitTaskCallbacks : public virtual CallbacksBase {
    private:

      // callbacks to be registered

      static void onImplicitTask(
          ompt_scope_endpoint_t endpoint,
          ompt_data_t* parallelData,
          ompt_data_t* taskData,
          unsigned int actualParallelism,
          unsigned int index,
          int flags) {

        // no callback treatment for initial task
        if (flags & ompt_task_initial) {
          return;
        }

        if (ompt_scope_begin == endpoint) {
          taskData->ptr = logic->onImplicitTaskBegin(actualParallelism, index, parallelData->ptr);
        }
        else {
          logic->onImplicitTaskEnd(taskData->ptr);
        }
      }

    protected:

      static void init() {

        OPDI_CHECK_ERROR(CallbacksBase::registerCallback(ompt_callback_implicit_task,
                                                         (ompt_callback_t) ImplicitTaskCallbacks::onImplicitTask));
      }

      static void finalize() {

        OPDI_CHECK_ERROR(CallbacksBase::clearCallback(ompt_callback_implicit_task));
      }
  };

}
