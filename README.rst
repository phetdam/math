.. README.rst for my "math" repo

math
====

.. image:: ./banner.png
   :alt: ./banner.png
   :align: center

Some of the better bits of mathematical work that I have done on my own for fun.

   Note:

   I might be updating the banner soon. Keep an eye out :)

Any errors are wholly my own. This repository will always be a work in progress,
but will usually likely see few updates, as most topics are not substantial
enough to do a full article on. Article topics will most likely fall within the
categories of financial math or statistical learning, which are my main
recreational interests.

See below section for instructions on how to compile ``.tex`` source to PDF.

Compiling to PDF
----------------

You may compile the ``.tex`` source to PDF files using the provided
``compile.sh`` shell script if you have the ``bash`` shell [#]_ and
``pdflatex``. After using ``git clone`` to clone the repository contents, ``cd``
into the top-level repository directory, use ``chmod`` to set permissions
appropriately, for example with ``chmod 777 compile.sh``, and execute in your
``bash`` shell

.. code:: bash

   ./compile.sh

You may ``source`` the script if you like, but since there are variables being
defined in the script, it's better to adjust the permissions and just ``./`` it.
``compile.sh`` will then compile all ``.tex`` files and write its output to the
directory specfied by ``$OUT_DIR`` variable defined within it.

.. [#] The ``bash`` shell is necessary since the improved ``[[`` is used in some
   places within ``compile.sh``.