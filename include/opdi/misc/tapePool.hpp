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
#include <map>
#include <set>

#include "../tool/toolInterface.hpp"

namespace opdi {

  struct TapePool {
    private:
      std::map<void*, std::map<int, void*>> tapes;
      std::set<void*> createdTapes;

      omp_lock_t lock;

    public:

      TapePool() {}

      virtual ~TapePool() {}

      void init() {
        omp_init_lock(&this->lock);
      }

      void finalize() {
        omp_destroy_lock(&this->lock);
      }

      omp_lock_t* getInternalLock() {
        return &this->lock;
      }

      void* getTape(void* master, int index) {
        omp_set_lock(&this->lock);

        if (this->tapes[master].find(index) == this->tapes[master].end()) {
          void* newTape = tool->createTape();
          this->tapes[master][index] = newTape;
          this->createdTapes.insert(newTape);
        }

        void* result = this->tapes[master][index];
        omp_unset_lock(&this->lock);
        return result;
      }

      void clear() {
        omp_set_lock(&this->lock);
        for (auto& tape : this->createdTapes) {
          tool->deleteTape(tape);
        }
        this->createdTapes.clear();
        this->tapes.clear();
        omp_unset_lock(&this->lock);
      }
  };
}
