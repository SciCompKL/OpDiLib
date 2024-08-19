/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2024 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
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

#include <vector>

#include "../../misc/tapePool.hpp"

#include "../logicInterface.hpp"

#include "adjointAccessControl.hpp"

namespace opdi {

  struct ParallelOmpLogic : public virtual AdjointAccessControl,
                            public virtual LogicInterface,
                            public virtual TapePool {
    public:

      using LogicInterface::AdjointAccessMode;

      struct Data {
        public:
          int maxThreads;
          int actualThreads;
          bool activeParallelRegion;
          void* parentTask;  // ParallelOmpLogic::Data of parent task
          void* parentTape;
          AdjointAccessMode parentAdjointAccessMode;
          std::vector<void*> childTasks;  // ImplicitTaskOmpLogic::Data of child tasks
      };

    private:

      static int skipParallelHandling;
      #pragma omp threadprivate(skipParallelHandling)

      static void reverseFunc(void* dataPtr);
      static void deleteFunc(void* dataPtr);

    public:

      virtual void* onParallelBegin(void* encounteringTask, int maxThreads);
      virtual void onParallelEnd(void* dataPtr);

      virtual void setAdjointAccessMode(AdjointAccessMode mode);
      virtual AdjointAccessMode getAdjointAccessMode();
  };
}
