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

#include <stack>

#include "../../logic/logicInterface.hpp"

namespace opdi {

  struct ImplicitBarrierTools {
    public:
      static std::stack<bool> implicitBarrierStack;
      #pragma omp threadprivate(implicitBarrierStack)

      static void beginRegionWithImplicitBarrier() {
        ImplicitBarrierTools::implicitBarrierStack.push(true);
      }

      static void nowait() {
        ImplicitBarrierTools::implicitBarrierStack.top() = false;
      }

      static void endRegionWithImplicitBarrier() {
        if (ImplicitBarrierTools::implicitBarrierStack.top()) {
          logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplicit,
                              LogicInterface::ScopeEndpoint::Begin);
          ImplicitBarrierTools::implicitBarrierStack.pop();
        }
      }
  };

}
