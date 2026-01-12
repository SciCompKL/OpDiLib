/*
 * OpDiLib, an Open Multiprocessing Differentiation Library
 *
 * Copyright (C) 2020-2022 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Copyright (C) 2023-2026 Chair for Scientific Computing (SciComp), RPTU University Kaiserslautern-Landau
 * Homepage: https://scicomp.rptu.de
 * Contact:  Prof. Nicolas R. Gauger (opdi@scicomp.uni-kl.de)
 *
 * Lead developer: Johannes Blühdorn (SciComp, RPTU University Kaiserslautern-Landau)
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

/* ------------------ OpDiLib version ------------------ */

#define OPDI_MAJOR 2
#define OPDI_MINOR 0
#define OPDI_PATCH 0

/* ------------------ constants ------------------ */

#define OPDI_MACRO_BACKEND 1
#define OPDI_OMPT_BACKEND 2

#define OPDI_ADJOINT_ACCESS_ATOMIC 1
#define OPDI_ADJOINT_ACCESS_CLASSICAL 2

#define OPDI_SCOPE_ENDPOINT_BEGIN 1
#define OPDI_SCOPE_ENDPOINT_END 2
#define OPDI_SCOPE_ENDPOINT_BOTH 3

#define OPDI_OMPT_IMPLICIT_TASK_END 1
#define OPDI_OMPT_SYNC_REGION_END 2

#define OPDI_PAIR_OF_AD_EVENTS_PER_ENDPOINT 1
#define OPDI_SINGLE_AD_EVENT_PER_ENDPOINT 2

/* ------------------ configuration ------------------ */

/* ----- backend configuration ----- */

#ifndef OPDI_BACKEND_GENERATE_MASKED_EVENTS
  #define OPDI_BACKEND_GENERATE_MASKED_EVENTS 0
#endif

#ifndef OPDI_BACKEND_GENERATE_WORK_EVENTS
  #define OPDI_BACKEND_GENERATE_WORK_EVENTS 0
#endif

#ifndef OPDI_OMPT_BACKEND_IMPLICIT_TASK_END_SOURCE
  #define OPDI_OMPT_BACKEND_IMPLICIT_TASK_END_SOURCE OPDI_OMPT_IMPLICIT_TASK_END
#endif

static_assert(0 < OPDI_OMPT_BACKEND_IMPLICIT_TASK_END_SOURCE);
static_assert(OPDI_OMPT_BACKEND_IMPLICIT_TASK_END_SOURCE <= 2);

#ifndef OPDI_OMPT_BACKEND_BARRIER_IMPLEMENTATION_BEHAVIOUR
  #define OPDI_OMPT_BACKEND_BARRIER_IMPLEMENTATION_BEHAVIOUR OPDI_PAIR_OF_AD_EVENTS_PER_ENDPOINT
#endif

static_assert(0 < OPDI_OMPT_BACKEND_BARRIER_IMPLEMENTATION_BEHAVIOUR);
static_assert(OPDI_OMPT_BACKEND_BARRIER_IMPLEMENTATION_BEHAVIOUR <= 2);

/* ----- logic configuration ----- */

/* general logic options */

#ifndef OPDI_OMP_LOGIC_INSTRUMENT
  #define OPDI_OMP_LOGIC_INSTRUMENT 0
#endif

#ifndef OPDI_VARIABLE_ADJOINT_ACCESS_MODE
  #define OPDI_VARIABLE_ADJOINT_ACCESS_MODE 1
#endif

#ifndef OPDI_DEFAULT_ADJOINT_ACCESS_MODE
  #define OPDI_DEFAULT_ADJOINT_ACCESS_MODE OPDI_ADJOINT_ACCESS_ATOMIC
#endif

static_assert(0 < OPDI_DEFAULT_ADJOINT_ACCESS_MODE);
static_assert(OPDI_DEFAULT_ADJOINT_ACCESS_MODE <= 2);

#ifndef OPDI_OMP_LOGIC_CLEAR_ADJOINTS
  #define OPDI_OMP_LOGIC_CLEAR_ADJOINTS 0
#endif

/* sync region behaviour */

#ifndef OPDI_SYNC_REGION_BARRIER_BEHAVIOUR
  #define OPDI_SYNC_REGION_BARRIER_BEHAVIOUR OPDI_SCOPE_ENDPOINT_BEGIN
#endif

static_assert(0 < OPDI_SYNC_REGION_BARRIER_BEHAVIOUR);
static_assert(OPDI_SYNC_REGION_BARRIER_BEHAVIOUR <= 3);

#ifndef OPDI_SYNC_REGION_BARRIER_IMPLICIT_BEHAVIOUR
  #define OPDI_SYNC_REGION_BARRIER_IMPLICIT_BEHAVIOUR OPDI_SCOPE_ENDPOINT_BEGIN
#endif

static_assert(0 < OPDI_SYNC_REGION_BARRIER_IMPLICIT_BEHAVIOUR);
static_assert(OPDI_SYNC_REGION_BARRIER_IMPLICIT_BEHAVIOUR <= 3);

#ifndef OPDI_SYNC_REGION_BARRIER_EXPLICIT_BEHAVIOUR
  #define OPDI_SYNC_REGION_BARRIER_EXPLICIT_BEHAVIOUR OPDI_SCOPE_ENDPOINT_BEGIN
#endif

static_assert(0 < OPDI_SYNC_REGION_BARRIER_EXPLICIT_BEHAVIOUR);
static_assert(OPDI_SYNC_REGION_BARRIER_EXPLICIT_BEHAVIOUR <= 3);

#ifndef OPDI_SYNC_REGION_BARRIER_IMPLEMENTATION_BEHAVIOUR
  #define OPDI_SYNC_REGION_BARRIER_IMPLEMENTATION_BEHAVIOUR OPDI_SCOPE_ENDPOINT_BEGIN
#endif

static_assert(0 < OPDI_SYNC_REGION_BARRIER_IMPLEMENTATION_BEHAVIOUR);
static_assert(OPDI_SYNC_REGION_BARRIER_IMPLEMENTATION_BEHAVIOUR <= 3);

#ifndef OPDI_SYNC_REGION_BARRIER_REVERSE_BEHAVIOUR
  #define OPDI_SYNC_REGION_BARRIER_REVERSE_BEHAVIOUR OPDI_SCOPE_ENDPOINT_BEGIN
#endif

static_assert(0 < OPDI_SYNC_REGION_BARRIER_REVERSE_BEHAVIOUR);
static_assert(OPDI_SYNC_REGION_BARRIER_REVERSE_BEHAVIOUR <= 3);

/* ----- error handling ----- */

#ifndef OPDI_ENABLE_WARNINGS
  #define OPDI_ENABLE_WARNINGS 1
#endif

#ifndef OPDI_ENABLE_ERRORS
  #define OPDI_ENABLE_ERRORS 1
#endif

#ifndef OPDI_TREAT_ERRORS_AS_WARNINGS
  #define OPDI_TREAT_ERRORS_AS_WARNINGS 0
#endif
