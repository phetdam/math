.. README.rst for my "math" repo

math
====

.. image:: ./banner.png
   :alt: ./banner.png
   :align: center

Some mathematical work that I have done on my own for fun.

   Note:

   Still deciding on a new banner.

See the `Building PDFs from TeX source`_ section for how to compile the ``.tex``
source in ``tex`` into PDF and see the `Building the C++ source`_ section for
how to build the accompanying C++ source code using `CMake`_.

Any mathematical errors are wholly my own.

.. _CMake: https://cmake.org/


Contents
--------

``examples``
~~~~~~~~~~~~
Contains C++ source and ``CMakeLists.txt`` for building CLI executables. These
typically will compute some numerical result or demonstrate an implementation
as instructed in an exercise or other application.

``include``
~~~~~~~~~~~
Contains C++ headers and ``CMakeLists.txt`` for library code used by the CLI
executables in ``examples``. There is no ``src`` directory since all the code
currently written is either templated or implemented as ``inline``.

``test``
~~~~~~~~
Contains C++ source and ``CMakeLists.txt`` for unit tests written with
GoogleTest_. If tests have been built, they can be run with ``run_tests.sh``,
which is a thin convenience script around ctest_.

.. _GoogleTest: https://google.github.io/googletest/

.. _ctest: https://cmake.org/cmake/help/latest/manual/ctest.1.html

``tex``
~~~~~~~

Contains the ``.tex`` source for my independent mathematical work. The
``exercises`` directory is for worked exercises from textbooks, monographs,
etc., while the ``original`` directory is for my own original work. The
``utils`` directory is for LaTeX setup or command definitions, which may be
used in other ``.tex`` files, typically by using ``\input``.


Building the C++ source
-----------------------

TBA. This project depends on `Boost`_ 1.71 headers and is intended to be built
using `CMake`_ 3.16 or newer using a C++17-compliant compiler. Local builds
on WSL 1 Ubuntu 20.04.3 LTS used `GCC`_ 9.3.0. Optionally, if one has
GoogleTest_ installed, unit test runners can be built and run.

.. _Boost: https://www.boost.org/

.. _GCC: https://gcc.gnu.org/


Building PDFs from TeX source
-----------------------------

You may compile the ``.tex`` source to PDF files using the provided
``build_pdf.sh`` shell script if you have the ``bash`` shell [#]_ and
``pdflatex``. After using ``git clone`` to clone the repository contents, ``cd``
into the top-level repository directory, use ``chmod`` to set permissions
appropriately, for example with ``chmod 777 build_pdf.sh``, and execute

.. code:: bash

   ./build_pdf.sh

``build_pdf.sh`` will then compile all ``.tex`` files in ``tex`` and write its
output to the ``pdf`` directory, which will mirror the directory tree of
``tex``. The ``pdf`` directory and its subdirectories will be created as needed.
If you do not wish to adjust permissions, use ``bash build_pdf.sh`` instead of
executing with ``./``.

You can view help on more advanced use of ``build_pdf.sh`` with
``build_pdf.sh -h`` or ``build_pdf.sh --help``.

.. [#] The ``bash`` shell is necessary since the improved ``[[`` is used once
   in ``build_pdf.sh``.