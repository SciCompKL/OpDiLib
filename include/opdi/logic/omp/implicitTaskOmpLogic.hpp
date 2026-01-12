/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2026 Chair for Scientific Computing (SciComp), RPTU University Kaiserslautern-Landau
 * Homepage: https://scicomp.rptu.de
 * Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
 *
 * Lead developer: Johannes Blühdorn (SciComp, RPTU University Kaiserslautern-Landau)
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

#pragma once

#include <deque>

#include "../../misc/tapePool.hpp"

#include "../logicInterface.hpp"

#include "parallelOmpLogic.hpp"

namespace opdi {

  struct ImplicitTaskData {
    public:
      bool isInitialImplicitTask;
      int level;
      int indexInTeam;
      void* oldTape;
      void* newTape;
      ParallelData* parallelData;
      std::deque<void*> positions;
      std::deque<LogicInterface::AdjointAccessMode> adjointAccessModes;
  };

  struct ImplicitTaskOmpLogic : public virtual LogicInterface {
    protected:
      TapePool tapePool;

      void internalInit();
      void internalFinalize();

    public:

      using LogicInterface::AdjointAccessMode;

      static AdjointAccessMode const defaultAdjointAccessMode;

      virtual void* onImplicitTaskBegin(bool isInitialImplicitTask, int actualSizeOfTeam, int indexInTeam,
                                        void* parallelData);
      virtual void onImplicitTaskEnd(void* implicitTaskData);

      virtual void resetImplicitTask(void* position, AdjointAccessMode mode);
  };
}
