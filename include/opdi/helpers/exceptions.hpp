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

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#include "../config.hpp"

#include "macros.hpp"

namespace opdi {

  inline void printWarning(const char* func, const char* file, int line, const char* message, ...) {
    fprintf(stderr, "Error in %s (%s:%d): ", func, file, line);

    va_list vl;
    va_start(vl, message);
    vfprintf(stderr, message, vl);
    va_end(vl);

    fprintf(stderr, "\n");
  }

  inline void printError(const char* func, const char* file, int line, const char* message, ...) {
    fprintf(stderr, "Error in %s (%s:%d): ", func, file, line);

    va_list vl;
    va_start(vl, message);
    vfprintf(stderr, message, vl);
    va_end(vl);

    fprintf(stderr, "\n");
    exit(-1);
  }
}

// convenience macros

#if OPDI_ENABLE_WARNINGS
  #define OPDI_WARNING(...) opdi::printWarning(__func__, __FILE__, __LINE__, __VA_ARGS__)
  #define OPDI_CHECK_WARNING(success) \
    if (!success) OPDI_WARNING(OPDI_STR(success))
#else
  #define OPDI_WARNING(...)
  #define OPDI_CHECK_WARNING(success)
#endif

#if OPDI_ENABLE_ERRORS
  #if OPDI_TREAT_ERRORS_AS_WARNINGS
    #define OPDI_ERROR(...) OPDI_WARNING(__VA_ARGS__)
    #define OPDI_CHECK_ERROR(success) OPDI_CHECK_WARNING(success)
  #else
    #define OPDI_ERROR(...) opdi::printError(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define OPDI_CHECK_ERROR(success) \
      if (!success) OPDI_ERROR(OPDI_STR(success))
  #endif
#else
  #define OPDI_ERROR(...)
  #define OPDI_CHECK_ERROR(success)
#endif
