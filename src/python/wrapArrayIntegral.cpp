// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modified by Jeremy Retailleau.

#define NUMERIC_OPERATORS
#define MOD_OPERATOR

#include <pxr/vt/typeHeaders.h>
#include <pxr/vt/wrapArray.h>
#include <pxr/tf/preprocessorUtilsLite.h>

using namespace pxr;

void wrapArrayIntegral() {
    TF_PP_SEQ_FOR_EACH(VT_WRAP_ARRAY, ~,
                       VT_INTEGRAL_BUILTIN_VALUE_TYPES);
}
