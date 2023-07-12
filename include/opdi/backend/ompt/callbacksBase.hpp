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

#include <omp-tools.h>

namespace opdi {

  struct CallbacksBase {
    private:
      static ompt_set_callback_t setCallback;
      static ompt_get_callback_t getCallback;

    protected:

      static bool init(ompt_set_callback_t setCallback, ompt_get_callback_t getCallback) {
        CallbacksBase::setCallback = setCallback;
        CallbacksBase::getCallback = getCallback;
        return true;
      }

      static bool registerCallback(ompt_callbacks_t whichCallback, ompt_callback_t callback) {

        ompt_set_result_t result = CallbacksBase::setCallback(whichCallback, callback);
        return (result == ompt_set_always);
      }

      static void queryCallback(ompt_callbacks_t whichCallback, ompt_callback_t* callback) {

        int hasCallback = CallbacksBase::getCallback(whichCallback, callback);
        if (!hasCallback) {
          *callback = NULL;
        }
      }

      static bool clearCallback(ompt_callbacks_t whichCallback) {
        ompt_set_result_t result = CallbacksBase::setCallback(whichCallback, NULL);
        return (result == ompt_set_always);
      }
  };
}
