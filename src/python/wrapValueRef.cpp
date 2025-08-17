//
// Copyright 2025 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include <pxr/vt/pxr.h>
#include <pxr/vt/valueRef.h>

#include <pxr/boost/python/def.hpp>

VT_NAMESPACE_USING_DIRECTIVE

using namespace pxr_boost::python;

static VtValue
_test_ValueRefFromPython(VtValueRef valueRef)
{
    return valueRef;
}

void wrapValueRef()
{
    def("_test_ValueRefFromPython", _test_ValueRefFromPython);
}
