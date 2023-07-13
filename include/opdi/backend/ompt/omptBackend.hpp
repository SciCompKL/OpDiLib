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

#include "../../config.hpp"

#ifdef OPDI_BACKEND
  #error Please include only one backend.
#else
  #define OPDI_BACKEND OPDI_OMPT_BACKEND
#endif

#include <iostream>

#include "../../helpers/exceptions.hpp"
#include "../../helpers/macros.hpp"

#include "../backendInterface.hpp"
#include "../runtime.hpp"

#include "implicitTaskCallbacks.hpp"
#include "macros.hpp"
#include "masterCallbacks.hpp"
#include "mutexCallbacks.hpp"
#include "parallelCallbacks.hpp"
#include "reductionCallbacks.hpp"
#include "syncRegionCallbacks.hpp"
#include "waitIdExtractor.hpp"
#include "workCallbacks.hpp"

namespace opdi {

  struct OmptBackend : public ImplicitTaskCallbacks,
                       public MasterCallbacks,
                       public MutexCallbacks,
                       public ParallelCallbacks,
                       public ReductionCallbacks,
                       public SyncRegionCallbacks,
                       public WaitIdExtractor,
                       public WorkCallbacks,
                       public virtual CallbacksBase,
                       public virtual BackendInterface
  {
    public:
      using LockIdentifier = ompt_wait_id_t;

    private:

      // runtime entry points to be queried in addition to the ones stored in CallbacksBase

      static ompt_get_parallel_info_t getParallelInfo;
      static ompt_finalize_tool_t finalizeTool;

    public:
      // callbacks for initialization and finalization

      static int onInitialize(
        ompt_function_lookup_t lookup,
        int initialDeviceNum,
        ompt_data_t* toolData) {

        OPDI_UNUSED(initialDeviceNum);
        OPDI_UNUSED(toolData);

        // look up runtime entry points
        ompt_set_callback_t setCallback = (ompt_set_callback_t) lookup("ompt_set_callback");
        ompt_get_callback_t getCallback = (ompt_get_callback_t) lookup("ompt_get_callback");
        OmptBackend::getParallelInfo = (ompt_get_parallel_info_t) lookup("ompt_get_parallel_info");
        OmptBackend::finalizeTool = (ompt_finalize_tool_t) lookup("ompt_finalize_tool");

        // initialize base
        // stores setCallback, getCallback
        CallbacksBase::init(setCallback, getCallback);

        // initialize callback structures
        ParallelCallbacks::init();
        ImplicitTaskCallbacks::init();
        WorkCallbacks::init();
        SyncRegionCallbacks::init();
        MutexCallbacks::init();
        ReductionCallbacks::init();
        MasterCallbacks::init();

        return 1; // success
      }

      static void onFinalize(ompt_data_t* toolData) {

        OPDI_UNUSED(toolData);

        // finalize callback structures
        MasterCallbacks::finalize();
        ReductionCallbacks::finalize();
        MutexCallbacks::finalize();
        SyncRegionCallbacks::finalize();
        WorkCallbacks::finalize();
        ImplicitTaskCallbacks::finalize();
        ParallelCallbacks::finalize();
      }

      // functions from backend interface

      void init() {}

      void finalize() {
        OmptBackend::finalizeTool();
      }

      std::size_t getLockIdentifier(omp_lock_t* lock) {

        ompt_wait_id_t waitId;
        WaitIdExtractor::begin(&waitId);
        omp_set_lock(lock);
        omp_unset_lock(lock);
        WaitIdExtractor::end();

        return (std::size_t) waitId;
      }

      std::size_t getNestedLockIdentifier(omp_nest_lock_t* lock) {

        ompt_wait_id_t waitId;
        WaitIdExtractor::begin(&waitId);
        omp_set_nest_lock(lock);
        omp_unset_nest_lock(lock);
        WaitIdExtractor::end();

        return (std::size_t) waitId;
      }

      void* getParallelData() {
        ompt_data_t* parallelData;
        int teamSize;

        int result = getParallelInfo(0, &parallelData, &teamSize);

        if (result != 2) {
          return nullptr;
        }

        return parallelData->ptr;
      }
  };
}

// declaration of ompt entry point
extern "C" ompt_start_tool_result_t* ompt_start_tool(unsigned int ompVersion, char const* runtimeVersion);
