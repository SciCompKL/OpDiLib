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

/* ------------------ constants ------------------ */

/* backends */

#define OPDI_MACRO_BACKEND 1
#define OPDI_OMPT_BACKEND 2

/* logic output options */

#define OPDI_PARALLEL_OUT 1
#define OPDI_IMPLICIT_TASK_OUT 2
#define OPDI_MUTEX_OUT 4
#define OPDI_SYNC_REGION_OUT 8
#define OPDI_WORK_OUT 16

/* ------------------ configuration ------------------ */

/* logic options */

// control output by forming conjunctions (sums) of options, 0 means no output
#define OPDI_LOGIC_OUT 0

/* error handling options */

#define OPDI_ENABLE_WARNINGS 1
#define OPDI_ENABLE_ERRORS 1
#define OPDI_TREAT_ERRORS_AS_WARNINGS 0
