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

#include <vector>

#include "../../misc/tapePool.hpp"

#include "../logicInterface.hpp"

namespace opdi {

  struct ImplicitTaskData;

  struct ParallelData {
    public:
      int maximumSizeOfTeam;
      int actualSizeOfTeam;
      bool isActiveParallelRegion;
      ImplicitTaskData* encounteringTaskData;
      void* encounteringTaskTape;
      void* encounteringTaskTapePosition;
      LogicInterface::AdjointAccessMode encounteringTaskAdjointAccessMode;
      std::vector<ImplicitTaskData*> childTaskData;
  };

  struct ParallelOmpLogic : public virtual LogicInterface {
    public:

      using LogicInterface::AdjointAccessMode;

    private:

      static int skipParallelRegion;
      #pragma omp threadprivate(skipParallelRegion)

      static void internalBeginSkippedParallelRegion();
      static void internalEndSkippedParallelRegion();

      static void reverseFunc(void* parallelData);
      static void deleteFunc(void* parallelData);

      AdjointAccessMode internalGetAdjointAccessMode(ImplicitTaskData* implicitTaskData) const;
      void internalSetAdjointAccessMode(ImplicitTaskData* implicitTaskData, AdjointAccessMode mode);

    public:

      virtual void* onParallelBegin(void* encounteringTaskData, int maximumSizeOfTeam);
      virtual void onParallelEnd(void* parallelData);

      virtual void setAdjointAccessMode(AdjointAccessMode mode);
      virtual AdjointAccessMode getAdjointAccessMode() const;

      virtual void beginSkippedParallelRegion();
      virtual void endSkippedParallelRegion();
  };
}
