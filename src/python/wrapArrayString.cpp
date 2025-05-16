// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modified by Jeremy Retailleau.

#define ADDITION_OPERATOR

#include <pxr/vt/typeHeaders.h>
#include <pxr/vt/wrapArray.h>

#include <string>
using std::string;

using namespace pxr;

void wrapArrayString() {
    VtWrapArray<VtArray<string> >();
}
