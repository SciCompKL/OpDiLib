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

#include <sstream>

#include "../tool/toolInterface.hpp"

#include "output.hpp"

namespace opdi {

  struct TapedOutput : public Output {
    private:

      static void reverseActivate(void*) {
        TapedOutput::setActive(true);
      }

      static void reverseDeactivate(void*) {
        TapedOutput::setActive(false);
      }

      struct ReversePrintData {
        public:
          std::string message;
      };

      static void reversePrint(void* dataPtr) {
        ReversePrintData* data = static_cast<ReversePrintData*>(dataPtr);
        print(data->message);
      }

      static void reversePrintDelete(void* dataPtr) {
        ReversePrintData* data = static_cast<ReversePrintData*>(dataPtr);
        delete data;
      }

      static bool active;

    public:

      TapedOutput() {}
      virtual ~TapedOutput() {}

      // meant to be called outside parallel regions
      static void setActive(bool active) {
        TapedOutput::active = active;

        Handle* handle = new Handle;
        handle->reverseFunc = (active) ? (TapedOutput::reverseDeactivate) : (TapedOutput::reverseActivate);

        tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
      }

      template<typename... Args>
      static void print(Args const&... args) {
        if (TapedOutput::active) {
          Output::print(args...);
        }
      }

      template<typename... Args>
      static void printReverse(Args const&... args) {
        if (TapedOutput::active) {
          std::stringstream out;
          Output::printRec(out, args...);

          ReversePrintData* data = new ReversePrintData;
          data->message = out.rdbuf()->str();

          Handle* handle = new Handle;
          handle->data = data;
          handle->reverseFunc = reversePrint;
          handle->deleteFunc = reversePrintDelete;

          tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
        }
      }
  };
}
