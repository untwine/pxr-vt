// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modified by Jeremy Retailleau.

#include <pxr/vt/typeHeaders.h>
#include <pxr/vt/wrapArray.h>

using namespace pxr;

void wrapArrayToken() {
    VtWrapArray<VtArray<TfToken> >();
}
