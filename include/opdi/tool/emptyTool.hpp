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

#include "toolInterface.hpp"

namespace opdi {

  struct EmptyTool : public ToolInterface {
    public:

      // initialization and finalization
      void init() {}
      void finalize() {}

      // tape creation and deletion

      void* createTape() {
        return nullptr;
      }

      void deleteTape(void* tape) {
        OPDI_UNUSED(tape);
      }

      // management of thread local tapes

      void* getThreadLocalTape() {
        return nullptr;
      }

      void setThreadLocalTape(void* tape) {
        OPDI_UNUSED(tape);
      }

      // position handling

      void* allocPosition() {
        return nullptr;
      }

      void freePosition(void* position) {
        OPDI_UNUSED(position);
      }

      size_t getPositionSize() {
        return 0;
      }

      std::string positionToString(void* position) {
        OPDI_UNUSED(position);
        return "";
      }

      void getTapePosition(void* tape, void* position) {
        OPDI_UNUSED(tape);
        OPDI_UNUSED(position);
      }

      void getZeroPosition(void* tape, void* position) {
        OPDI_UNUSED(tape);
        OPDI_UNUSED(position);
      }

      void copyPosition(void* dst, void* src) {
        OPDI_UNUSED(dst);
        OPDI_UNUSED(src);
      }

      int comparePosition(void* lhs, void* rhs) {
        OPDI_UNUSED(lhs);
        OPDI_UNUSED(rhs);
        return 0;
      }

      // tape handling

      bool isActive(void* tape) {
        OPDI_UNUSED(tape);
        return false;
      }

      void setActive(void* tape, bool active) {
        OPDI_UNUSED(tape);
        OPDI_UNUSED(active);
      }

      void evaluate(void* tape, void* start, void* end, bool useAtomics = true) {
        OPDI_UNUSED(tape);
        OPDI_UNUSED(start);
        OPDI_UNUSED(end);
        OPDI_UNUSED(useAtomics);
      }

      void reset(void* tape, bool clearAdjoints = true) {
        OPDI_UNUSED(tape);
        OPDI_UNUSED(clearAdjoints);
      }

      void reset(void* tape, void* position, bool clearAdjoints = true) {
        OPDI_UNUSED(tape);
        OPDI_UNUSED(position);
        OPDI_UNUSED(clearAdjoints);
      }

      void pushExternalFunction(void* tape, Handle const* handle) {
        OPDI_UNUSED(tape);
        OPDI_UNUSED(handle);
      }

      // tape editing

      void erase(void* tape, void* start, void* end) {
        OPDI_UNUSED(tape);
        OPDI_UNUSED(start);
        OPDI_UNUSED(end);
      }

      void append(void* dstTape, void* srcTape, void* start, void* end) {
        OPDI_UNUSED(dstTape);
        OPDI_UNUSED(srcTape);
        OPDI_UNUSED(start);
        OPDI_UNUSED(end);
      }
  };
}
