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

#define OPDI_PARALLEL(...)
#define OPDI_END_PARALLEL

#define OPDI_FOR(...)
#define OPDI_END_FOR

#define OPDI_SECTIONS(...)
#define OPDI_END_SECTIONS

#define OPDI_SINGLE(...)
#define OPDI_SINGLE_NOWAIT(...)
#define OPDI_END_SINGLE

#define OPDI_NOWAIT

#define OPDI_CRITICAL(...)
#define OPDI_CRITICAL_NAME(name, ...)
#define OPDI_END_CRITICAL

#define OPDI_ORDERED(...)
#define OPDI_END_ORDERED

#define OPDI_SECTION(...)
#define OPDI_END_SECTION

#define OPDI_MASTER(...)
#define OPDI_END_MASTER

#define OPDI_BARRIER(...)

#define OPDI_DECLARE_REDUCTION(...)

#define OPDI_REDUCTION
