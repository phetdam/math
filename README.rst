.. README.rst for my "math" repo

math
====

.. image:: ./banner.png
   :alt: ./banner.png
   :align: center

Some mathematical work that I have done on my own for fun.

   Note:

   Still deciding on a new banner.

See the `Compiling to PDF`_  section for instructions on how to compile the
``.tex`` source in ``tex`` into PDF.

Any mathematical errors are wholly my own.

Contents
--------

``tex``
~~~~~~~

Contains the ``.tex`` source for my independent mathematical work. The
``exercises`` directory is for worked exercises from textbooks, monographs,
etc., while the ``original`` directory is for my own original work.


Compiling to PDF
----------------

You may compile the ``.tex`` source to PDF files using the provided
``build_pdf.sh`` shell script if you have the ``bash`` shell [#]_ and
``pdflatex``. After using ``git clone`` to clone the repository contents, ``cd``
into the top-level repository directory, use ``chmod`` to set permissions
appropriately, for example with ``chmod 777 build_pdf.sh``, and execute

.. code:: bash

   ./build_pdf.sh

``build_pdf.sh`` will then compile all ``.tex`` files in ``tex`` and write its
output to the ``pdf`` directory, which will mirror the directory tree of
``tex``. Any directories that do not exist will be created. If you do not wish
to adjust permissions, use ``bash build_pdf.sh`` instead of executing.

You can view help on more advanced use of ``build_pdf.sh`` with
``build_pdf.sh -h`` or ``build_pdf.sh --help``.

.. [#] The ``bash`` shell is necessary since the improved ``[[`` is used once
   in ``build_pdf.sh``.