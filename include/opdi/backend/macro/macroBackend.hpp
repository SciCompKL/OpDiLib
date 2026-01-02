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

#include <cassert>

#include "../../config.hpp"

#ifdef OPDI_BACKEND
  #error Please include only one backend.
#else
  #define OPDI_BACKEND OPDI_MACRO_BACKEND
#endif

#include "../../helpers/macros.hpp"

#include "../backendInterface.hpp"
#include "../runtime.hpp"

#include "dataTools.hpp"
#include "implicitBarrierTools.hpp"
#include "macros.hpp"
#include "mutexIdentifiers.hpp"
#include "probes.hpp"
#include "reductionTools.hpp"

namespace opdi {

  struct MacroBackend : public MutexIdentifiers,
                        public virtual BackendInterface
  {
    public:

      // remaining functions from backend interface

      void init() {
        // task data for initial implicit task is created in the logic layer
      }

      void finalize() {
        // pop task data associated with initial implicit task
        DataTools::popTaskData();
        assert(DataTools::getImplicitTaskData() == nullptr);
      }

      void* getParallelData() {
        return DataTools::getParallelData();
      }

      void* getImplicitTaskData() {
        return DataTools::getImplicitTaskData();
      }

      void setInitialImplicitTaskData(void* data) {
        assert(DataTools::getImplicitTaskData() == nullptr);
        DataTools::pushTaskData(data);
      }
  };

}
