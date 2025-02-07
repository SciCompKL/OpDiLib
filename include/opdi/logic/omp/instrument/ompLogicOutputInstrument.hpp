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

#include "../../../misc/tapedOutput.hpp"

#include "ompLogicInstrumentInterface.hpp"

namespace opdi {

  struct OmpLogicOutputInstrument : public OmpLogicInstrumentInterface {
    public:

      virtual ~OmpLogicOutputInstrument() {}

      virtual void reverseFlush() {
        TapedOutput::print("R FLSH l", omp_get_level(), "t", omp_get_thread_num());
      }

      virtual void reverseImplicitTaskBegin(ImplicitTaskOmpLogic::Data* data) {
        TapedOutput::print("R IMTB l", data->level,
                           "t", data->index,
                           "tape", data->tape,
                           "pos", tool->positionToString(data->positions.back()));
      }

      virtual void reverseImplicitTaskEnd(ImplicitTaskOmpLogic::Data* data) {
        TapedOutput::print("R IMTE l", data->level,
                           "t", data->index,
                           "tape", data->tape,
                           "pos", tool->positionToString(data->positions.front()));
      }

      virtual void reverseImplicitTaskPart(ImplicitTaskOmpLogic::Data* data, std::size_t part) {
        TapedOutput::print("R IMTP l", data->level,
                           "t", data->index,
                           "tape", data->tape,
                           "start", tool->positionToString(data->positions[part]),
                           "end", tool->positionToString(data->positions[part - 1]),
                           "mode", data->adjointAccessModes[part - 1]);
      }

      virtual void onImplicitTaskBegin(ImplicitTaskOmpLogic::Data* data) {
        if (data->initialImplicitTask) {
          TapedOutput::print("F IMTB IIT");
        }
        else {
          TapedOutput::print("F IMTB l", data->level,
                             "t", data->index,
                             "tape", data->tape,
                             "pos", tool->positionToString(data->positions.back()),
                             "mode", data->adjointAccessModes[0]);
        }
      }

      virtual void onImplicitTaskEnd(ImplicitTaskOmpLogic::Data* data) {
        if (data->initialImplicitTask) {
          TapedOutput::print("F IMTE IIT");
        }
        else {
          TapedOutput::print("F IMTE l", data->level,
                             "t", data->index,
                             "tape", data->tape,
                             "pos", tool->positionToString(data->positions.back()),
                             "mode", data->adjointAccessModes.back());
        }
      }

      virtual void reverseMutexWait(MutexOmpLogic::Data* data) {
        TapedOutput::print("R MWAI l", omp_get_level(),
                           "t", omp_get_thread_num(),
                           "kind", data->kind,
                           "id", data->waitId,
                           "until", data->traceValue);
      }

      virtual void reverseMutexDecrement(MutexOmpLogic::Data* data) {
        TapedOutput::print("R MDEC t", omp_get_thread_num(),
                           "kind", data->kind,
                           "id", data->waitId,
                           "to", data->traceValue);
      }

      virtual void onMutexDestroyed(LogicInterface::MutexKind kind, std::size_t waitId) {
        TapedOutput::print("F MDES t", omp_get_thread_num(),
                           "kind", kind,
                           "id", waitId);
      }

      virtual void onMutexAcquired(MutexOmpLogic::Data* data) {
        TapedOutput::print("F MACQ t", omp_get_thread_num(),
                           "kind", data->kind,
                           "id", data->waitId,
                           "at", data->traceValue);
      }

      virtual void onMutexReleased(MutexOmpLogic::Data* data) {
        TapedOutput::print("F MREL t", omp_get_thread_num(),
                           "kind", data->kind,
                           "id", data->waitId,
                           "at", data->traceValue);
      }

      virtual void reverseParallelBegin(ParallelOmpLogic::Data* data) {
        TapedOutput::print("R PARB l", omp_get_level(),
                           "t", omp_get_thread_num(),
                           "parent", data->parentTape);
      }

      virtual void reverseParallelEnd(ParallelOmpLogic::Data* data) {
        TapedOutput::print("R PARE l", omp_get_level(),
                           "t", omp_get_thread_num(),
                           "parent", data->parentTape);
      }

      virtual void onParallelBegin(ParallelOmpLogic::Data* data) {
        if (data == nullptr) {
          TapedOutput::print("F PARB l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "(skipped)");
        }
        else if (!data->activeParallelRegion) {
          TapedOutput::print("F PARB l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "parent", data->parentTape,
                             "mode", data->parentAdjointAccessMode,
                             "(passive)");
        }
        else {
          TapedOutput::print("F PARB l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "parent", data->parentTape,
                             "mode", data->parentAdjointAccessMode);
        }
      }

      virtual void onParallelEnd(ParallelOmpLogic::Data* data) {
        if (data == nullptr) {
          TapedOutput::print("F PARE l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "(skipped)");
        }
        else if (!data->activeParallelRegion) {
          TapedOutput::print("F PARE l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "parent", data->parentTape,
                             "mode", data->parentAdjointAccessMode,
                             "(passive)");
        }
        else {
          TapedOutput::print("F PARE l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "parent", data->parentTape,
                             "mode", data->parentAdjointAccessMode);
        }
      }

      virtual void reverseSyncRegion(SyncRegionOmpLogic::Data* data) {
        TapedOutput::print("R SYNC l", omp_get_level(), "t", omp_get_thread_num(), "kind", data->kind, "endp", data->endpoint);
      }

      virtual void onSyncRegion(LogicInterface::SyncRegionKind kind, LogicInterface::ScopeEndpoint endpoint) {
        TapedOutput::print("F SYNC t", omp_get_thread_num(), "kind", kind, "endp", endpoint);
      }

      virtual void reverseWork(WorkOmpLogic::Data* data) {
        TapedOutput::print("R WORK t", omp_get_thread_num(), "kind", data->kind, "endp", data->endpoint);
      }

      virtual void onWork(LogicInterface::WorksharingKind kind, LogicInterface::ScopeEndpoint endpoint) {
        TapedOutput::print("F WORK t", omp_get_thread_num(), "kind", kind, "endp", endpoint);
      }

      virtual void reverseMaster(MasterOmpLogic::Data* data) {
        TapedOutput::print("R MAST t", omp_get_thread_num(), "endp", data->endpoint);
      }

      virtual void onMaster(LogicInterface::ScopeEndpoint endpoint) {
        TapedOutput::print("F MAST t", omp_get_thread_num(), "endp", endpoint);
      }

      virtual void onSetAdjointAccessMode(LogicInterface::AdjointAccessMode adjointAccess) {
        TapedOutput::print("F SAAM t", omp_get_thread_num(), "mode", adjointAccess);
      }
  };
}
