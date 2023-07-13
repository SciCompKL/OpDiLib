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

#include "../../logic/logicInterface.hpp"

#include "macroBackend.hpp"

// static macro backend members

std::stack<void*> opdi::DataTools::parallelData;

std::stack<bool> opdi::ImplicitBarrierTools::implicitBarrierStack;

omp_lock_t opdi::ReductionTools::globalReducerLock;
std::list<omp_nest_lock_t*> opdi::ReductionTools::individualReducerLocks;
std::stack<bool> opdi::ReductionTools::reductionBarrierStack;

template<typename Type, int identifier>
omp_nest_lock_t opdi::Reducer<Type, identifier>::reduceLock;

template<typename Type, int identifier>
bool opdi::Reducer<Type, identifier>::isInitialized = false;

std::stack<opdi::ProbeScopeStatus::Status> opdi::ProbeScopeStatus::statusStack;

// global macro backend variables

opdi::LoopProbe opdi::internalLoopProbe(0);
opdi::SectionsProbe opdi::internalSectionsProbe(0);
opdi::SingleProbe opdi::internalSingleProbe(0);
opdi::ReductionProbe opdi::internalReductionProbe(0);
opdi::NowaitProbe opdi::internalNowaitProbe(0);

/* runtime */

#include "../runtime.cpp" // contains implementations that do not depend on the backend

namespace opdi {

  /* lock routines */

  void opdi_destroy_lock(omp_lock_t* lock) {
    omp_destroy_lock(lock);
    opdi::logic->onMutexDestroyed(LogicInterface::MutexKind::Lock, opdi::backend->getLockIdentifier(lock));
  }

  void opdi_destroy_nest_lock(omp_nest_lock_t* lock) {
    omp_destroy_nest_lock(lock);
    opdi::logic->onMutexDestroyed(LogicInterface::MutexKind::NestedLock,
                                  opdi::backend->getNestedLockIdentifier(lock));
  }

  void opdi_set_lock(omp_lock_t* lock) {
    omp_set_lock(lock);
    opdi::logic->onMutexAcquired(LogicInterface::MutexKind::Lock, opdi::backend->getLockIdentifier(lock));
  }

  void opdi_set_nest_lock(omp_nest_lock_t* lock) {
    omp_set_nest_lock(lock);
    int lockCount = omp_test_nest_lock(lock); // user triggered locks plus lock caused by test
    if (lockCount == 2) {
      opdi::logic->onMutexAcquired(LogicInterface::MutexKind::NestedLock,
                                   opdi::backend->getNestedLockIdentifier(lock));
    }
    omp_unset_nest_lock(lock); // revert lock caused by test
  }

  void opdi_unset_lock(omp_lock_t* lock) {
    opdi::logic->onMutexReleased(LogicInterface::MutexKind::Lock, opdi::backend->getLockIdentifier(lock));
    omp_unset_lock(lock);
  }

  void opdi_unset_nest_lock(omp_nest_lock_t* lock) {
    int lockCount = omp_test_nest_lock(lock); // user triggered locks plus lock caused by test
    if (lockCount == 2) {
      opdi::logic->onMutexReleased(LogicInterface::MutexKind::NestedLock,
                                   opdi::backend->getNestedLockIdentifier(lock));
    }
    omp_unset_nest_lock(lock); // revert lock caused by test
    omp_unset_nest_lock(lock); // user triggered unlock
  }

  int opdi_test_lock(omp_lock_t* lock) {
    int result = omp_test_lock(lock);
    if (result) {
      opdi::logic->onMutexAcquired(LogicInterface::MutexKind::Lock, opdi::backend->getLockIdentifier(lock));
    }
    return result;
  }

  int opdi_test_nest_lock(omp_nest_lock_t* lock) {
    int lockCount = omp_test_nest_lock(lock);
    if (lockCount == 1) {
      opdi::logic->onMutexAcquired(LogicInterface::MutexKind::NestedLock,
                                   opdi::backend->getNestedLockIdentifier(lock));
    }
    return lockCount;
  }
}
