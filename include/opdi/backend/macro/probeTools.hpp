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

#include "../../helpers/exceptions.hpp"

#include <omp.h>
#include <stack>

namespace opdi {

  struct ProbeScopeStatus {
    private:
      struct Status {
        public:
          bool insideImplicitTaskProbeScope;
          bool insideParallelReductionProbeScope;
          bool insideWorksharingOrLateParallelReductionProbeScope;

          Status() : insideImplicitTaskProbeScope(false),
                               insideParallelReductionProbeScope(false),
                               insideWorksharingOrLateParallelReductionProbeScope(false)
          {}

          Status(bool insideImplicitTaskProbeScope, bool insideParallelReductionProbeScope, bool insideWorksharingOrLateParallelReductionProbeScope)
            : insideImplicitTaskProbeScope(insideImplicitTaskProbeScope),
              insideParallelReductionProbeScope(insideParallelReductionProbeScope),
              insideWorksharingOrLateParallelReductionProbeScope(insideWorksharingOrLateParallelReductionProbeScope)
          {}
      };

      static std::stack<Status> statusStack;
      #pragma omp threadprivate(statusStack)

    public:
      static void beginImplicitTaskProbeScope() {
        // new parallel region, implicit task probe created first
        if ((size_t)omp_get_level() != statusStack.size()) {
          // possibly add dummy statuses
          while (statusStack.size() < (size_t)omp_get_level()) {
            statusStack.push(Status());
          }
          statusStack.top().insideImplicitTaskProbeScope = true;
        }
        // new parallel region, implicit task probe created second
        else {
          if (!statusStack.top().insideParallelReductionProbeScope) {
            OPDI_ERROR("unreachable");
          }

          statusStack.top().insideImplicitTaskProbeScope = true;
        }
      }

      static void endImplicitTaskProbeScope() {
        // no reduction probe or implicit task probe destructed second
        if (statusStack.top().insideParallelReductionProbeScope == false &&
            statusStack.top().insideWorksharingOrLateParallelReductionProbeScope == false) {
          if (!statusStack.top().insideImplicitTaskProbeScope) {
            OPDI_ERROR("unreachable");
          }

          statusStack.pop();

          // pop dummy statuses
          while (!statusStack.empty()
                 && statusStack.top().insideImplicitTaskProbeScope == false
                 && statusStack.top().insideParallelReductionProbeScope == false
                 && statusStack.top().insideWorksharingOrLateParallelReductionProbeScope == false) {
            statusStack.pop();
          }
        }
        else {
          statusStack.top().insideImplicitTaskProbeScope = false;
        }
      }

      static void beginReductionProbeScope() {
        // new parallel region, reduction probe created first
        if ((size_t)omp_get_level() != statusStack.size()) {
          while (statusStack.size() < (size_t)omp_get_level()) {
            statusStack.push(Status());
          }
          statusStack.top().insideParallelReductionProbeScope = true;
        }
        // implicit task probe created first
        else {
          if (!statusStack.top().insideImplicitTaskProbeScope) {
            OPDI_ERROR("unreachable");
          }

          // reduction on parallel region already detected, must be a reduction on a nested worksharing construct
          if (statusStack.top().insideParallelReductionProbeScope) {
            if (statusStack.top().insideWorksharingOrLateParallelReductionProbeScope) {
              OPDI_ERROR("unreachable");
            }

            statusStack.top().insideWorksharingOrLateParallelReductionProbeScope = true;
          }
          // two nested reduction scopes indicate that the first one was on the parallel region
          else if (statusStack.top().insideWorksharingOrLateParallelReductionProbeScope) {
            if (statusStack.top().insideParallelReductionProbeScope) {
              OPDI_ERROR("unreachable");
            }
            statusStack.top().insideParallelReductionProbeScope = true;
          }
          // no reduction indicated yet, could be either on the parallel region or a nested worksharing construct
          else {
            if (statusStack.top().insideWorksharingOrLateParallelReductionProbeScope) {
              OPDI_ERROR("unreachable");
            }
            statusStack.top().insideWorksharingOrLateParallelReductionProbeScope = true;
          }
        }
      }

      static void endReductionProbeScope() {
        // end of parallel region, reduction probe destructed second
        if (!statusStack.top().insideImplicitTaskProbeScope) {
          if (statusStack.top().insideParallelReductionProbeScope && statusStack.top().insideWorksharingOrLateParallelReductionProbeScope) {
            OPDI_ERROR("unreachable");
          }
          if (!statusStack.top().insideParallelReductionProbeScope && !statusStack.top().insideWorksharingOrLateParallelReductionProbeScope) {
            OPDI_ERROR("unreachable");
          }

          statusStack.pop();

          // pop dummy statuses
          while (!statusStack.empty()
                 && statusStack.top().insideImplicitTaskProbeScope == false
                 && statusStack.top().insideParallelReductionProbeScope == false
                 && statusStack.top().insideWorksharingOrLateParallelReductionProbeScope == false) {
            statusStack.pop();
          }
        }
        // worksharing reduction probe destructed or there were no worksharing reductions and the parallel reduction probe is destructed first
        else if (statusStack.top().insideWorksharingOrLateParallelReductionProbeScope) {
          statusStack.top().insideWorksharingOrLateParallelReductionProbeScope = false;
        }
        // parallel reduction probe is destructed first
        else if (statusStack.top().insideParallelReductionProbeScope) {
          if (statusStack.top().insideWorksharingOrLateParallelReductionProbeScope) {
            OPDI_ERROR("unreachable");
          }
          statusStack.top().insideParallelReductionProbeScope = false;
        }
        else {
          OPDI_ERROR("unreachable");
        }
      }

      static bool insideImplicitTaskProbeScope() {
        if (statusStack.empty()) {
          return false;
        }

        return statusStack.top().insideImplicitTaskProbeScope;
      }

      static bool insideParallelReductionProbeScope() {
        if (statusStack.empty()) {
          return false;
        }

        return statusStack.top().insideParallelReductionProbeScope;
      }

      static bool insideWorksharingOrLateParallelReductionProbeScope() {
        if (statusStack.empty()) {
          return false;
        }

        return statusStack.top().insideWorksharingOrLateParallelReductionProbeScope;
      }
  };
}
