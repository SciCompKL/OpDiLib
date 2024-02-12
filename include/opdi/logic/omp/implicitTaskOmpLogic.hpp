/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2024 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
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

      using ParallelData = typename ParallelOmpLogic::Data;

      struct Data {
        public:
          int level;
          int index;
          void* oldTape;
          ParallelData* parallelData;
      };

      virtual void* onImplicitTaskBegin(int actualParallelism, int index, void* parallelDataPtr);
      virtual void onImplicitTaskEnd(void* dataPtr);
  };
}
