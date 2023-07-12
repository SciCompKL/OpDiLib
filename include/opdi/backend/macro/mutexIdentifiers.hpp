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

#include <map>
#include <omp.h>

#include "../backendInterface.hpp"

namespace opdi {

  struct MutexIdentifiers : public virtual BackendInterface {
    private:

      omp_lock_t criticalLock;

      std::map<std::string, std::size_t> criticalIdentifiers;

      std::size_t nextCriticalIdentifier;

    public:
      MutexIdentifiers() : nextCriticalIdentifier(1) {

        omp_init_lock(&this->criticalLock);
      }

      ~MutexIdentifiers() {
        omp_destroy_lock(&this->criticalLock);
      }

      std::size_t getCriticalIdentifier(std::string const& name) {
        // unnamed critical region has index 0
        if (name.empty()) {
          return 0;
        }

        omp_set_lock(&this->criticalLock);
        if (this->criticalIdentifiers.count(name) == 0) {
          this->criticalIdentifiers[name] = this->nextCriticalIdentifier++;
        }
        unsigned int result = this->criticalIdentifiers[name];
        omp_unset_lock(&this->criticalLock);

        return result;
      }

      std::size_t getLockIdentifier(omp_lock_t* lock) {
        return reinterpret_cast<std::size_t>(lock);
      }

      std::size_t getNestedLockIdentifier(omp_nest_lock_t* lock) {
        return reinterpret_cast<std::size_t>(lock);
      }
  };

}
