// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modified by Jeremy Retailleau.

#include <pxr/vt/dictionary.h>
#include <pxr/vt/types.h>
#include <pxr/vt/value.h>

#include <pxr/tf/mallocTag.h>
#include <pxr/tf/pyContainerConversions.h>
#include <pxr/tf/pyUtils.h>

#include <pxr/tf/iterator.h>

#include <pxr/trace/trace.h>

#include <pxr/boost/python/dict.hpp>
#include <pxr/boost/python/def.hpp>
#include <pxr/boost/python/list.hpp>
#include <pxr/boost/python/to_python_converter.hpp>
#include <pxr/boost/python/converter/from_python.hpp>
#include <pxr/boost/python/converter/registered.hpp>
#include <pxr/boost/python/converter/rvalue_from_python_data.hpp>
#include <pxr/boost/python/detail/api_placeholder.hpp>

using namespace pxr;

using namespace pxr::boost::python;

namespace {

// Converter from std::vector<VtValue> to python list
struct VtValueArrayToPython
{
    static PyObject* convert(const std::vector<VtValue> &v)
    {
        // TODO Use result converter. TfPySequenceToList.
        list result;
        TF_FOR_ALL(i, v) {
            object o = TfPyObject(*i);
            result.append(o);
        }
        return incref(result.ptr());
    }
};

// Converter from std::vector<VtDictionary> to python list
struct VtDictionaryArrayToPython
{
    static PyObject* convert(const std::vector<VtDictionary> &v)
    {
        // TODO Use result converter. TfPySequenceToList.
        list result;
        TF_FOR_ALL(i, v) {
            object o = TfPyObject(*i);
            result.append(o);
        }
        return incref(result.ptr());
    }
};

// Converter from VtDictionary to python dict.
struct VtDictionaryToPython
{
    static PyObject* convert(const VtDictionary &v)
    {
        TRACE_FUNCTION();

        // TODO Use result converter TfPyMapToDictionary??
        dict result;
        TF_FOR_ALL(i, v) {
            object o = TfPyObject(i->second);
            result.setdefault(i->first, o);
        }
        return incref(result.ptr());
    }
};

static bool
_CanVtValueFromPython(object pVal);

// Converts a python object to a VtValue, with some special behavior.
// If the python object is a dictionary, puts a VtDictionary in the
// result.  If the python object is a list, puts an std::vector<VtValue>
// in the result.  If the python object can be converted to something
// VtValue knows about, does that.  In each of these cases, returns true.
//
// Otherwise, returns false.
static bool _VtValueFromPython(object pVal, VtValue *result) {
    // Try to convert a nested dictionary into a VtDictionary.
    extract<VtDictionary> valDictProxy(pVal);
    if (valDictProxy.check()) {
        if (result) {
            VtDictionary dict = valDictProxy;
            result->Swap(dict);
        }
        return true;
    }

    // Try to convert a nested list into a vector.
    extract<std::vector<VtValue> > valArrayProxy(pVal);
    if (valArrayProxy.check()) {
        if (result) {
            std::vector<VtValue> array = valArrayProxy;
            result->Swap(array);
        }
        return true;
    }

    // Try to convert a value into a VtValue.
    extract<VtValue> valProxy(pVal);
    if (valProxy.check()) {
        VtValue v = valProxy();
        if (v.IsHolding<TfPyObjWrapper>()) {
            return false;
        }
        if (result) {
            result->Swap(v);
        }
        return true;
    }
    return false;
}

// Converter from python dict to VtValueArray.
struct _VtValueArrayFromPython {
    _VtValueArrayFromPython() {
        converter::registry::insert(
            &convertible, &construct, type_id<std::vector<VtValue> >());
    }

    // Returns p if p can convert to an array, NULL otherwise.
    // If result is non-NULL, does the conversion into *result.
    static PyObject *convert(PyObject *p, std::vector<VtValue> *result) {
        extract<list> dProxy(p);
        if (!dProxy.check()) {
            return NULL;
        }
        list d = dProxy();
        int numElts = len(d);

        if (result)
            result->reserve(numElts);
        for (int i = 0; i < numElts; i++) {
            object pVal = d[i];
            if (result) {
                result->push_back(VtValue());
                if (!_VtValueFromPython(pVal, &result->back()))
                    return NULL;
                // Fall through to return p.
            } else {
                // Test for convertibility.
            }
        }
        return p;
    }
    static void *convertible(PyObject *p) {
        return convert(p, NULL);
    }

    static void construct(PyObject* source, converter::
                          rvalue_from_python_stage1_data* data) {
        TfAutoMallocTag2
            tag("Vt", "_VtValueArrayFromPython::construct");
        void* storage = (
            (converter::rvalue_from_python_storage<std::vector<VtValue> >*)
            data)->storage.bytes;
        new (storage) std::vector<VtValue>();
        data->convertible = storage;
        convert(source, (std::vector<VtValue>*)storage);
    }
};

// Converter from python dict to VtDictionary.
struct _VtDictionaryFromPython {
    _VtDictionaryFromPython() {
        converter::registry::insert(
            &convertible, &construct, type_id<VtDictionary>());
    }

    // Returns p if p can convert to a dictionary, NULL otherwise.
    // If result is non-NULL, does the conversion into *result.
    static PyObject *convert(PyObject *p, VtDictionary *result) {
        if (!PyDict_Check(p)) {
            return NULL;
        }

        Py_ssize_t pos = 0;
        PyObject *pyKey = NULL, *pyVal = NULL;
        while (PyDict_Next(p, &pos, &pyKey, &pyVal)) {
            extract<std::string> keyProxy(pyKey);
            if (!keyProxy.check())
                return NULL;
            object pVal(handle<>(borrowed(pyVal)));
            if (result) {
                VtValue &val = (*result)[keyProxy()];
                if (!_VtValueFromPython(pVal, &val))
                    return NULL;
            } else {
                if (!_CanVtValueFromPython(pVal))
                    return NULL;
            }
        }
        return p;
    }
    static void *convertible(PyObject *p) {
        TRACE_FUNCTION();
        return convert(p, NULL);
    }

    static void construct(PyObject* source, converter::
                          rvalue_from_python_stage1_data* data) {
        TRACE_FUNCTION();
        TfAutoMallocTag2
            tag("Vt", "_VtDictionaryFromPython::construct");
        void* storage = (
            (converter::rvalue_from_python_storage<VtDictionary>*)
            data)->storage.bytes;
        new (storage) VtDictionary(0);
        data->convertible = storage;
        convert(source, (VtDictionary*)storage);
    }
};

// Converter from python list to std::vector<VtDictionary>.
struct _VtDictionaryArrayFromPython {
    _VtDictionaryArrayFromPython() {
        converter::registry::insert(&convertible, &construct,
                                    type_id<std::vector<VtDictionary>>());
    }

    // Returns p if p can convert to an array, NULL otherwise.
    // If result is non-NULL, does the conversion into *result.
    static PyObject *convert(PyObject *p, std::vector<VtDictionary> *result) {
        extract<list> dProxy(p);
        if (!dProxy.check()) {
            return NULL;
        }
        list d = dProxy();
        int numElts = len(d);

        if (result) {
            result->reserve(numElts);
        }
        for (int i = 0; i < numElts; i++) {
            object pVal = d[i];
            extract<VtDictionary> e(pVal);
            if (!e.check()) {
                return nullptr;
            }
            if (result) {
                result->push_back(e());
            }
        }
        return p;
    }
    
    static void *convertible(PyObject *p) {
        return convert(p, NULL);
    }

    static void construct(PyObject* source, converter::
                          rvalue_from_python_stage1_data* data) {
        TfAutoMallocTag2
            tag("Vt", "_VtDictionaryArrayFromPython::construct");
        void* storage = (
            (converter::rvalue_from_python_storage<std::vector<VtDictionary> >*)
            data)->storage.bytes;
        new (storage) std::vector<VtDictionary>();
        data->convertible = storage;
        convert(source, static_cast<std::vector<VtDictionary>*>(storage));
    }
};


// Converter from python list to VtValue holding VtValueArray.
struct _VtValueHoldingVtValueArrayFromPython {
    _VtValueHoldingVtValueArrayFromPython() {
        converter::registry::insert(&_VtValueArrayFromPython::convertible,
                                    &construct, type_id<VtValue>());
    }

    static void construct(PyObject* source, converter::
                           rvalue_from_python_stage1_data* data) {
        TfAutoMallocTag2
            tag("Vt", "_VtValueHoldingVtValueArrayFromPython::construct");
        std::vector<VtValue> arr;
        _VtValueArrayFromPython::convert(source, &arr);
        void* storage = (
            (converter::rvalue_from_python_storage<VtValue>*)
            data)->storage.bytes;
        new (storage) VtValue();
        ((VtValue *)storage)->Swap(arr);
        data->convertible = storage;
    }
};

// Converter from python dict to VtValue holding VtDictionary.
struct _VtValueHoldingVtDictionaryFromPython {
    _VtValueHoldingVtDictionaryFromPython() {
        converter::registry::insert(&_VtDictionaryFromPython::convertible,
                                    &construct, type_id<VtValue>());
    }

    static void construct(PyObject* source, converter::
                           rvalue_from_python_stage1_data* data) {
        TfAutoMallocTag2
            tag("Vt", "_VtValueHoldingVtDictionaryFromPython::construct");
        VtDictionary dictionary;
        _VtDictionaryFromPython::convert(source, &dictionary);
        void* storage = (
            (converter::rvalue_from_python_storage<VtValue>*)
            data)->storage.bytes;
        new (storage) VtValue();
        ((VtValue *)storage)->Swap(dictionary);
        data->convertible = storage;
    }
};


static bool
_CanVtValueFromPython(object pVal)
{
    if (_VtDictionaryFromPython::convertible(pVal.ptr()))
        return true;

    if (_VtValueArrayFromPython::convertible(pVal.ptr()))
        return true;

    extract<VtValue> e(pVal);
    return e.check() && !e().IsHolding<TfPyObjWrapper>();
}


static VtDictionary
_ReturnDictionary(VtDictionary const &x) {
    return x;
}

static std::vector<VtDictionary>
_DictionaryArrayIdent(std::vector<VtDictionary> const &v) {
    return v;
}

} // anonymous namespace 

void wrapDictionary()
{
    def("_ReturnDictionary", _ReturnDictionary);
    def("_DictionaryArrayIdent", _DictionaryArrayIdent);

    to_python_converter<VtDictionary, VtDictionaryToPython>();
    to_python_converter<std::vector<VtDictionary>, VtDictionaryArrayToPython>();
    to_python_converter<std::vector<VtValue>, VtValueArrayToPython>();
    _VtValueArrayFromPython();
    _VtDictionaryFromPython();
    _VtDictionaryArrayFromPython();
    _VtValueHoldingVtValueArrayFromPython();
    _VtValueHoldingVtDictionaryFromPython();
}
