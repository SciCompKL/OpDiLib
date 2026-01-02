/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2026 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
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
#include <omp.h>
#include <omp-tools.h>

#include "../../helpers/exceptions.hpp"
#include "../../helpers/macros.hpp"
#include "../../logic/logicInterface.hpp"

#include "callbacksBase.hpp"

namespace opdi {

  struct SyncRegionCallbacks : public virtual CallbacksBase {
    private:

      // callbacks to be registered

      static void onSyncRegion(
          ompt_sync_region_t kind,
          ompt_scope_endpoint_t _endpoint,
          ompt_data_t* parallelData,
          ompt_data_t* taskData,
          void const* codeptr) {

        OPDI_UNUSED(parallelData);
        OPDI_UNUSED(codeptr);

        #if OPDI_OMPT_BACKEND_IMPLICIT_TASK_END_SOURCE != OPDI_SYNC_REGION_END
          OPDI_UNUSED(taskData);
        #endif

        LogicInterface::ScopeEndpoint endpoint;
        if (ompt_scope_begin == _endpoint) {
          endpoint = LogicInterface::ScopeEndpoint::Begin;
        }
        else {
          endpoint = LogicInterface::ScopeEndpoint::End;
        }

        switch (kind) {
          case ompt_sync_region_barrier:
            logic->onSyncRegion(LogicInterface::SyncRegionKind::Barrier, endpoint);
            break;
          case ompt_sync_region_barrier_implicit:
        #if _OPENMP >= 202011
          case ompt_sync_region_barrier_implicit_workshare:
        #else  // fallback for compilers with _OPENMP < 202011 that already support fine-grained sync region types
          case 8:  // ompt_sync_region_barrier_implicit_workshare
        #endif
            logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplicit, endpoint);
            break;
          case ompt_sync_region_barrier_explicit:
            logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierExplicit, endpoint);
            break;
          case ompt_sync_region_barrier_implementation:
          #if OPDI_OMPT_BACKEND_BARRIER_IMPLEMENTATION_BEHAVIOUR == OPDI_PAIR_OF_AD_EVENTS_PER_ENDPOINT
            logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplementation, LogicInterface::ScopeEndpoint::Begin);
            logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplementation, LogicInterface::ScopeEndpoint::End);
          #else
            logic->onSyncRegion(LogicInterface::SyncRegionKind::BarrierImplementation, endpoint);
          #endif
            break;
        #if _OPENMP >= 202011
          case ompt_sync_region_barrier_implicit_parallel:
        #else  // fallback for compilers with _OPENMP < 202011 that already support fine-grained sync region types
          case 9:  // ompt_sync_region_barrier_implicit_parallel
        #endif
            // implicit barriers on parallel regions themselves do not require treatment
            // however, we optionally use this to generate ImplicitTaskEnd events of non-master threads
          #if OPDI_OMPT_BACKEND_IMPLICIT_TASK_END_SOURCE == OPDI_OMPT_SYNC_REGION_END
            if (LogicInterface::ScopeEndpoint::Begin == endpoint && omp_get_thread_num() != 0) {
              opdi::logic->onImplicitTaskEnd(taskData->ptr);
            }
          #endif
            break;  // no treatment needed
          case ompt_sync_region_reduction: // does not occur in this callback
            OPDI_WARNING("Unexpected kind argument ompt_sync_region_reduction.");
            break;
          case ompt_sync_region_taskwait:  // not supported, no AD handling
          case ompt_sync_region_taskgroup:
            break;
          default:
            OPDI_WARNING("Unknown kind argument.");
            break;
        }
      }

    protected:

      static void init() {

        OPDI_CHECK_ERROR(CallbacksBase::registerCallback(ompt_callback_sync_region,
                                                         (ompt_callback_t) SyncRegionCallbacks::onSyncRegion));
      }

      static void finalize() {

        OPDI_CHECK_ERROR(CallbacksBase::clearCallback(ompt_callback_sync_region));
      }
  };
}
