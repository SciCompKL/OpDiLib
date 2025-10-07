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

#include <iostream>
#include <list>
#include <omp.h>
#include <stack>
#include <string>

#include "../../logic/logicInterface.hpp"

#include "../runtime.hpp"

namespace opdi {

  struct ReductionTools {
    public:
      static omp_lock_t globalReductionLock;
      static std::list<omp_nest_lock_t*> individualReductionLocks;

      /* item indicates a construct that might have a reduction clause */
      /* its value indicates whether there is a reduction clause */
      static std::stack<bool> hasReductions;
      #pragma omp threadprivate(hasReductions)

      /* item indicates a construct that might have a reduction clause */
      /* its value indicates whether the barrier before reductions (still) needs to be added */
      static std::stack<bool> needsBarrierBeforeReductions;
      #pragma omp threadprivate(needsBarrierBeforeReductions)

      /* item indicates a construct that might have a reduction clause */
      /* its value indicates whether a barrier after reductions is required */
      static std::stack<bool> needsBarrierAfterReductions;
      #pragma omp threadprivate(needsBarrierAfterReductions)

      /* resolves ordering issues between TaskProbe and ReductionProbe constructors */
      static int implicitTaskNestingDepth;
      #pragma omp threadprivate(implicitTaskNestingDepth)

      static void beginRegionThatSupportsReductions(bool needsBarrierAfterReductions) {
        ReductionTools::hasReductions.push(false);
        ReductionTools::needsBarrierBeforeReductions.push(false);
        ReductionTools::needsBarrierAfterReductions.push(needsBarrierAfterReductions);
      }

      static void endRegionThatSupportsReductions() {
        /* regards threads that did not participate in the reduction */
        ReductionTools::addBarrierBeforeReductionsIfNeeded();

        if (ReductionTools::hasReductions.top() && ReductionTools::needsBarrierAfterReductions.top()) {

          /* add barrier after reductions */
          logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplementation,
                              LogicInterface::ScopeEndpoint::Begin);
          /* actual barrier to separate subsequent worksharing constructs with a reduction, in particular if the first
           * one uses nowait; otherwise risk of invalid order of locks and this reverse-only barrier */
          #pragma omp barrier
          logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplementation,
                              LogicInterface::ScopeEndpoint::End);

          if (ReductionTools::needsBarrierBeforeReductions.top() == true) {
            OPDI_ERROR("barrier missing before reductions");
          }
        }

        ReductionTools::hasReductions.pop();
        ReductionTools::needsBarrierBeforeReductions.pop();
        ReductionTools::needsBarrierAfterReductions.pop();
      }

      static void regionHasReductions() {
        ReductionTools::hasReductions.top() = true;
        ReductionTools::needsBarrierBeforeReductions.top() = true;
        /* needsBarrierAfterReductions is indicated as a parameter to beginRegionThatSupportsReductions */
      }

      static void addBarrierBeforeReductionsIfNeeded() {
        if (ReductionTools::needsBarrierBeforeReductions.top()) {
          logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplementation,
                              LogicInterface::ScopeEndpoint::Begin);
          logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplementation,
                              LogicInterface::ScopeEndpoint::End);
          ReductionTools::needsBarrierBeforeReductions.top() = false;
        }
      }
  };

  template<typename Type, int identifier>
  struct Reducer {
    public:
      static omp_nest_lock_t reductionLock;
      static bool isInitialized;

      Type& value;

      void checkInitialized() {

        bool initialized;
        #pragma omp atomic read
        initialized = Reducer::isInitialized;

        if (!initialized) {

          opdi_set_lock(&ReductionTools::globalReductionLock);

          #pragma omp atomic read
          initialized = Reducer::isInitialized;

          if (!initialized) {
            opdi_init_nest_lock(&Reducer::reductionLock);
            ReductionTools::individualReductionLocks.push_back(&Reducer::reductionLock);

            #pragma omp atomic write
            Reducer::isInitialized = true;
          }

          opdi_unset_lock(&ReductionTools::globalReductionLock);
        }
      }

      Reducer(Type& value) : value(value) {
        /* push barrier prior to first reduction-related operation */
        ReductionTools::addBarrierBeforeReductionsIfNeeded();
        this->checkInitialized();
        opdi_set_nest_lock(&reductionLock);
      }

      Reducer& operator=(Type const& rhs) {
        value = rhs;
        opdi_unset_nest_lock(&reductionLock);
        opdi_unset_nest_lock(&reductionLock);
        opdi_unset_nest_lock(&reductionLock);
        return *this;
      }
  };
}
