%include "std_string.i"
%include "enumtypeunsafe.swg"

%javaconst(1);

%module GraphicsExtender
%{
#include "interface.h"
%}

%pragma(java) jniclassclassmodifiers="class"

%include interface.h
%include module_code.i
%include jniclass_code.i
