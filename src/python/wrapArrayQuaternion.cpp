// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modified by Jeremy Retailleau.

#define ADDITION_OPERATOR
#define SUBTRACTION_OPERATOR
#define MULTIPLICATION_OPERATOR
#define DOUBLE_MULT_OPERATOR
#define DOUBLE_DIV_OPERATOR

#include <pxr/vt/typeHeaders.h>
#include <pxr/vt/wrapArray.h>
#include <pxr/tf/preprocessorUtilsLite.h>

using namespace pxr;

void wrapArrayQuaternion() {
    TF_PP_SEQ_FOR_EACH(VT_WRAP_ARRAY, ~, VT_QUATERNION_VALUE_TYPES);
}
