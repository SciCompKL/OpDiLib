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

#ifdef __SANITIZE_THREAD__
  #define ANNOTATE_RWLOCK_CREATE(lock) AnnotateRWLockCreate(__FILE__, __LINE__, (void*)lock)
  #define ANNOTATE_RWLOCK_DESTROY(lock) AnnotateRWLockDestroy(__FILE__, __LINE__, (void*)lock)
  #define ANNOTATE_RWLOCK_ACQUIRED(lock, isWrite) AnnotateRWLockAcquired(__FILE__, __LINE__, (void*)lock, isWrite)
  #define ANNOTATE_RWLOCK_RELEASED(lock, isWrite) AnnotateRWLockReleased(__FILE__, __LINE__, (void*)lock, isWrite)

  extern "C" void AnnotateRWLockCreate(const char* f, int l, void* addr);
  extern "C" void AnnotateRWLockDestroy(const char* f, int l, void* addr);
  extern "C" void AnnotateRWLockAcquired(const char* f, int l, void* addr, size_t isWrite);
  extern "C" void AnnotateRWLockReleased(const char* f, int l, void* addr, size_t isWrite);
#endif
