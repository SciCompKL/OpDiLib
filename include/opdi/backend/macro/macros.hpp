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

#include <omp.h>

#include "../../helpers/macros.hpp"

#include "implicitBarrierTools.hpp"
#include "mutexIdentifiers.hpp"
#include "probes.hpp"
#include "reductionTools.hpp"

// macros that come in pairs

#define OPDI_PARALLEL(...) \
  { \
    void* opdiInternalParallelData = opdi::logic->onParallelBegin(omp_get_max_threads()); \
    opdi::TaskProbe opdiInternalTaskProbe(opdiInternalParallelData); \
    OPDI_PRAGMA(omp parallel __VA_ARGS__ firstprivate(opdiInternalTaskProbe))

#define OPDI_END_PARALLEL \
    opdi::logic->onParallelEnd(opdiInternalParallelData); \
  }

#define OPDI_FOR(...) \
  opdi::ImplicitBarrierTools::beginRegionWithImplicitBarrier(); \
  OPDI_PRAGMA(omp for __VA_ARGS__ private(opdi::internalLoopProbe))

#define OPDI_END_FOR \
  opdi::ImplicitBarrierTools::endRegionWithImplicitBarrier();

#define OPDI_SECTIONS(...) \
  opdi::ImplicitBarrierTools::beginRegionWithImplicitBarrier(); \
  OPDI_PRAGMA(omp sections private(opdi::internalSectionsProbe) __VA_ARGS__)

#define OPDI_END_SECTIONS \
  opdi::ImplicitBarrierTools::endRegionWithImplicitBarrier();

#define OPDI_SINGLE(...) \
  { \
    bool constexpr opdiInternalBarrierIndicator = true; \
    void* opdiInternalTapePosition1 = opdi::tool->allocPosition(); \
    opdi::tool->getTapePosition(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition1); \
    opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierImplementation, \
                              opdi::LogicInterface::ScopeEndpoint::Begin); \
    void* opdiInternalTapePosition2 = opdi::tool->allocPosition(); \
    opdi::tool->getTapePosition(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition2); \
    opdi::ImplicitBarrierTools::beginRegionWithImplicitBarrier(); \
    { \
      opdi::SingleProbe localSingleProbe; \
      OPDI_PRAGMA(omp single __VA_ARGS__) \
      { \
        opdi::tool->erase(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition1, opdiInternalTapePosition2);

#define OPDI_SINGLE_NOWAIT(...) \
  { \
    bool constexpr opdiInternalBarrierIndicator = false; \
    void* opdiInternalTapePosition = opdi::tool->allocPosition(); \
    opdi::tool->getTapePosition(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition); \
    opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierImplementation, \
                              opdi::LogicInterface::ScopeEndpoint::Begin); \
    opdi::ImplicitBarrierTools::beginRegionWithImplicitBarrier(); \
    { \
      opdi::SingleProbe localSingleProbe; \
      OPDI_PRAGMA(omp single nowait __VA_ARGS__) \
      { \
        opdi::tool->reset(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition);

#define OPDI_END_SINGLE \
        opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierImplementation, \
                                  opdi::LogicInterface::ScopeEndpoint::Begin); \
      } \
      opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierImplementation, \
                                opdi::LogicInterface::ScopeEndpoint::End); \
    } \
    opdi::ImplicitBarrierTools::implicitBarrierStack.top() = opdiInternalBarrierIndicator; \
    opdi::ImplicitBarrierTools::endRegionWithImplicitBarrier(); \
  }

#define OPDI_NOWAIT nowait private(opdi::internalNowaitProbe)

#define OPDI_CRITICAL(...) \
  OPDI_PRAGMA(omp critical __VA_ARGS__) \
  { \
    std::size_t constexpr opdiInternalCriticalIdentifier = 0; \
    opdi::logic->onMutexAcquired(opdi::LogicInterface::MutexKind::Critical, opdiInternalCriticalIdentifier);

#define OPDI_CRITICAL_NAME(name, ...) \
  OPDI_PRAGMA(omp critical (name) __VA_ARGS__) \
  { \
    std::size_t const opdiInternalCriticalIdentifier = \
        dynamic_cast<opdi::MacroBackend*>(opdi::backend)->getCriticalIdentifier(std::string(#name)); \
    opdi::logic->onMutexAcquired(opdi::LogicInterface::MutexKind::Critical, opdiInternalCriticalIdentifier);

#define OPDI_END_CRITICAL \
    opdi::logic->onMutexReleased(opdi::LogicInterface::MutexKind::Critical, opdiInternalCriticalIdentifier); \
  }

#define OPDI_ORDERED(...) \
  OPDI_PRAGMA(omp ordered __VA_ARGS__) \
  { \
    opdi::logic->onMutexAcquired(opdi::LogicInterface::MutexKind::Ordered, 0);

#define OPDI_END_ORDERED \
    opdi::logic->onMutexReleased(opdi::LogicInterface::MutexKind::Ordered, 0); \
  }

#define OPDI_SECTION(...) \
  OPDI_PRAGMA(omp section __VA_ARGS__)

#define OPDI_END_SECTION

#define OPDI_MASTER(...) \
  OPDI_PRAGMA(omp master __VA_ARGS__) \
  { \
    opdi::logic->onMaster(opdi::LogicInterface::ScopeEndpoint::Begin);

#define OPDI_END_MASTER \
    opdi::logic->onMaster(opdi::LogicInterface::ScopeEndpoint::End); \
  }

// standalone macros

#define OPDI_BARRIER(...) \
  opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierExplicit, \
                            opdi::LogicInterface::ScopeEndpoint::Begin); \
  OPDI_PRAGMA(omp barrier __VA_ARGS__) \
  opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierExplicit, \
                            opdi::LogicInterface::ScopeEndpoint::End);

// reduction macros

#define OPDI_INTERNAL_DECLARE_REDUCTION(OP_NAME, TYPE, OP, INIT, ID) \
  TYPE operator OP (opdi::Reducer<TYPE, ID> const& lhs, \
                    opdi::Reducer<TYPE, ID> const& rhs) { \
    return lhs.value OP rhs.value; \
  } \
  \
  OPDI_PRAGMA(omp declare reduction(OP_NAME : TYPE : \
      opdi::Reducer<TYPE, ID>(omp_out) = opdi::Reducer<TYPE, ID>(omp_out) OP opdi::Reducer<TYPE, ID>(omp_in)) \
      initializer(omp_priv = INIT))

#define OPDI_DECLARE_REDUCTION(OP_NAME, TYPE, OP, INIT) \
  OPDI_INTERNAL_DECLARE_REDUCTION(OP_NAME, TYPE, OP, INIT, __COUNTER__)

#define OPDI_REDUCTION private(opdi::internalReductionProbe)


