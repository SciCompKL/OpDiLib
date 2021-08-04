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

#include <deque>
#include <omp.h>
#include <vector>

#include "../../config.hpp"
#include "../../misc/tapedOutput.hpp"
#include "../../misc/tapePool.hpp"
#include "../../tool/toolInterface.hpp"

#include "../logicInterface.hpp"

#include "adjointAccessControl.hpp"

namespace opdi {

  struct ImplicitTaskOmpLogic;

  struct ParallelOmpLogic : public virtual AdjointAccessControl,
                            public virtual LogicInterface,
                            public virtual TapePool {
    public:

      using LogicInterface::AdjointAccessMode;

      struct Data {
        public:
          int maxThreads;
          int actualThreads;
          void* masterTape;
          void** tapes;
          std::vector<std::deque<void*>> positions;
          AdjointAccessMode outerAdjointAccessMode;
          std::vector<std::deque<AdjointAccessMode>> adjointAccessModes;
      };

    private:
      static void reverseFunc(void* dataPtr) {

        Data* data = (Data*) dataPtr;

        #pragma omp parallel num_threads(data->actualThreads)
        {
          int threadNum = omp_get_thread_num();

          #if OPDI_LOGIC_OUT & OPDI_PARALLEL_OUT
            TapedOutput::print("EVAL t", data->tapes[threadNum],
                               "s", tool->positionToString(data->positions[threadNum].back()),
                               "e", tool->positionToString(data->positions[threadNum].front()));
          #endif

          void* oldTape = tool->getThreadLocalTape();
          tool->setThreadLocalTape(data->tapes[threadNum]);
          // since the tapes are already set passive prior to release, there is no need to do that here

          for (size_t i = data->positions[threadNum].size() - 1; i > 0; --i) {

            #if OPDI_LOGIC_OUT & OPDI_PARALLEL_OUT
              TapedOutput::print("PART t", data->tapes[threadNum], "i", i,
                                 "s", tool->positionToString(data->positions[threadNum][i]),
                                 "e", tool->positionToString(data->positions[threadNum][i - 1]));
            #endif

            tool->evaluate(data->tapes[threadNum],
                             data->positions[threadNum][i],
                             data->positions[threadNum][i - 1],
                             data->adjointAccessModes[threadNum][i - 1] == AdjointAccessMode::Atomic);
          }

          tool->setThreadLocalTape(oldTape);
        }
      }

      static void deleteFunc(void* dataPtr) {

        Data* data = (Data*) dataPtr;

        // this triggers possibly pending implicit task end events
        #pragma omp parallel num_threads(data->actualThreads)
        {
          int threadNum = omp_get_thread_num();

          void* oldTape = tool->getThreadLocalTape();
          tool->setThreadLocalTape(data->tapes[threadNum]);

          tool->reset(data->tapes[threadNum], data->positions[threadNum][0], false);

          tool->setThreadLocalTape(oldTape);

          // delete positions
          for (auto pos : data->positions[threadNum]) {
            tool->freePosition(pos);
          }
        }

        delete [] data->tapes;
        delete data;
      }

    public:
      virtual void* onParallelBegin(int maxThreads) {

        if (tool->getThreadLocalTape() != nullptr && tool->isActive(tool->getThreadLocalTape())) {

          #if OPDI_LOGIC_OUT & OPDI_PARALLEL_OUT
            TapedOutput::print("PB l", omp_get_level());
          #endif

          Data* data = new Data;

          data->maxThreads = maxThreads;
          data->masterTape = tool->getThreadLocalTape();
          data->tapes = new void*[maxThreads]();
          data->positions.resize(maxThreads);
          data->outerAdjointAccessMode = AdjointAccessControl::currentMode();
          data->adjointAccessModes.resize(maxThreads);

          return (void*) data;
        }

        return nullptr;
      }

      virtual void onParallelEnd(void* dataPtr) {

        Data* data = (Data*) dataPtr;

        if (data != nullptr) {

          #if OPDI_LOGIC_OUT & OPDI_PARALLEL_OUT
            TapedOutput::print("PE l", omp_get_level());
          #endif

          Handle* handle = new Handle;
          handle->data = (void*) data;
          handle->reverseFunc = ParallelOmpLogic::reverseFunc;
          handle->deleteFunc = ParallelOmpLogic::deleteFunc;

          tool->pushExternalFunction(data->masterTape, handle);

          // do not delete data, it is deleted with the handle
        }
      }

      virtual void setAdjointAccessMode(AdjointAccessMode mode) {

        #if OPDI_VARIABLE_ADJOINT_ACCESS_MODE
          AdjointAccessControl::currentMode() = mode;

          Data* data = (Data*) backend->getParallelData();
          int threadNum = omp_get_thread_num();

          if (data != nullptr) {
            data->adjointAccessModes[threadNum].push_back(mode);
            data->positions[threadNum].push_back(tool->allocPosition());
            tool->getTapePosition(tool->getThreadLocalTape(), data->positions[threadNum].back());
          }
        #endif
      }

      virtual AdjointAccessMode getAdjointAccessMode() {
        return AdjointAccessControl::currentMode();
      }
  };
}
