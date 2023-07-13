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

#include <array>
#include <cstdlib>
#include <iostream>

#include "codi.hpp"

#ifndef BUILD_REFERENCE
  #ifdef OPDI_USE_MACRO_BACKEND
    #include "opdi/backend/macro/macroBackend.hpp"
  #elif OPDI_USE_OMPT_BACKEND
    #include "opdi/backend/ompt/omptBackend.hpp"
  #endif
  #include "opdi.hpp"
  #ifdef OUTPUT_INSTRUMENT
    #include "opdi/logic/omp/instrument/ompLogicOutputInstrument.hpp"
  #endif
  #include "opdi/helpers/undefineMacros.hpp"
#endif

#include "opdi/helpers/emptyMacros.hpp"

#include "driverBase.hpp"

#ifdef BUILD_REFERENCE
  using TestReal = codi::RealReverseIndex;
#else
  using TestReal = codi::RealReverseIndexOpenMP;

  OPDI_DECLARE_REDUCTION(plus, TestReal, +, 0.0);
  OPDI_DECLARE_REDUCTION(prod, TestReal, *, 1.0);
#endif

template<typename _Case>
struct DriverFirstOrderReverseNoOpenMP : public DriverBase<DriverFirstOrderReverseNoOpenMP<_Case> > {
  public:

    using Case = _Case;

    static void run() {

      #ifndef BUILD_REFERENCE
        #ifdef OPDI_USE_MACRO_BACKEND
          opdi::backend = new opdi::MacroBackend();
          opdi::backend->init();
        #else
          if (omp_get_num_threads() /* trigger OMPT initialization */ && opdi::backend == nullptr) {
            std::cout << "Could not initialize OMPT backend. Please check OMPT support." << std::endl;
            exit(1);
          }
        #endif
        opdi::logic = new opdi::OmpLogic;
        opdi::logic->init();
        opdi::tool = new CoDiOpDiLibTool<TestReal>;
        opdi::tool->init();
        #ifdef OUTPUT_INSTRUMENT
          opdi::ompLogicInstruments.push_back(new opdi::OmpLogicOutputInstrument);
        #endif
      #endif

      std::array<std::array<TestReal, Case::nIn>, Case::nPoints> inputs = Case::template genPoints<TestReal>();

      for (int p = 0; p < Case::nPoints; ++p) {
        double jacobian[Case::nOut][Case::nIn];
        double primal[Case::nOut];

        for (int o = 0; o < Case::nOut; ++o) {
          TestReal::Tape& tape = TestReal::getTape();

          std::array<TestReal, Case::nOut> outputs = {0.0};

          tape.setActive();

          for (int i = 0; i < Case::nIn; ++i)
            tape.registerInput(inputs[p][i]);

          Case::template test<TestReal>(inputs[p], outputs);

          tape.registerOutput(outputs[o]);

          tape.setPassive();

          outputs[o].setGradient(1.0);

          #ifndef BUILD_REFERENCE
            opdi::logic->prepareEvaluate();
          #endif
          tape.evaluate();

          for (int i = 0; i < Case::nIn; ++i)
          {
            jacobian[o][i] = inputs[p][i].getGradient();
            inputs[p][i].setGradient(0.0);
          }

          primal[o] = outputs[o].getValue();

          tape.reset();
          #ifndef BUILD_REFERENCE
            opdi::logic->reset();
          #endif
        }

        std::cout << "Point " << p << " :" << std::endl;
        for (int o = 0; o < Case::nOut; ++o)
          std::cout << primal[o] << std::endl;

        for (int o = 0; o < Case::nOut; ++o) {
          for (int i = 0; i < Case::nIn; ++i) {
            std::cout << jacobian[o][i] << std::endl;
          }
        }
      }

      #ifndef BUILD_REFERENCE
        #ifdef OUTPUT_INSTRUMENT
          delete opdi::ompLogicInstruments.front();
          opdi::ompLogicInstruments.clear();
        #endif
        opdi::tool->finalize();
        opdi::logic->finalize();
        opdi::backend->finalize();
        #ifdef OPDI_USE_MACRO_BACKEND
          delete opdi::backend;
        #endif
        delete opdi::tool;
        delete opdi::logic;
      #endif
    }
};

#ifndef BUILD_REFERENCE
  #include "opdi.cpp"
#endif
