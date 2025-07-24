# OpDiLib

[OpDiLib](https://scicomp.rptu.de/software/opdi) (Open Multiprocessing Differentiation Library) is a universal add-on for reverse mode operator overloading AD tools that enables the differentiation of OpenMP parallel code.

It makes use of modern OpenMP features around OMPT to deduce a parallel reverse pass without any additional modifications of the parallel source code. Additionally, we provide a second mode of operation that works via replacement macros for OpenMP's directives and clauses as well as replacements for OpenMP's runtime functions. This mode of operation can also be used with compilers that do not support OMPT. There are no restrictions on data access patterns so that a first differentiated parallel code is obtained with little to no effort. As a next step, the parallel performance of the reverse pass can be optimized with various tools. One important aspect is disabling atomic updates on adjoint variables where appropriate. If the underlying AD tool is capable of differentiating MPI, for example via the add-on [MeDiPack](https://scicomp.rptu.de/software/medi/), OpDiLib can also be employed for a differentiation of OpenMP-MPI hybrid parallel codes.

The [Scientific Computing Group](https://scicomp.rptu.de) at the University of Kaiserslautern-Landau (RPTU) develops OpDiLib and will enhance and extend OpDiLib in the future.
There is a newsletter available at [opdi-info@scicomp.uni-kl.de](https://lists.uni-kl.de/scicomp/subscribe/opdi-info).
If you want to contact us, please write a mail to [opdi@scicomp.uni-kl.de](mailto:opdi@scicomp.uni-kl.de).

## OpenMP Support

OpDiLib supports all directives, clauses and runtime functions of the OpenMP 2.5 specification with the exception of

- `atomic` directives,
- `flush` directives.

## Usage

If you have a code that is differentiated with a serial AD tool and parallelize it using OpenMP, the procedure of obtaining an efficient parallel differentiated code with OpDiLib is as follows.

1. **Couple OpDiLib with your AD tool.** This step can be skipped if you use an AD tool that already has OpDiLib bindings, for example [CoDiPack](https://scicomp.rptu.de/software/codi/), which has OpDiLib support since [version 2.1](https://github.com/SciCompKL/CoDiPack/releases/tag/v2.1.0).
2. **Obtain a first parallel differentiated version of your code.** If your compiler supports OMPT, it suffices to add a few lines of code for the initialization and finalization of OpDiLib. Otherwise, you have to use OpDiLib's macro backend, which involves rewriting your OpenMP constructs according to OpDiLib's macro interface. Both approaches are demonstrated in the minimal example below.
3. **Optimize the performance of the parallel reverse pass.** Check your parallel forward code for parts that do not involve shared reading. Use OpDiLib's adjoint access control tools to disable atomic adjoints for these parts. You may also revise your data access patterns to eliminate additional instances of shared reading.

## Publications

For further details about OpDiLib's design, features, and modes of operation, please refer to our publication
[Event-Based Automatic Differentiation of OpenMP with OpDiLib](https://doi.org/10.1145/3570159).

~~~~{.txt}
@article{BluehdornSG2023,
  title = {{Event-Based Automatic Differentiation of OpenMP with OpDiLib}},
  author = {Bl\"{u}hdorn, Johannes and Sagebaum, Max and Gauger, Nicolas R.},
  year = {2023},
  month = {03},
  journal = {ACM Trans. Math. Softw.},
  volume = {49},
  number = {1},
  note = {Article No.: 3},
  pages = {1--31},
  publisher = {Association for Computing Machinery},
  address = {New York, NY, USA},
  doi = {10.1145/3570159}
}
~~~~

Additional details on applying OpDiLib in a large software with an advanced AD workflow are given in our publication
[Hybrid parallel discrete adjoints in SU2](https://doi.org/10.1016/j.compfluid.2024.106528).

~~~~{.txt}
@article{BluehdornGAG2025,
  title = {Hybrid parallel discrete adjoints in {SU2}},
  author = {Bl\"{u}hdorn, Johannes and Gomes, Pedro and Aehle, Max and Gauger, Nicolas R.},
  year = {2025},
  month = {01},
  journal = {Computers & Fluids},
  volume = {289},
  note = {Article 106528},
  pages = {1--18},
  publisher = {Elsevier},
  doi = {10.1016/j.compfluid.2024.106528}
}
~~~~

If you use OpDiLib in one of your applications and write a paper, please cite us!

## Minimal Example

The following minimal example assumes that [CoDiPack](https://scicomp.rptu.de/software/codi/) is used as the underlying AD tool. You need CoDiPack [version 2.1](https://github.com/SciCompKL/CoDiPack/releases/tag/v2.1.0) or newer. For additional examples, please refer to OpDiLib's test suite.

### OMPT Backend

~~~~{.cpp}
#include <codi.hpp>
#include <iostream>

#include <opdi/backend/ompt/omptBackend.hpp>
#include <opdi.hpp>

using Real = codi::RealReverseIndexOpenMP;  // use a suitable CoDiPack type
using Tape = typename Real::Tape;

int main(int nargs, char** args) {

  // initialize OpDiLib

  if (omp_get_num_threads() /* trigger OMPT initialization */ && opdi::backend == nullptr) {
    std::cout << "Could not initialize OMPT backend. Please check OMPT support." << std::endl;
    exit(1);
  }

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

  #pragma omp parallel
  {
    Real localSum = 0.0;

    #pragma omp for
    for (size_t i = 0; i < N; ++i)
    {
      localSum += sin(x * i);
    }

    #pragma omp critical
    {
      y += localSum;
    }
  }

  // usual AD workflow

  tape.registerOutput(y);
  tape.setPassive();
  y.setGradient(1.0);

  opdi::logic->prepareEvaluate();  // prepare OpDiLib for evaluation
  tape.evaluate();
  opdi::logic->postEvaluate();  // OpDiLib-specific postprocessing

  std::cout << "f(" << x << ") = " << y << std::endl;
  std::cout << "df/dx(" << x << ") = " << x.getGradient() << std::endl;

  // finalize OpDiLib

  opdi::tool->finalize();
  opdi::logic->finalize();
  opdi::backend->finalize();
  delete opdi::tool;
  delete opdi::logic;

  return 0;
}

// don't forget to include the OpDiLib source file
#include "opdi.cpp"
~~~~

The following command can be used to compile the code.

~~~~{.txt}
clang++  -I<path to codi>/include -I<path to opdi>/include -DCODI_EnableOpenMP -DCODI_EnableOpDiLib --std=c++17 -fopenmp -o omptexample omptexample.cpp
~~~~

### Macro Backend

~~~~{.cpp}
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
  opdi::logic->postEvaluate();  // OpDiLib-specific postprocessing

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
~~~~

The following command can be used to compile the code.

~~~~{.txt}
clang++  -I<path to codi>/include -I<path to opdi>/include -DCODI_EnableOpenMP -DCODI_EnableOpDiLib --std=c++17 -fopenmp -o macroexample macroexample.cpp
~~~~

