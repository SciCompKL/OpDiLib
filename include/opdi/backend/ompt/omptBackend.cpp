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

#include "omptBackend.hpp"

// static ompt backend members

ompt_wait_id_t* opdi::WaitIdExtractor::waitId = nullptr;
ompt_callback_t opdi::WaitIdExtractor::onAcquire = NULL;
ompt_callback_t opdi::WaitIdExtractor::onAcquired = NULL;
ompt_callback_t opdi::WaitIdExtractor::onReleased = NULL;

ompt_set_callback_t opdi::CallbacksBase::setCallback;
ompt_get_callback_t opdi::CallbacksBase::getCallback;

ompt_get_parallel_info_t opdi::OmptBackend::getParallelInfo = NULL;
ompt_finalize_tool_t opdi::OmptBackend::finalizeTool = NULL;

// ompt entry point

extern "C" ompt_start_tool_result_t* ompt_start_tool(unsigned int ompVersion, char const* runtimeVersion) {

  OPDI_UNUSED(ompVersion);
  OPDI_UNUSED(runtimeVersion);

  ompt_start_tool_result_t* startTool = new ompt_start_tool_result_t;
  startTool->initialize = opdi::OmptBackend::onInitialize;
  startTool->finalize = opdi::OmptBackend::onFinalize;
  opdi::backend = new opdi::OmptBackend;
  startTool->tool_data.ptr = (void*) opdi::backend;

  return startTool;
}

/* runtime */

#include "../runtime.cpp" // contains implementations that do not depend on the backend

namespace opdi {

  /* lock routines */

  void opdi_destroy_lock(omp_lock_t* lock) {
    omp_destroy_lock(lock);
  }

  void opdi_destroy_nest_lock(omp_nest_lock_t* lock) {
    omp_destroy_nest_lock(lock);
  }

  void opdi_set_lock(omp_lock_t* lock) {
    omp_set_lock(lock);
  }

  void opdi_set_nest_lock(omp_nest_lock_t* lock) {
    omp_set_nest_lock(lock);
  }

  void opdi_unset_lock(omp_lock_t* lock) {
    omp_unset_lock(lock);
  }

  void opdi_unset_nest_lock(omp_nest_lock_t* lock) {
    omp_unset_nest_lock(lock);
  }

  int opdi_test_lock(omp_lock_t* lock) {
    return omp_test_lock(lock);
  }

  int opdi_test_nest_lock(omp_nest_lock_t* lock) {
    return omp_test_nest_lock(lock);
  }
}
