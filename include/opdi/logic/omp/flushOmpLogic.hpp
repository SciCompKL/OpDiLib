/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2021 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Homepage: http://www.scicomp.uni-kl.de
 * Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
 *
 * Lead developer: Johannes Blühdorn (SciComp, TU Kaiserslautern)
 *
 * This file is part of OpDiLib (http://www.scicomp.uni-kl.de/software/opdi).
 *
 * OpDiLib is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OpDiLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU
 * General Public License along with OpDiLib.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * For other licensing options please contact us.
 *
 */

#pragma once

#include "../../config.hpp"
#include "../../misc/tapedOutput.hpp"
#include "../../tool/toolInterface.hpp"

#include "../logicInterface.hpp"

namespace opdi {

  struct FlushOmpLogic : public virtual LogicInterface {
    private:

      static void reverseFunc(void*) {

        #if OPDI_LOGIC_OUT & OPDI_FLUSH_OUT
          TapedOutput::print("FLSH i", omp_get_thread_num());
        #endif

        #pragma omp flush
      }

    public:

      virtual void addReverseFlush() {
        Handle* handle = new Handle;
        handle->reverseFunc = FlushOmpLogic::reverseFunc;
        tool->pushExternalFunction(tool->getThreadLocalTape(), handle);
      }
  };
}
