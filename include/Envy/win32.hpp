///////////////////////////////////////////////////////////////////////////////////////
//
//    Envy Game Engine
//    https://github.com/PatrickTorgerson/Envy
//
//    Copyright (c) 2021 Patrick Torgerson
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

// The following #defines disable a bunch of unused windows stuff. If you
// get weird errors when trying to do some windows stuff, try removing some
// (or all) of these defines (it will increase build time though).
#ifndef ENVY_FULL_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    // #define NOGDICAPMASKS
    // #define NOSYSMETRICS
    // #define NOMENUS
    // #define NOICONS
    // #define NOSYSCOMMANDS
    // #define NORASTEROPS
    // #define OEMRESOURCE
    // #define NOATOM
    // #define NOCLIPBOARD
    // #define NOCOLOR
    // #define NOCTLMGR
    // #define NODRAWTEXT
    // #define NOKERNEL
    // #define NONLS
    // #define NOMEMMGR
    // #define NOMETAFILE
    // #define NOOPENFILE
    // #define NOSCROLL
    // #define NOSERVICE
    // #define NOSOUND
    // #define NOTEXTMETRIC
    // #define NOWH
    // #define NOCOMM
    // #define NOKANJI
    // #define NOHELP
    // #define NOPROFILER
    // #define NODEFERWINDOWPOS
    // #define NOMCX
    // #define NORPC
    // #define NOPROXYSTUB
    // #define NOIMAGE
    // #define NOTAPE
#endif

#define NOMINMAX
#define STRICT

#include <Windows.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef assert
#undef assert
#endif