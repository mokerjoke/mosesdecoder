# Python interface to Moses

The idea is to have some of Moses' internals exposed to Python (inspired on pycdec).

## What's been interfaced?

* Binary phrase table:

        Moses::PhraseDictionaryTree.h

## Building

1.  Build the python extension

        python setup.py build_ext -i [--with-cmph]

3.  Check the example code

        echo "casa" | python example.py examples/phrase-table 5 1
        echo "essa casa" | python example.py examples/phrase-table 5 1

## Changing the code

If you want to add your changes you are going to have to recompile the cython code.

1.  Compile the cython code (use Cython 0.16): this will generate binpt/binpt.cpp

        cython --cplus binpt/binpt.pyx
