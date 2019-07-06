%include "std_string.i"
%include "enumtypeunsafe.swg"

%javaconst(1);

%module GraphicsExtender
%{
#include "swig_interface.h"
%}

%pragma(java) jniclassclassmodifiers="class"
%pragma(java) moduleimports="import com.maddox.il2ge.GraphicsExtenderJNI;"

%include "swig_interface.h"
%include init_code.i
