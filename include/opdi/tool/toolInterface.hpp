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

#include <string>

#include "helpers/handle.hpp"

namespace opdi {
  
  struct ToolInterface {
    public:

      virtual ~ToolInterface() {}

      // initialization and finalization
      virtual void init() = 0;
      virtual void finalize() = 0;

      // tape creation and deletion

      virtual void* createTape() = 0;
      virtual void deleteTape(void* tape) = 0;

      // management of thread local tapes

      virtual void* getThreadLocalTape() = 0;
      virtual void setThreadLocalTape(void* tape) = 0;

      // position handling

      virtual void* allocPosition() = 0;
      virtual void freePosition(void* position) = 0;
      virtual size_t getPositionSize() = 0;
      virtual std::string positionToString(void* position) = 0;
      virtual void getTapePosition(void* tape, void* position) = 0;
      virtual void getZeroPosition(void* tape, void* position) = 0;
      virtual void copyPosition(void* dst, void* src) = 0;
      virtual int comparePosition(void* lhs, void* rhs) = 0; // follows the <=> return rules

      // tape handling

      virtual bool isActive(void* tape) = 0;
      virtual void setActive(void* tape, bool active) = 0;

      virtual void evaluate(void* tape, void* start, void* end, bool useAtomics = true) = 0;
      virtual void reset(void* tape, bool clearAdjoints = true) = 0;
      virtual void reset(void* tape, void* position, bool clearAdjoints = true) = 0;
      
      virtual void pushExternalFunction(void* tape, Handle const* handle) = 0;

      // tape editing

      virtual void erase(void* tape, void* start, void* end) = 0;
      virtual void append(void* dstTape, void* srcTape, void* start, void* end) = 0;
  };

  // pointer must be set by the user to an instance of a proper AD tool implementation
  extern ToolInterface* tool;
}
