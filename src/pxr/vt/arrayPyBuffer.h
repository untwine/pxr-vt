// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modified by Jeremy Retailleau.

#ifndef PXR_VT_ARRAY_PY_BUFFER_H
#define PXR_VT_ARRAY_PY_BUFFER_H

#include "./api.h"
#include "./array.h"
#include <pxr/tf/pyObjWrapper.h>

#include <optional>

namespace pxr {

/// Convert \p obj which should support the python buffer protocol (e.g. a
/// numpy array) to a VtArray if possible and return it.  Return empty
/// optional if \p obj does not support the buffer protocol or does not have
/// compatible type and dimensions.  If \p err is supplied, set it to an
/// explanatory message in case of conversion failure.  This function may be
/// invoked for VtArray<T> where T is one of VT_ARRAY_PYBUFFER_TYPES.
template <class T>
std::optional<VtArray<T> >
VtArrayFromPyBuffer(TfPyObjWrapper const &obj, std::string *err=nullptr);

/// The set of types for which it's valid to call VtArrayFromPyBuffer().
#define VT_ARRAY_PYBUFFER_TYPES                 \
    VT_BUILTIN_NUMERIC_VALUE_TYPES              \
    VT_VEC_VALUE_TYPES                          \
    VT_MATRIX_VALUE_TYPES                       \
    VT_GFRANGE_VALUE_TYPES                      \
    ((GfRect2i, Rect2i))                        \
    ((GfQuath, Quath))                          \
    ((GfQuatf, Quatf))                          \
    ((GfQuatd, Quatd))                          \
    ((GfDualQuath, DualQuath))                  \
    ((GfDualQuatf, DualQuatf))                  \
    ((GfDualQuatd, DualQuatd))


}  // namespace pxr

#endif // PXR_VT_ARRAY_PY_BUFFER_H
