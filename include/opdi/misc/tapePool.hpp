/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2021 Chair for Scientific Computing (SciComp), TU Kaiserslautern
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

#include <list>
#include <omp.h>
#include <set>

#include "../tool/toolInterface.hpp"

namespace opdi {

  struct TapePool {
    private:
      // tapes that are not used and can be acquired
      std::list<void*> unusedTapes;

      // tapes that are no longer used but do not qualify for reuse yet
      std::list<void*> blockedTapes;

      std::set<void*> allTapes;

    protected:
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

      void* acquireTape() {
        omp_set_lock(&this->lock);

        if (this->unusedTapes.empty()) {
          this->unusedTapes.push_back(tool->createTape());
          this->allTapes.insert(this->unusedTapes.back());
        }

        void* result = unusedTapes.front();
        unusedTapes.pop_front();
        omp_unset_lock(&this->lock);
        return result;
      }

      void releaseTape(void* tape, bool blocked = true) {
        omp_set_lock(&this->lock);
        if (blocked) {
          blockedTapes.push_back(tape);
        }
        else {
          unusedTapes.push_back(tape);
        }
        omp_unset_lock(&this->lock);
      }

      void unblock() {
        omp_set_lock(&this->lock);
        unusedTapes.insert(unusedTapes.end(), blockedTapes.begin(), blockedTapes.end());
        blockedTapes.clear();
        omp_unset_lock(&this->lock);
      }

      void clear() {
        omp_set_lock(&this->lock);
        for (auto& tape : this->allTapes) {
          tool->deleteTape(tape);
        }
        omp_unset_lock(&this->lock);
      }
  };
}
