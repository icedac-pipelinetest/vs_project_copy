# vs_project_copy
visual studio c++ proejct copy tool


# usage 
```
# ProjectCopier AnlFrameEdgeTest moduleTest
try to ProjectCopy from [srcDir:AnlFrameEdgeTest] to [moduleTest] (filter: ;*.h;*.inl;*.hpp;*.cpp;*.c;*.filters;*.vcxproj)
processing... [moduleTest\AnlFrameEdgeTest.*]
        [...] moduleTest\AnlFrameEdgeTest.cpp ...ok
        [...] moduleTest\AnlFrameEdgeTest.vcxproj ...ok
        [...] moduleTest\AnlFrameEdgeTest.vcxproj.filters ...ok
processing... [moduleTest\*.vcxproj]
        [...] moduleTest\moduleTest.vcxproj ...ok
 - give a new GUID: 2320CD6E-BEEE-4318-A632-B6074932B412
 - moduleTest\moduleTest.vcxproj file modified.
processing... [moduleTest\*.filters]
        [...] moduleTest\moduleTest.vcxproj.filters ...ok
 - moduleTest\moduleTest.vcxproj.filters file modified.
processing... [moduleTest\*.c]
processing... [moduleTest\*.cpp]
        [...] moduleTest\AnlFrameEdge.cpp ...ok
        [...] moduleTest\moduleTest.cpp ...ok
 - moduleTest\moduleTest.cpp file modified.
        [...] moduleTest\stdafx.cpp ...ok
 - moduleTest\stdafx.cpp file modified.
processing... [moduleTest\*.hpp]
processing... [moduleTest\*.inl]
processing... [moduleTest\*.h]
        [...] moduleTest\AnlFrameEdge.h ...ok
        [...] moduleTest\stdafx.h ...ok
#
```
