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
#include "../../misc/tapePool.hpp"
#include "../../tool/toolInterface.hpp"

#include "../logicInterface.hpp"

#include "parallelOmpLogic.hpp"
#include "adjointAccessControl.hpp"

namespace opdi {

  struct ImplicitTaskOmpLogic : public virtual LogicInterface,
                                  public virtual AdjointAccessControl,
                                  public virtual TapePool {
    public:

      using ParallelData = typename ParallelOmpLogic::Data;

      struct Data {
        int level;
        int index;
        void* oldTape;
        ParallelData* parallelData;
      };

    public:
      virtual void* onImplicitTaskBegin(int actualParallelism, int index, void* parallelDataPtr) {

        ParallelData* parallelData = (ParallelData*) parallelDataPtr;

        if (parallelData != nullptr && tool->isActive(parallelData->masterTape)) {
          if (index == 0) {
            parallelData->actualThreads = actualParallelism;
          }

          Data* data = new Data;
          data->level = omp_get_level();
          data->index = index;
          data->oldTape = tool->getThreadLocalTape();
          data->parallelData = parallelData;

          void* newTape = TapePool::getTape(parallelData->masterTape, index);
          tool->setActive(newTape, true);

          data->parallelData->tapes[index] = newTape;

          data->parallelData->positions[index].push_back(tool->allocPosition());
          tool->getTapePosition(newTape, data->parallelData->positions[index].back());

          tool->setThreadLocalTape(newTape);

          AdjointAccessControl::pushMode(data->parallelData->outerAdjointAccessMode);
          data->parallelData->adjointAccessModes[index].push_back(data->parallelData->outerAdjointAccessMode);

          #if OPDI_LOGIC_OUT & OPDI_IMPLICIT_TASK_OUT
            TapedOutput::print("ITB l", omp_get_level(),
                               "i", index,
                               "m", parallelData->masterTape,
                               "t", newTape,
                               "p", tool->positionToString(data->parallelData->positions[index].back()));
          #endif

          return data;
        }

        return nullptr;
      }

      virtual void onImplicitTaskEnd(void* dataPtr) {

        if (dataPtr != nullptr) {
          Data* data = (Data*) dataPtr;

          AdjointAccessMode lastAccessMode = AdjointAccessControl::currentMode();
          AdjointAccessControl::popMode();
          AdjointAccessControl::currentMode() = lastAccessMode;

          tool->setThreadLocalTape(data->oldTape);

          data->parallelData->positions[data->index].push_back(tool->allocPosition());
          tool->getTapePosition(data->parallelData->tapes[data->index],
                                data->parallelData->positions[data->index].back());

          #if OPDI_LOGIC_OUT & OPDI_IMPLICIT_TASK_OUT
            TapedOutput::print("ITE l", omp_get_level(),
                               "i", data->index,
                               "m", data->parallelData->masterTape,
                               "t", data->parallelData->tapes[data->index],
                               "p", tool->positionToString(data->parallelData->positions[data->index].back()));
          #endif

          tool->setActive(data->parallelData->tapes[data->index], false);

          delete data;
        }
      }
  };
}
