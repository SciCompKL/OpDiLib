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

#include "instrument/ompLogicInstrumentInterface.hpp"

#if OPDI_DEFAULT_ADJOINT_ACCESS_MODE == OPDI_ADJOINT_ACCESS_ATOMIC
  opdi::LogicInterface::AdjointAccessMode const opdi::ImplicitTaskOmpLogic::defaultAdjointAccessMode
      = opdi::LogicInterface::AdjointAccessMode::Atomic;
#elif OPDI_DEFAULT_ADJOINT_ACCESS_MODE == OPDI_ADJOINT_ACCESS_CLASSICAL
  opdi::LogicInterface::AdjointAccessMode const opdi::ImplicitTaskOmpLogic::defaultAdjointAccessMode
      = opdi::LogicInterface::AdjointAccessMode::Classical;
#else
  #error Unknown adjoint access mode.
#endif

std::list<opdi::OmpLogicInstrumentInterface*> opdi::ompLogicInstruments;

#include "implicitTaskOmpLogic.cpp"
#include "masterOmpLogic.cpp"
#include "mutexOmpLogic.cpp"
#include "parallelOmpLogic.cpp"
#include "syncRegionOmpLogic.cpp"
#include "workOmpLogic.cpp"
