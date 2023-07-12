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

#include "../../helpers/macros.hpp"

#define OPDI_PARALLEL(...) \
  OPDI_PRAGMA(omp parallel __VA_ARGS__)

#define OPDI_END_PARALLEL

#define OPDI_FOR(...) \
  OPDI_PRAGMA(omp for __VA_ARGS__)

#define OPDI_END_FOR

#define OPDI_SECTIONS(...) \
  OPDI_PRAGMA(omp sections __VA_ARGS__)

#define OPDI_END_SECTIONS

#define OPDI_SINGLE(...) \
  OPDI_PRAGMA(omp single __VA_ARGS__)

#define OPDI_SINGLE_NOWAIT(...) \
  OPDI_PRAGMA(omp single nowait __VA_ARGS__)

#define OPDI_END_SINGLE

#define OPDI_NOWAIT nowait

#define OPDI_CRITICAL(...) \
  OPDI_PRAGMA(omp critical __VA_ARGS__)

#define OPDI_CRITICAL_NAME(name, ...) \
  OPDI_PRAGMA(omp critical (name) __VA_ARGS__)

#define OPDI_END_CRITICAL

#define OPDI_ORDERED(...) \
  OPDI_PRAGMA(omp ordered __VA_ARGS__) \

#define OPDI_END_ORDERED

#define OPDI_SECTION(...) \
  OPDI_PRAGMA(omp section __VA_ARGS__)

#define OPDI_END_SECTION

#define OPDI_MASTER(...) \
  OPDI_PRAGMA(omp master __VA_ARGS__)

#define OPDI_END_MASTER

#define OPDI_BARRIER(...) \
  OPDI_PRAGMA(omp barrier __VA_ARGS__)

#define OPDI_DECLARE_REDUCTION(OP_NAME, TYPE, OP, INIT) \
  OPDI_PRAGMA(omp declare reduction(OP_NAME : TYPE : omp_out = omp_out OP omp_in) initializer(omp_priv = INIT))

#define OPDI_REDUCTION
