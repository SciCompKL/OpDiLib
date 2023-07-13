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

#include "../../helpers/exceptions.hpp"
#include "../../helpers/macros.hpp"
#include "../../logic/logicInterface.hpp"

#include "callbacksBase.hpp"

namespace opdi {

  struct MasterCallbacks : public virtual CallbacksBase {

    private:

      static void onMaster(ompt_scope_endpoint_t _endpoint,
                           ompt_data_t* parallelData,
                           ompt_data_t* taskData,
                           void const* codeptr) {
        OPDI_UNUSED(parallelData);
        OPDI_UNUSED(taskData);
        OPDI_UNUSED(codeptr);

        LogicInterface::ScopeEndpoint endpoint;
        if (ompt_scope_begin == _endpoint) {
          endpoint = LogicInterface::ScopeEndpoint::Begin;
        }
        else {
          endpoint = LogicInterface::ScopeEndpoint::End;
        }

        logic->onMaster(endpoint);
      }

    protected:

      static void init() {
        OPDI_CHECK_ERROR(CallbacksBase::registerCallback(ompt_callback_master,
                                                        (ompt_callback_t) MasterCallbacks::onMaster));
      }

      static void finalize() {
        OPDI_CHECK_ERROR(CallbacksBase::clearCallback(ompt_callback_master));
      }

  };

}
