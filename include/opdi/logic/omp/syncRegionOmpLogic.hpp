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

#include "../../config.hpp"
#include "../../misc/tapedOutput.hpp"
#include "../../tool/toolInterface.hpp"

#include "../logicInterface.hpp"

namespace opdi {

  struct SyncRegionOmpLogic : public virtual LogicInterface {
    public:

      using LogicInterface::ScopeEndpoint;
      using LogicInterface::SyncRegionKind;

    private:

      struct Data {
        public:
          SyncRegionKind kind;
          ScopeEndpoint endpoint;
      };

      static void reverseFunc(void* dataPtr) {

        #if OPDI_LOGIC_OUT & OPDI_SYNC_REGION_OUT
          Data* data = (Data*) dataPtr;
          TapedOutput::print("SBAR i", omp_get_thread_num(), "k", data->kind, "p", data->endpoint);
        #else
          OPDI_UNUSED(dataPtr);
        #endif

        #pragma omp barrier
      }

      static void deleteFunc(void* dataPtr) {
        Data* data = (Data*) dataPtr;
        delete data;
      }

      void internalPushHandle(Data* data) {
        Handle* handle = new Handle;
        handle->data = (void*) data;
        handle->reverseFunc = SyncRegionOmpLogic::reverseFunc;
        handle->deleteFunc = SyncRegionOmpLogic::deleteFunc;
        tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
      }

    public:

      virtual void onSyncRegion(SyncRegionKind kind, ScopeEndpoint endpoint) {
        if (tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {
          if (endpoint == ScopeEndpoint::Begin) {
            #if OPDI_LOGIC_OUT & OPDI_SYNC_REGION_OUT
              TapedOutput::print("SYNC i", omp_get_thread_num(), "k", kind, "p", endpoint);
            #endif

            Data* data = new Data;
            data->kind = kind;
            data->endpoint = endpoint;

            this->internalPushHandle(data);
          }
        }
      }

      virtual void addReverseBarrier() {
        Data* data = new Data;
        data->kind = SyncRegionKind::BarrierReverse;
        data->endpoint = ScopeEndpoint::BeginEnd;

        this->internalPushHandle(data);
      }

  };
}
