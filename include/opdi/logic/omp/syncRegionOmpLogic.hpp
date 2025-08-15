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

#include "../../config.hpp"

#include "../logicInterface.hpp"

namespace opdi {

  struct SyncRegionOmpLogic : public virtual LogicInterface {
    public:

      static int constexpr syncRegionBehaviour[] = {
        0,  /* array offset */
        OPDI_SYNC_REGION_BARRIER_BEHAVIOUR,
        OPDI_SYNC_REGION_BARRIER_IMPLICIT_BEHAVIOUR,
        OPDI_SYNC_REGION_BARRIER_EXPLICIT_BEHAVIOUR,
        OPDI_SYNC_REGION_BARRIER_IMPLEMENTATION_BEHAVIOUR
      };

      using LogicInterface::ScopeEndpoint;
      using LogicInterface::SyncRegionKind;

      struct Data {
        public:
          SyncRegionKind kind;
          ScopeEndpoint endpoint;
      };

    private:

      static void reverseFunc(void* dataPtr);
      static void deleteFunc(void* dataPtr);

      void internalPushHandle(SyncRegionKind kind, ScopeEndpoint endpoint);

    public:

      virtual void onSyncRegion(SyncRegionKind kind, ScopeEndpoint endpoint);
      virtual void addReverseBarrier();
  };
}
