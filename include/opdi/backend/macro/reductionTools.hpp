/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
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

#include <iostream>
#include <list>
#include <omp.h>
#include <stack>
#include <string>

#include "../../logic/logicInterface.hpp"

#include "../runtime.hpp"

#include "probeTools.hpp"

namespace opdi {

  struct ReductionTools {
    public:
      static omp_lock_t globalReducerLock;
      static std::list<omp_nest_lock_t*> individualReducerLocks;

      static std::stack<bool> reductionBarrierStack;
      #pragma omp threadprivate(reductionBarrierStack)

      static void beginRegionWithReduction() {
        ReductionTools::reductionBarrierStack.push(false);
      }

      static void endRegionWithReduction() {
        if (ReductionTools::reductionBarrierStack.top() == false) {
          OPDI_ERROR("reduction barrier missing at end of region");
        }
        ReductionTools::reductionBarrierStack.pop();
        if (ProbeScopeStatus::insideImplicitTaskProbeScope()) {
          #if defined(__GNUC__) && !defined(__clang__)
            opdi_set_lock(&globalReducerLock);
            opdi_unset_lock(&globalReducerLock);
          #else
            logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplementation,
                                LogicInterface::ScopeEndpoint::End);
          #endif
        }
      }

      static void addBarrierIfNeeded() {
        if (ReductionTools::reductionBarrierStack.top() == false) {
          if (ProbeScopeStatus::insideImplicitTaskProbeScope()) {
            #if defined(__GNUC__) && !defined(__clang__)
              opdi_set_lock(&globalReducerLock);
              opdi_unset_lock(&globalReducerLock);
            #else
              logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplementation,
                                  LogicInterface::ScopeEndpoint::Begin);
            #endif
          }
          ReductionTools::reductionBarrierStack.top() = true;
        }
      }
  };

  template<typename Type, int identifier>
  struct Reducer {
    public:
      static omp_nest_lock_t reduceLock;
      static bool isInitialized;

      Type& value;

      void checkInitialized() {

        bool initialized;
        #pragma omp atomic read
        initialized = Reducer::isInitialized;

        if (!initialized) {

          opdi_set_lock(&ReductionTools::globalReducerLock);

          #pragma omp atomic read
          initialized = Reducer::isInitialized;

          if (!initialized) {
            opdi_init_nest_lock(&Reducer::reduceLock);
            ReductionTools::individualReducerLocks.push_back(&Reducer::reduceLock);

            #pragma omp atomic write
            Reducer::isInitialized = true;
          }

          opdi_unset_lock(&ReductionTools::globalReducerLock);
        }
      }

      Reducer(Type& value) : value(value) {
        ReductionTools::addBarrierIfNeeded();
        this->checkInitialized();
        opdi_set_nest_lock(&reduceLock);
      }

      Reducer& operator=(Type const& rhs) {
        value = rhs;
        opdi_unset_nest_lock(&reduceLock);
        opdi_unset_nest_lock(&reduceLock);
        opdi_unset_nest_lock(&reduceLock);
        return *this;
      }
  };
}
