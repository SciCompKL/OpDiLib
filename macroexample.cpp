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

#include <codi.hpp>
#include <iostream>

#include <opdi/backend/macro/macroBackend.hpp>
#include <opdi.hpp>

using Real = codi::RealReverseIndexOpenMP;  // use a suitable CoDiPack type
using Tape = typename Real::Tape;

int main(int nargs, char** args) {

  // initialize OpDiLib

  opdi::backend = new opdi::MacroBackend();
  opdi::backend->init();
  opdi::logic = new opdi::OmpLogic;
  opdi::logic->init();
  opdi::tool = new CoDiOpDiLibTool<Real>;
  opdi::tool->init();

  // usual AD workflow

  Real x = 4.0;

  Tape& tape = Real::getTape();
  tape.setActive();
  tape.registerInput(x);

  // parallel computation

  size_t constexpr N = 10000000;

  Real y = 0.0;

  OPDI_PARALLEL()
  {
    Real localSum = 0.0;

    OPDI_FOR()
    for (size_t i = 0; i < N; ++i)
    {
      localSum += sin(x * i);
    }
    OPDI_END_FOR

    OPDI_CRITICAL()
    {
      y += localSum;
    }
    OPDI_END_CRITICAL
  }
  OPDI_END_PARALLEL

  // usual AD workflow

  tape.registerOutput(y);
  tape.setPassive();
  y.setGradient(1.0);

  opdi::logic->prepareEvaluate();  // prepare OpDiLib for evaluation

  tape.evaluate();

  std::cout << "f(" << x << ") = " << y << std::endl;
  std::cout << "df/dx(" << x << ") = " << x.getGradient() << std::endl;

  // finalize OpDiLib

  opdi::tool->finalize();
  opdi::logic->finalize();
  opdi::backend->finalize();
  delete opdi::tool;
  delete opdi::logic;
  delete opdi::backend;

  return 0;
}

// don't forget to include the OpDiLib source file
#include "opdi.cpp"
