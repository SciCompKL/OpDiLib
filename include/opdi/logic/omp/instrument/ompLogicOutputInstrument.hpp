/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
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

#include "../../../misc/tapedOutput.hpp"

#include "ompLogicInstrumentInterface.hpp"

namespace opdi {

  struct OmpLogicOutputInstrument : public OmpLogicInstrumentInterface {
    public:

      virtual ~OmpLogicOutputInstrument() {}

      virtual void reverseFlush() {
        TapedOutput::print("R FLSH l", omp_get_level(), "i", omp_get_thread_num());
      }

      virtual void onImplicitTaskBegin(ImplicitTaskOmpLogic::Data* data) {
        TapedOutput::print("F IMTB l", omp_get_level(),
                           "i", data->index,
                           "tape", data->parallelData->tapes[data->index],
                           "pos", tool->positionToString(data->parallelData->positions[data->index].back()));
      }

      virtual void onImplicitTaskEnd(ImplicitTaskOmpLogic::Data* data) {
        TapedOutput::print("F IMTE l", omp_get_level(),
                           "i", data->index,
                           "tape", data->parallelData->tapes[data->index],
                           "pos", tool->positionToString(data->parallelData->positions[data->index].back()));
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

      virtual void reverseParallel(ParallelOmpLogic::Data* data) {
        TapedOutput::print("R PARA i", omp_get_thread_num(),
                           "tape", data->tapes[omp_get_thread_num()],
                           "start", tool->positionToString(data->positions[omp_get_thread_num()].back()),
                           "end", tool->positionToString(data->positions[omp_get_thread_num()].front()));
      }

      virtual void reverseParallelPart(ParallelOmpLogic::Data* data, std::size_t j) {
        TapedOutput::print("R PARP i", omp_get_thread_num(),
                           "tape", data->tapes[omp_get_thread_num()],
                           "j", j,
                           "start", tool->positionToString(data->positions[omp_get_thread_num()][j]),
                           "end", tool->positionToString(data->positions[omp_get_thread_num()][j - 1]),
                           "mode", data->adjointAccessModes[omp_get_thread_num()][j - 1]);
      }

      virtual void onParallelBegin(ParallelOmpLogic::Data* data) {
        TapedOutput::print("F PARB l", omp_get_level(),
                           "master", data->masterTape,
                           "mode", data->outerAdjointAccessMode);
      }

      virtual void onParallelEnd(ParallelOmpLogic::Data* data) {
        TapedOutput::print("F PARE l", omp_get_level(),
                           "master", data->masterTape,
                           "mode", data->outerAdjointAccessMode);
      }

      virtual void reverseSyncRegion(SyncRegionOmpLogic::Data* data) {
        TapedOutput::print("R SYNC l", omp_get_level(), "i", omp_get_thread_num(), "kind", data->kind, "endp", data->endpoint);
      }

      virtual void onSyncRegion(LogicInterface::SyncRegionKind kind, LogicInterface::ScopeEndpoint endpoint) {
        TapedOutput::print("F SYNC i", omp_get_thread_num(), "kind", kind, "endp", endpoint);
      }

      virtual void reverseWork(WorkOmpLogic::Data* data) {
        TapedOutput::print("R WORK i", omp_get_thread_num(), "kind", data->kind, "endp", data->endpoint);
      }

      virtual void onWork(LogicInterface::WorksharingKind kind, LogicInterface::ScopeEndpoint endpoint) {
        TapedOutput::print("F WORK i", omp_get_thread_num(), "kind", kind, "endp", endpoint);
      }
  };
}
