/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2025 Chair for Scientific Computing (SciComp), University of Kaiserslautern-Landau
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

#include <omp.h>

#include "../../config.hpp"
#include "../../helpers/macros.hpp"

#include "implicitBarrierTools.hpp"
#include "mutexIdentifiers.hpp"
#include "probes.hpp"
#include "reductionTools.hpp"

// macros that come in pairs

#define OPDI_PARALLEL(...) \
  { \
    void* opdiInternalParallelData = opdi::logic->onParallelBegin(opdi::DataTools::getTaskData(), omp_get_max_threads()); \
    opdi::TaskProbe opdiInternalTaskProbe(opdiInternalParallelData); \
    OPDI_PRAGMA(omp parallel __VA_ARGS__ firstprivate(opdiInternalTaskProbe))

#define OPDI_END_PARALLEL \
    opdi::logic->onParallelEnd(opdiInternalParallelData); \
  }

#define OPDI_FOR(...) \
  opdi::ImplicitBarrierTools::beginRegionWithImplicitBarrier(); \
  opdi::ReductionTools::beginRegionThatSupportsReductions(true); \
  OPDI_PRAGMA(omp for __VA_ARGS__ private(opdi::internalLoopProbe))

#define OPDI_END_FOR \
  opdi::ReductionTools::endRegionThatSupportsReductions(); \
  opdi::ImplicitBarrierTools::endRegionWithImplicitBarrier();

#define OPDI_SECTIONS(...) \
  opdi::ImplicitBarrierTools::beginRegionWithImplicitBarrier(); \
  opdi::ReductionTools::beginRegionThatSupportsReductions(true); \
  OPDI_PRAGMA(omp sections private(opdi::internalSectionsProbe) __VA_ARGS__)

#define OPDI_END_SECTIONS \
  opdi::ReductionTools::endRegionThatSupportsReductions(); \
  opdi::ImplicitBarrierTools::endRegionWithImplicitBarrier();

#define OPDI_SINGLE(...) \
  { \
    bool constexpr opdiInternalBarrierIndicator = true; \
    bool constexpr opdiInternalBroadcastIndicator = false; \
    void* opdiInternalTapePosition1;  /* for consistency with the end macro */ \
    void* opdiInternalTapePosition2; \
    OPDI_UNUSED(opdiInternalTapePosition1); \
    OPDI_UNUSED(opdiInternalTapePosition2); \
    opdi::ImplicitBarrierTools::beginRegionWithImplicitBarrier(); \
    { \
      opdi::SingleProbe localSingleProbe;  /* worksharing events */ \
      OPDI_PRAGMA(omp single __VA_ARGS__) \
      {

#define OPDI_SINGLE_NOWAIT(...) \
  { \
    bool constexpr opdiInternalBarrierIndicator = false; \
    bool constexpr opdiInternalBroadcastIndicator = false; \
    void* opdiInternalTapePosition1;  /* for consistency with the end macro */ \
    void* opdiInternalTapePosition2; \
    OPDI_UNUSED(opdiInternalTapePosition1); \
    OPDI_UNUSED(opdiInternalTapePosition2); \
    opdi::ImplicitBarrierTools::beginRegionWithImplicitBarrier(); \
    { \
      opdi::SingleProbe localSingleProbe;  /* worksharing events */ \
      OPDI_PRAGMA(omp single nowait __VA_ARGS__) \
      {

#define OPDI_SINGLE_COPYPRIVATE(...) \
  { \
    bool constexpr opdiInternalBarrierIndicator = true; \
    bool constexpr opdiInternalBroadcastIndicator = true; \
    void* opdiInternalTapePosition1 = opdi::tool->allocPosition(); \
    opdi::tool->getTapePosition(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition1); \
    /* broadcast-related barrier */ \
    opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierImplementation, \
                              opdi::LogicInterface::ScopeEndpoint::Begin); \
    opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierImplementation, \
                              opdi::LogicInterface::ScopeEndpoint::End); \
    void* opdiInternalTapePosition2 = opdi::tool->allocPosition(); \
    opdi::tool->getTapePosition(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition2); \
    opdi::ImplicitBarrierTools::beginRegionWithImplicitBarrier(); \
    { \
      opdi::SingleProbe localSingleProbe;  /* worksharing events */ \
      OPDI_PRAGMA(omp single __VA_ARGS__) \
      { \
        /* delay broadcast-related barrier for executor */ \
        opdi::tool->erase(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition1, opdiInternalTapePosition2);

#define OPDI_SINGLE_COPYPRIVATE_NOWAIT(...) \
  { \
    bool constexpr opdiInternalBarrierIndicator = false; \
    bool constexpr opdiInternalBroadcastIndicator = true; \
    void* opdiInternalTapePosition1 = opdi::tool->allocPosition(); \
    opdi::tool->getTapePosition(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition1); \
    /* broadcast-related barrier */ \
    opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierImplementation, \
                              opdi::LogicInterface::ScopeEndpoint::Begin); \
    opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierImplementation, \
                              opdi::LogicInterface::ScopeEndpoint::End); \
    void* opdiInternalTapePosition2 = opdi::tool->allocPosition(); \
    opdi::tool->getTapePosition(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition2); \
    opdi::ImplicitBarrierTools::beginRegionWithImplicitBarrier(); \
    { \
      opdi::SingleProbe localSingleProbe; \
      OPDI_PRAGMA(omp single nowait __VA_ARGS__) \
      { \
        /* delay broadcast-related barrier for executor */ \
        opdi::tool->erase(opdi::tool->getThreadLocalTape(), opdiInternalTapePosition1, opdiInternalTapePosition2);

#define OPDI_END_SINGLE \
        /* broadcast-related barrier */ \
        if (opdiInternalBroadcastIndicator) { \
          opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierImplementation, \
                                    opdi::LogicInterface::ScopeEndpoint::Begin); \
          opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierImplementation, \
                                    opdi::LogicInterface::ScopeEndpoint::End); \
        } \
      } \
    } \
    if (opdiInternalBroadcastIndicator) { \
      opdi::tool->freePosition(opdiInternalTapePosition1); \
      opdi::tool->freePosition(opdiInternalTapePosition2); \
    } \
    /* implicit barrier */ \
    opdi::ImplicitBarrierTools::implicitBarrierStack.top() = opdiInternalBarrierIndicator; \
    opdi::ImplicitBarrierTools::endRegionWithImplicitBarrier(); \
  }

#define OPDI_NOWAIT nowait private(opdi::internalNowaitProbe)

#define OPDI_CRITICAL(...) \
  OPDI_PRAGMA(omp critical __VA_ARGS__) \
  { \
    std::size_t constexpr opdiInternalCriticalIdentifier = 0; \
    opdi::logic->onMutexAcquired(opdi::LogicInterface::MutexKind::Critical, opdiInternalCriticalIdentifier);

#define OPDI_CRITICAL_NAME(name) \
  OPDI_PRAGMA(omp critical (name)) \
  { \
    std::size_t const opdiInternalCriticalIdentifier = opdi::backend->getCriticalIdentifier(std::string(#name)); \
    opdi::logic->onMutexAcquired(opdi::LogicInterface::MutexKind::Critical, opdiInternalCriticalIdentifier);

#define OPDI_CRITICAL_NAME_ARGS(name, ...) \
  OPDI_PRAGMA(omp critical (name) __VA_ARGS__) \
  { \
    std::size_t const opdiInternalCriticalIdentifier = opdi::backend->getCriticalIdentifier(std::string(#name)); \
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

#if OPDI_BACKEND_GENERATE_MASKED_EVENTS
  #define OPDI_MASTER(...) \
    OPDI_PRAGMA(omp master __VA_ARGS__) \
    { \
      opdi::logic->onMasked(opdi::LogicInterface::ScopeEndpoint::Begin);

  #define OPDI_END_MASTER \
      opdi::logic->onMasked(opdi::LogicInterface::ScopeEndpoint::End); \
    }

  #define OPDI_MASKED(...) \
    OPDI_PRAGMA(omp masked __VA_ARGS__) \
    { \
      opdi::logic->onMasked(opdi::LogicInterface::ScopeEndpoint::Begin);

  #define OPDI_END_MASKED \
      opdi::logic->onMasked(opdi::LogicInterface::ScopeEndpoint::End); \
    }
#else
  #define OPDI_MASTER(...) \
    OPDI_PRAGMA(omp master __VA_ARGS__) \

  #define OPDI_END_MASTER

  #define OPDI_MASKED(...) \
    OPDI_PRAGMA(omp masked __VA_ARGS__) \

  #define OPDI_END_MASKED
#endif

// standalone macros

#define OPDI_BARRIER(...) \
  opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierExplicit, \
                            opdi::LogicInterface::ScopeEndpoint::Begin); \
  OPDI_PRAGMA(omp barrier __VA_ARGS__) \
  opdi::logic->onSyncRegion(opdi::LogicInterface::SyncRegionKind::BarrierExplicit, \
                            opdi::LogicInterface::ScopeEndpoint::End);

// reduction macros

#if _OPENMP >= 202411
  #define OPDI_DECLARE_REDUCTION(OP_NAME, TYPE, OP, INIT) \
    TYPE operator OP (opdi::Reducer<TYPE> const& lhs, \
                      opdi::Reducer<TYPE> const& rhs) { \
          return lhs.value OP rhs.value; \
    } \
    \
    OPDI_PRAGMA(omp declare_reduction(OP_NAME : TYPE) \
        combiner(opdi::Reducer(omp_out) = opdi::Reducer(omp_out) OP opdi::Reducer(omp_in)) \
        initializer(omp_priv = INIT))
#else
  #define OPDI_DECLARE_REDUCTION(OP_NAME, TYPE, OP, INIT) \
    TYPE operator OP (opdi::Reducer<TYPE> const& lhs, \
                      opdi::Reducer<TYPE> const& rhs) { \
      return lhs.value OP rhs.value; \
    } \
    \
    OPDI_PRAGMA(omp declare reduction(OP_NAME : TYPE : \
        opdi::Reducer(omp_out) = opdi::Reducer(omp_out) OP opdi::Reducer(omp_in)) \
        initializer(omp_priv = INIT))
#endif

#define OPDI_REDUCTION private(opdi::internalReductionProbe)


