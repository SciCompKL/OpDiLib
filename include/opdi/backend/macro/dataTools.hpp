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

#include <stack>

namespace opdi {

  struct DataTools {
    private:
      static std::stack<void*> parallelData;
      #pragma omp threadprivate(parallelData)

      static std::stack<void*> implicitTaskData;
      #pragma omp threadprivate(implicitTaskData)

    public:
      static void pushParallelData(void* parallelData) {
        DataTools::parallelData.push(parallelData);
      }

      static void popParallelData() {
        DataTools::parallelData.pop();
      }

      static void* getParallelData() {
        if (DataTools::parallelData.empty()) {
          return nullptr;
        }

        return DataTools::parallelData.top();
      }

      static void pushTaskData(void* implicitTaskData) {
        DataTools::implicitTaskData.push(implicitTaskData);
      }

      static void popTaskData() {
        DataTools::implicitTaskData.pop();
      }

      static void* getImplicitTaskData() {
        if (DataTools::implicitTaskData.empty()) {
          return nullptr;
        }

        return DataTools::implicitTaskData.top();
      }
  };
}
