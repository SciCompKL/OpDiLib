/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2025 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
 * Homepage: https://scicomp.rptu.de
 * Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
 *
 * Lead developer: Johannes Blühdorn (SciComp, University of Kaiserslautern-Landau)
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

#include "adjointAccessControl.hpp"
#include "parallelOmpLogic.hpp"

namespace opdi {

  struct ImplicitTaskOmpLogic : public virtual LogicInterface,
                                public virtual AdjointAccessControl {
    protected:
      TapePool tapePool;

      void internalInit();
      void internalFinalize();

    public:

      using LogicInterface::AdjointAccessMode;

      static AdjointAccessMode const defaultAdjointAccessMode;

      using ParallelData = typename ParallelOmpLogic::Data;

      struct Data {
        public:
          bool initialImplicitTask;
          int level;
          int index;
          void* oldTape;
          void* tape;
          ParallelData* parallelData;
          std::deque<void*> positions;
          std::deque<AdjointAccessMode> adjointAccessModes;
      };

      virtual void* onImplicitTaskBegin(bool initialImplicitTask, int actualParallelism, int index,
                                        void* parallelDataPtr);
      virtual void onImplicitTaskEnd(void* dataPtr);
  };
}
