.. README.rst for my "math" repo

math
====

.. image:: ./banner.png
   :alt: ./banner.png
   :align: center

Some mathematical work that I have done on my own for fun.

   Note:

   Still deciding on a new banner.

See the Building_ section for instructions on how to compile the repo's C++ and
LaTeX sources with CMake_.

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
which is a thin convenience script around CTest_.

.. _GoogleTest: https://google.github.io/googletest/

.. _CTest: https://cmake.org/cmake/help/latest/manual/ctest.1.html

``tex``
~~~~~~~

Contains the ``.tex`` source for my independent mathematical work. The
``exercises`` directory is for worked exercises from textbooks, monographs,
etc., while the ``original`` directory is for my own original work. The
``utils`` directory is for LaTeX setup or command definitions, which may be
used in other ``.tex`` files, typically by using ``\input``.


Building
--------

To perform a local build with this repo the following dependencies are required:

   Note:

   Dependencies on Boost and Eigen are planned to be reduced, removed, and/or
   made optional in the future. Originally, the C++ code was written at a time
   when the author had just started learning C++17 and didn't understand
   template metaprogramming quite as well compared to his knowledge today.

* A C++17 compiler
* CMake_ >= 3.21
* Boost_ >= 1.71
* Eigen_ [#]_ >= 3.4

.. _Boost: https://www.boost.org/

.. _Eigen: https://libeigen.gitlab.io/docs/

.. [#] The Eigen documentation used to live at eigen.tuxfamily.org but the
   domain no longer seems reachable. Up-to-date ``master`` branch documentation
   for Eigen is now hosted on their GitLab Pages.

Unit tests will be included in the build if the following are available:

* GoogleTest_ >= 1.10.0

The LaTeX source in the ``tex`` subdirectory will also be built inplace into
PDF files in the repo tree if pdfTeX_ and BibTeX_ are locatable via
``find_package`` and/or searching the directories in your ``PATH``.

.. _pdfTeX: https://www.tug.org/applications/pdftex/
.. _BibTeX: https://www.bibtex.org/

Local builds have been tested with the following compiler + platforms:

* GCC_ 9.3.0 on WSL1 Ubuntu 20.04.3 LTS
* GCC 11.3.0 on WSL1 Ubuntu 22.04.2 LTS
* Visual Studio 2022 on Windows 10 Home
* Visual Studio 2022 on Windows 11 Home

.. _GCC: https://gcc.gnu.org/

For POSIX systems, use the included ``build.sh`` build script. To build a
release build, use:

.. code:: bash

   ./build.sh -c Release

You can use the ``-Ca`` option to pass additional arguments to the CMake
configuration step, e.g. hints for `find_package`_ like
``-DGTest_ROOT=/path/to/googletest``, and so on.

.. _find_package: https://cmake.org/cmake/help/latest/command/find_package.html

On Windows, the included ``build.bat`` can be used in the same manner:

.. code:: shell

   build -c Release

Note that arguments containing ``=`` have to be quoted for CMD. Therefore,
CMake ``-D`` arguments should be passed in double quotes, e.g. like
``"-DGTest_ROOT=C:\path\to\googletest"``, else CMD will be confused.


Building PDFs from TeX source
-----------------------------

   Note:

   Using ``build_tex.sh`` is deprecated in favor of the CMake integration which
   enables near-optimal minimal rebuild while correctly determining how many
   times pdfLaTeX and BibTeX each need to be invoked. ``build_tex.sh`` is a
   dumb script that simply loops through subdirectories.

   Do note the CMake integration produces the exact same in-tree results as
   building through an editor with latexmk_ or using ``build_tex.sh``; I found
   it difficult to reliably get all outputs written to a separate build
   directory because some LaTeX packages like minted_ don't integrate well with
   the pdfTeX ``-output-directory`` option.

.. _latexmk: https://www.cantab.net/users/johncollins/latexmk/
.. _minted: https://ctan.org/pkg/minted?lang=en

You may compile the ``.tex`` source to PDF files using the provided
``build_tex.sh`` shell script if you have the ``bash`` shell [#]_ and
``pdflatex``. After using ``git clone`` to clone the repository contents, ``cd``
into the top-level repository directory, use ``chmod`` to set permissions
appropriately, for example with ``chmod 777 build_tex.sh``, and execute

.. code:: bash

   ./build_tex.sh

``build_tex.sh`` will then compile all ``.tex`` files in ``tex`` and write its
output to the ``pdf`` directory, which will mirror the directory tree of
``tex``. The ``pdf`` directory and its subdirectories will be created as needed.
If you do not wish to adjust permissions, use ``bash build_tex.sh`` instead of
executing with ``./``.

You can view help on more advanced use of ``build_tex.sh`` with
``build_tex.sh -h`` or ``build_tex.sh --help``.

.. [#] The ``bash`` shell is necessary since the improved ``[[`` is used once
   in ``build_tex.sh``.
