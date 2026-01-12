/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2026 Chair for Scientific Computing (SciComp), RPTU University Kaiserslautern-Landau
 * Homepage: https://scicomp.rptu.de
 * Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
 *
 * Lead developer: Johannes Blühdorn (SciComp, RPTU University Kaiserslautern-Landau)
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

      /* instrumentation of forward actions */

      virtual void onParallelBegin(ParallelData* data) {
        if (data == nullptr) {
          TapedOutput::print("F PARB l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "(skipped)");
        }
        else if (!data->isActiveParallelRegion) {
          TapedOutput::print("F PARB l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "parent", data->encounteringTaskTape,
                             "mode", data->encounteringTaskAdjointAccessMode,
                             "(passive)");
        }
        else {
          TapedOutput::print("F PARB l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "parent", data->encounteringTaskTape,
                             "mode", data->encounteringTaskAdjointAccessMode);
        }
      }

      virtual void onParallelEnd(ParallelData* data) {
        if (data == nullptr) {
          TapedOutput::print("F PARE l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "(skipped)");
        }
        else if (!data->isActiveParallelRegion) {
          TapedOutput::print("F PARE l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "parent", data->encounteringTaskTape,
                             "mode", data->encounteringTaskAdjointAccessMode,
                             "(passive)");
        }
        else {
          TapedOutput::print("F PARE l", omp_get_level(),
                             "t", omp_get_thread_num(),
                             "parent", data->encounteringTaskTape,
                             "mode", data->encounteringTaskAdjointAccessMode);
        }
      }

      virtual void onImplicitTaskBegin(ImplicitTaskData* data) {
        if (data->isInitialImplicitTask) {
          TapedOutput::print("F IMTB IIT");
        }
        else {
          assert(tool != nullptr);
          TapedOutput::print("F IMTB l", data->level,
                             "t", data->indexInTeam,
                             "tape", data->newTape,
                             "pos", tool->positionToString(data->positions.back()),
                             "mode", data->adjointAccessModes[0]);
        }
      }

      virtual void onImplicitTaskEnd(ImplicitTaskData* data) {
        if (data->isInitialImplicitTask) {
          TapedOutput::print("F IMTE IIT");
        }
        else {
          assert(tool != nullptr);
          TapedOutput::print("F IMTE l", data->level,
                             "t", data->indexInTeam,
                             "tape", data->newTape,
                             "pos", tool->positionToString(data->positions.back()),
                             "mode", data->adjointAccessModes.back());
        }
      }

      virtual void onMutexAcquired(MutexOmpLogic::Data* data) {
        TapedOutput::print("F MACQ t", omp_get_thread_num(),
                           "kind", data->mutexKind,
                           "id", data->waitId,
                           "at", data->counter);
      }

      virtual void onMutexReleased(MutexOmpLogic::Data* data) {
        TapedOutput::print("F MREL t", omp_get_thread_num(),
                           "kind", data->mutexKind,
                           "id", data->waitId,
                           "at", data->counter);
      }

      virtual void onMutexDestroyed(MutexOmpLogic::Data* data) {
        TapedOutput::print("F MDES t", omp_get_thread_num(),
                           "kind", data->mutexKind,
                           "id", data->waitId);
      }

      virtual void onSyncRegion(SyncRegionOmpLogic::Data* data) {
        TapedOutput::print("F SYNC t", omp_get_thread_num(), "kind", data->kind, "endp", data->endpoint);
      }

      virtual void onMasked(MaskedOmpLogic::Data* data) {
        TapedOutput::print("F MASK t", omp_get_thread_num(), "endp", data->endpoint);
      }

      virtual void onWork(WorkOmpLogic::Data* data) {
        TapedOutput::print("F WORK t", omp_get_thread_num(), "kind", data->kind, "endp", data->endpoint);
      }

      /* instrumentation of reverse actions */

      virtual void reverseParallelBegin(ParallelData* data) {
        TapedOutput::print("R PARB l", omp_get_level(),
                           "t", omp_get_thread_num(),
                           "parent", data->encounteringTaskTape);
      }

      virtual void reverseParallelEnd(ParallelData* data) {
        TapedOutput::print("R PARE l", omp_get_level(),
                           "t", omp_get_thread_num(),
                           "parent", data->encounteringTaskTape);
      }

      virtual void reverseImplicitTaskBegin(ImplicitTaskData* data) {
        assert(tool != nullptr);
        TapedOutput::print("R IMTB l", data->level,
                           "t", data->indexInTeam,
                           "tape", data->newTape,
                           "pos", tool->positionToString(data->positions.back()));
      }

      virtual void reverseImplicitTaskEnd(ImplicitTaskData* data) {
        assert(tool != nullptr);
        TapedOutput::print("R IMTE l", data->level,
                           "t", data->indexInTeam,
                           "tape", data->newTape,
                           "pos", tool->positionToString(data->positions.front()));
      }

      virtual void reverseImplicitTaskPart(ImplicitTaskData* data, std::size_t part) {
        assert(tool != nullptr);
        TapedOutput::print("R IMTP l", data->level,
                           "t", data->indexInTeam,
                           "tape", data->newTape,
                           "start", tool->positionToString(data->positions[part]),
                           "end", tool->positionToString(data->positions[part - 1]),
                           "mode", data->adjointAccessModes[part - 1]);
      }

      virtual void reverseMutexWait(MutexOmpLogic::Data* data) {
        TapedOutput::print("R MWAI l", omp_get_level(),
                           "t", omp_get_thread_num(),
                           "kind", data->mutexKind,
                           "id", data->waitId,
                           "until", data->counter);
      }

      virtual void reverseMutexDecrement(MutexOmpLogic::Data* data) {
        TapedOutput::print("R MDEC t", omp_get_thread_num(),
                           "kind", data->mutexKind,
                           "id", data->waitId,
                           "to", data->counter);
      }

      virtual void reverseSyncRegion(SyncRegionOmpLogic::Data* data) {
        TapedOutput::print("R SYNC l", omp_get_level(), "t", omp_get_thread_num(), "kind", data->kind, "endp", data->endpoint);
      }

      virtual void reverseMasked(MaskedOmpLogic::Data* data) {
        TapedOutput::print("R MASK t", omp_get_thread_num(), "endp", data->endpoint);
      }

      virtual void reverseWork(WorkOmpLogic::Data* data) {
        TapedOutput::print("R WORK t", omp_get_thread_num(), "kind", data->kind, "endp", data->endpoint);
      }

      virtual void reverseFlush() {
        TapedOutput::print("R FLSH l", omp_get_level(), "t", omp_get_thread_num());
      }

      /* instrumentation of other functionality */

      virtual void onSetAdjointAccessMode(LogicInterface::AdjointAccessMode adjointAccess) {
        TapedOutput::print("F SAAM t", omp_get_thread_num(), "mode", adjointAccess);
      }
  };
}
