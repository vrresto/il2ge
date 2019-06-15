%include "std_string.i"

%module GraphicsExtender
%{
#include "swig_interface.h"
%}

%pragma(java) jniclassclassmodifiers="class"
%pragma(java) moduleimports="import org.il2ge.GraphicsExtenderJNI;"

%include "swig_interface.h"
%include init_code.i
