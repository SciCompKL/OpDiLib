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

#include <cstdlib>
#include <omp.h>

namespace opdi {

  struct BackendInterface {
    public:

      virtual ~BackendInterface() {}

      virtual void init() = 0;
      virtual void finalize() = 0;

      virtual std::size_t getLockIdentifier(omp_lock_t* lock) = 0;
      virtual std::size_t getNestLockIdentifier(omp_nest_lock_t* lock) = 0;
      virtual std::size_t getCriticalIdentifier(std::string const& name) = 0;
      virtual std::size_t getReductionIdentifier() = 0;
      virtual std::size_t getOrderedIdentifier() = 0;

      virtual void* getParallelData() = 0;
      virtual void* getImplicitTaskData() = 0;

      virtual void setInitialImplicitTaskData(void* data) = 0;
  };

  extern BackendInterface* backend;
}
