# multipartparser

This is a simple email parser written in C++ for use mainly on embedded 
systems. It decoded inplace and so is very light on memory usage, I 
wrote this code quite some time ago when I was supposed to be doing 
something much more useful but probably much more boring.

# Compiling and Running the test

Execute this from the same directory as this file.

    g++ src/main.cpp src/mimepart.cpp src/mimeparser.cpp -o main

# The Tests

I have not done lots of tests but the test email contains a base64 
encoded image and some Quoted printable plain text and HTML.

To run the tests from the same directory this file is in, compile as 
above and then run the following:

    ./main test/test1.eml
    ./main test/test2.eml
    testsrc/testheaders testdata/test3.eml

# Bugs and Problems

This was only a simple project to help me become familiar with C++ 
the only C++ I did before this was Win32 API based stuff and that 
really was a long time ago. I have been able to make use of it though 
as a CGI script on unspam.us. Just wait for version 2!

The code compiled without errors in the old 3.x series of GCC but in
modern versions you will get the following warning:

    warning: deprecated conversion from string constant to ‘char*’

This is fine and I will fix it at some point.
