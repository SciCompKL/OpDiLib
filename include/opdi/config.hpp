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

/* ------------------ constants ------------------ */

/* backends */

#define OPDI_MACRO_BACKEND 1
#define OPDI_OMPT_BACKEND 2

#define OPDI_ADJOINT_ACCESS_ATOMIC 1
#define OPDI_ADJOINT_ACCESS_CLASSICAL 2

/* ------------------ configuration ------------------ */

/* logic options */

#ifndef OPDI_OMP_LOGIC_INSTRUMENT
  #define OPDI_OMP_LOGIC_INSTRUMENT 0
#endif

#ifndef OPDI_VARIABLE_ADJOINT_ACCESS_MODE
  #define OPDI_VARIABLE_ADJOINT_ACCESS_MODE 1
#endif

#ifndef OPDI_DEFAULT_ADJOINT_ACCESS_MODE
  #define OPDI_DEFAULT_ADJOINT_ACCESS_MODE OPDI_ADJOINT_ACCESS_ATOMIC
#endif

/* error handling options */

#ifndef OPDI_ENABLE_WARNINGS
  #define OPDI_ENABLE_WARNINGS 1
#endif

#ifndef OPDI_ENABLE_ERRORS
  #define OPDI_ENABLE_ERRORS 1
#endif

#ifndef OPDI_TREAT_ERRORS_AS_WARNINGS
  #define OPDI_TREAT_ERRORS_AS_WARNINGS 0
#endif
