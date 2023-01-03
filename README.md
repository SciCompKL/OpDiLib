# OpDiLib

[OpDiLib](https://www.scicomp.uni-kl.de/software/opdi) (Open Multiprocessing Differentiation Library) is a universal add-on for reverse mode operator overloading AD tools that enables the differentiation of OpenMP parallel code.

It makes use of modern OpenMP features around OMPT to deduce a parallel reverse pass without any additional modifications of the parallel source code. Additionally, we provide a second mode of operation that works via replacement macros for OpenMP's directives and clauses as well as replacements for OpenMP's runtime functions. This mode of operation can also be used with compilers that do not support OMPT. There are no restrictions on data access patterns so that a first differentiated parallel code is obtained with little to no effort. As a next step, the parallel performance of the reverse pass can be optimized with various tools. One important aspect is disabling atomic updates on adjoint variables where appropriate. If the underlying AD tool is capable of differentiating MPI, for example via the add-on [MeDiPack](https://www.scicomp.uni-kl.de/software/medi/), OpDiLib can also be employed for a differentiation of OpenMP-MPI hybrid parallel codes.

The [Scientific Computing Group](https://www.scicomp.uni-kl.de) at the University of Kaiserslautern-Landau (RPTU) develops OpDiLib and will enhance and extend OpDiLib in the future.
There is a newsletter available at [opdi-info@scicomp.uni-kl.de](https://lists.uni-kl.de/scicomp/subscribe/opdi-info).
If you want to contact us, please write a mail to [opdi@scicomp.uni-kl.de](mailto:opdi@scicomp.uni-kl.de).

## OpenMP Support

OpDiLib supports all directives, clauses and runtime functions of the OpenMP 2.5 specification with the exception of

- `atomic` directives,
- `flush` directives.

## Usage

If you have a code that is differentiated with a serial AD tool and parallelize it using OpenMP, the procedure of obtaining an efficient parallel differentiated code with OpDiLib is as follows.

1. **Couple OpDiLib with your AD tool.** This step can be skipped if you use an AD tool that already has OpDiLib bindings, for example the thread-safe version of [CoDiPack](https://www.scicomp.uni-kl.de/software/codi/). It can be found in [this](https://github.com/scicompkl/codipack/tree/experimentalOpenMPSupport) branch.
2. **Obtain a first parallel differentiated version of your code.** If your compiler supports OMPT, it suffices to add a few lines of code for the initialization and finalization of OpDiLib. Otherwise, you have to use OpDiLib's macro backend, which involves rewriting your OpenMP constructs according to OpDiLib's macro interface. Both approaches are demonstrated in the minimal example below.
3. **Optimize the performance of the parallel reverse pass.** Check your parallel forward code for parts that do not involve shared reading. Use OpDiLib's adjoint access control tools to disable atomic adjoints for these parts. You may also revise your data access patterns to eliminate additional instances of shared reading.

## Publication

For further details about OpDiLib's design, features and modes of operation, please refer to our publication
[Event-Based Automatic Differentiation of OpenMP with OpDiLib](https://arxiv.org/abs/2102.11572).
If you use OpDiLib in one of your applications and write a paper, please cite us!

~~~~{.txt}
@misc{BluehdornSG2021,
  title = {{Event-Based Automatic Differentiation of OpenMP with OpDiLib}},
  author = {Bl{\"u}hdorn, Johannes and Sagebaum, Max and Gauger, Nicolas R.},
  url = {https://arxiv.org/abs/2102.11572},
  year = {2021},
  note={Preprint arXiv:2102.11572}
}
~~~~

## Minimal Example

The following minimal example assumes that the thread-safe version of [CoDiPack](https://www.scicomp.uni-kl.de/software/codi/) is used as the underlying AD tool. For additional examples, please refer to OpDiLib's test suite.

### OMPT Backend

~~~~{.cpp}
#include <codi.hpp>
#include <iostream>

#include <opdi/backend/ompt/omptBackend.hpp>
#include <codi/externals/codiOpdiTool.hpp>
#include <opdi.hpp>

using Real = codi::RealReverseIndexParallel;
using Tape = typename Real::TapeType;

int main(int nargs, char** args) {

  // initialize OpDiLib

  opdi::logic = new opdi::OmpLogic;
  opdi::logic->init();
  opdi::tool = new CoDiOpDiTool<Real>;

  // initialize thread-safe version of CoDiPack

  Tape& tape = Real::getGlobalTape();
  tape.initialize();

  // usual AD workflow

  Real x = 4.0;

  tape.setActive();
  tape.registerInput(x);

  // parallel computation

  Real a[1000];
  Real y = 0.0;

  #pragma omp parallel
  {
    #pragma omp for
    for (int i = 0; i < 1000; ++i)
    {
      a[i] = sin(x * i);
    }
  }

  for (int i = 0; i < 1000; ++i) {
    y += a[i];
  }

  // usual AD workflow

  tape.registerOutput(y);
  tape.setPassive();
  y.setGradient(1.0);
  tape.evaluate();

  std::cout << "f(" << x << ") = " << y << std::endl;
  std::cout << "df/dx(" << x << ") = " << x.getGradient() << std::endl;

  // finalize OpDiLib

  opdi::backend->finalize();
  delete opdi::logic;
  delete opdi::tool;

  return 0;
}

// don't forget to include the OpDiLib source file
#include "opdi.cpp"
~~~~

The following command can be used to compile the code.

~~~~{.txt}
clang++  -I<path to codi>/include -I<path to opdi>/include --std=c++11 -fopenmp -o omptexample omptexample.cpp
~~~~

### Macro Backend

~~~~{.cpp}
#include <codi.hpp>
#include <iostream>

#include <opdi/backend/macro/macroBackend.hpp>
#include <codi/externals/codiOpdiTool.hpp>
#include <opdi.hpp>

using Real = codi::RealReverseIndexParallel;
using Tape = typename Real::TapeType;

int main(int nargs, char** args) {

  // initialize OpDiLib

  opdi::backend = new opdi::MacroBackend();
  opdi::backend->init();
  opdi::logic = new opdi::OmpLogic;
  opdi::logic->init();
  opdi::tool = new CoDiOpDiTool<Real>;

  // initialize thread-safe version of CoDiPack

  Tape& tape = Real::getGlobalTape();
  tape.initialize();

  // usual AD workflow

  Real x = 4.0;

  tape.setActive();
  tape.registerInput(x);

  // parallel computation

  Real a[1000];
  Real y = 0.0;

  OPDI_PARALLEL()
  {
    OPDI_FOR()
    for (int i = 0; i < 1000; ++i)
    {
      a[i] = sin(x * i);
    }
    OPDI_END_FOR
  }
  OPDI_END_PARALLEL

  for (int i = 0; i < 1000; ++i) {
    y += a[i];
  }

  // usual AD workflow

  tape.registerOutput(y);
  tape.setPassive();
  y.setGradient(1.0);
  tape.evaluate();

  std::cout << "f(" << x << ") = " << y << std::endl;
  std::cout << "df/dx(" << x << ") = " << x.getGradient() << std::endl;

  // finalize OpDiLib

  opdi::backend->finalize();
  delete opdi::backend;
  delete opdi::logic;
  delete opdi::tool;

  return 0;
}

// don't forget to include the OpDiLib source file
#include "opdi.cpp"
~~~~

The following command can be used to compile the code.

~~~~{.txt}
clang++  -I<path to codi>/include -I<path to opdi>/include --std=c++11 -fopenmp -o macroexample macroexample.cpp
~~~~

