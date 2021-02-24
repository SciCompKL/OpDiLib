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

#include <omp.h>

#include "../../config.hpp"
#include "../../misc/tapedOutput.hpp"
#include "../../tool/toolInterface.hpp"

#include "../logicInterface.hpp"

namespace opdi {

  struct WorkOmpLogic : public virtual LogicInterface {
    public:

      using LogicInterface::ScopeEndpoint;
      using LogicInterface::WorksharingKind;

    private:

      struct Data {
        WorksharingKind kind;
        ScopeEndpoint endpoint;
      };

      static void reverseFunc(void* dataPtr) {

        #if OPDI_LOGIC_OUT & OPDI_WORK_OUT
          Data* data = (Data*) dataPtr;
          TapedOutput::print("WBAR i", omp_get_thread_num(), "k", data->kind, "p", data->endpoint);
        #else
          OPDI_UNUSED(dataPtr);
        #endif

        #pragma omp barrier
      }

      static void deleteFunc(void* dataPtr) {
        Data* data = (Data*) dataPtr;
        delete data;
      }

    public:

      virtual void onWork(WorksharingKind /*kind*/, ScopeEndpoint /*endpoint*/) {
        /*
        if (adTool->getThreadLocalTape() != nullptr && adTool->isActive(adTool->getThreadLocalTape())) {

          #if OPDI_LOGIC_OUT & OPDI_WORK_OUT
            TapedOutput::print("WORK i", omp_get_thread_num(), "k", kind, "p", endpoint);
          #endif

          Data* data = new Data;
          data->kind = kind;
          data->endpoint = endpoint;

          Handle* handle = new Handle;
          handle->data = (void*) data;
          handle->reverseFunc = WorkOmpADLogic::reverseFunc;
          handle->deleteFunc = WorkOmpADLogic::deleteFunc;
          adTool->pushExternalFunction(adTool->getThreadLocalTape(), handle);
        }*/
      }
  };
}
