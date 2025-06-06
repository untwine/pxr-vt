// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modified by Jeremy Retailleau.

#ifndef PXR_VT_VALUE_FROM_PYTHON_H
#define PXR_VT_VALUE_FROM_PYTHON_H

/// \file vt/valueFromPython.h

#include "./api.h"
#include "./value.h"

#include <pxr/tf/hash.h>
#include <pxr/tf/hashmap.h>
#include <pxr/tf/pyUtils.h>
#include <pxr/tf/singleton.h>

#include <pxr/tf/pySafePython.h>

#include <vector>

namespace pxr {

/// \class Vt_ValueFromPythonRegistry
///
class Vt_ValueFromPythonRegistry {
public:

    static bool HasConversions() {
        return !_GetInstance()._lvalueExtractors.empty() &&
               !_GetInstance()._rvalueExtractors.empty();
    }
    
    VT_API static VtValue Invoke(PyObject *obj);

    template <class T>
    static void Register(bool registerRvalue) {
        if (!TfPyIsInitialized()) {
            TF_FATAL_ERROR("Tried to register a VtValue from python conversion "
                           "but python is not initialized!");
        }
        _GetInstance()._RegisterLValue(_Extractor::MakeLValue<T>());
        if (registerRvalue)
            _GetInstance()._RegisterRValue(_Extractor::MakeRValue<T>());
    }

    Vt_ValueFromPythonRegistry(Vt_ValueFromPythonRegistry const&) = delete;
    Vt_ValueFromPythonRegistry& operator=(
        Vt_ValueFromPythonRegistry const&) = delete;

    Vt_ValueFromPythonRegistry(Vt_ValueFromPythonRegistry &&) = delete;
    Vt_ValueFromPythonRegistry& operator=(
        Vt_ValueFromPythonRegistry &&) = delete;

private:
    Vt_ValueFromPythonRegistry() {}
    VT_API ~Vt_ValueFromPythonRegistry();

    friend class TfSingleton<Vt_ValueFromPythonRegistry>;

    class _Extractor {
    private:
        using _ExtractFunc = VtValue (*)(PyObject *);

        // _ExtractLValue will attempt to obtain an l-value T from the python
        // object it's passed.  This effectively disallows type conversions
        // (other than things like derived-to-base type conversions).
        template <class T>
        static VtValue _ExtractLValue(PyObject *);

        // _ExtractRValue will attempt to obtain an r-value T from the python
        // object it's passed.  This allows boost.python to invoke type
        // conversions to produce the T.
        template <class T>
        static VtValue _ExtractRValue(PyObject *);

    public:

        template <class T>
        static _Extractor MakeLValue() {
            return _Extractor(&_ExtractLValue<T>);
        }
        
        template <class T>
        static _Extractor MakeRValue() {
            return _Extractor(&_ExtractRValue<T>);
        }
        
        VtValue Invoke(PyObject *obj) const {
            return _extract(obj);
        }

    private:
        explicit _Extractor(_ExtractFunc extract) : _extract(extract) {}

        _ExtractFunc _extract;
    };

    VT_API static Vt_ValueFromPythonRegistry &_GetInstance() {
        return TfSingleton<Vt_ValueFromPythonRegistry>::GetInstance();
    }
    
    VT_API void _RegisterLValue(_Extractor const &e);
    VT_API void _RegisterRValue(_Extractor const &e);
    
    std::vector<_Extractor> _lvalueExtractors;
    std::vector<_Extractor> _rvalueExtractors;

    typedef TfHashMap<PyObject *, _Extractor, TfHash> _LValueExtractorCache;
    _LValueExtractorCache _lvalueExtractorCache;

};

VT_API_TEMPLATE_CLASS(TfSingleton<Vt_ValueFromPythonRegistry>);

template <class T>
VtValue Vt_ValueFromPythonRegistry::
_Extractor::_ExtractLValue(PyObject *obj) {
    pxr::boost::python::extract<T &> x(obj);
    if (x.check())
        return VtValue(x());
    return VtValue();
}

template <class T>
VtValue Vt_ValueFromPythonRegistry::
_Extractor::_ExtractRValue(PyObject *obj) {
    pxr::boost::python::extract<T> x(obj);
    if (x.check())
        return VtValue(x());
    return VtValue();
}

template <class T>
void VtValueFromPython() {
    Vt_ValueFromPythonRegistry::Register<T>(/* registerRvalue = */ true);
}

template <class T>
void VtValueFromPythonLValue() {
    Vt_ValueFromPythonRegistry::Register<T>(/* registerRvalue = */ false);
}

}  // namespace pxr

#endif // PXR_VT_VALUE_FROM_PYTHON_H
