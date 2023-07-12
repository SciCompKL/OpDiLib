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

#include "../../backend/backendInterface.hpp"
#include "../../misc/tapedOutput.hpp"

#include "../logicInterface.hpp"

#include "adjointAccessControl.hpp"
#include "flushOmpLogic.hpp"
#include "implicitTaskOmpLogic.hpp"
#include "masterOmpLogic.hpp"
#include "mutexOmpLogic.hpp"
#include "parallelOmpLogic.hpp"
#include "syncRegionOmpLogic.hpp"
#include "workOmpLogic.hpp"

namespace opdi {

  struct OmpLogic : public FlushOmpLogic,
                    public ImplicitTaskOmpLogic,
                    public MasterOmpLogic,
                    public MutexOmpLogic,
                    public ParallelOmpLogic,
                    public SyncRegionOmpLogic,
                    public WorkOmpLogic,
                    public virtual AdjointAccessControl,
                    public virtual LogicInterface
  {
    public:

      virtual ~OmpLogic() {}

      virtual void init() {

        MutexOmpLogic::internalInit();
        ImplicitTaskOmpLogic::internalInit();

        // this is important to avoid deadlocks with the ompt backend
        MutexOmpLogic::registerInactiveMutex(MutexKind::Lock, backend->getLockIdentifier(ImplicitTaskOmpLogic::tapePool.getInternalLock()));

        TapedOutput::init();
        MutexOmpLogic::registerInactiveMutex(MutexKind::Lock, backend->getLockIdentifier(&(TapedOutput::lock)));
      }

      virtual void finalize() {
        MutexOmpLogic::internalFinalize();
        ImplicitTaskOmpLogic::internalFinalize();
        TapedOutput::finalize();
      }

      virtual void prepareEvaluate() {
        MutexOmpLogic::prepareEvaluate();
      }

      virtual void reset() {
        MutexOmpLogic::reset();
      }
  };
}
