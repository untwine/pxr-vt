// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modified by Jeremy Retailleau.

#include <pxr/tf/pyModule.h>

namespace pxr {

// Defined in arrayPyBuffer.cpp
void Vt_AddBufferProtocolSupportToVtArrays();

}  // namespace pxr

using namespace pxr;

TF_WRAP_MODULE
{
    TF_WRAP(Array);
    TF_WRAP(ArrayDualQuaternion);
    TF_WRAP(ArrayFloat);
    TF_WRAP(ArrayIntegral);
    TF_WRAP(ArrayMatrix);
    TF_WRAP(ArrayQuaternion);
    TF_WRAP(ArrayRange);
    TF_WRAP(ArrayString);
    TF_WRAP(ArrayToken);
    TF_WRAP(ArrayVec);

    // Add python buffer protocol support to VtArray.
    Vt_AddBufferProtocolSupportToVtArrays();

    TF_WRAP(Dictionary);
    TF_WRAP(Value);
}
