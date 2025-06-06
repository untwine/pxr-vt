// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modified by Jeremy Retailleau.

#ifndef PXR_VT_DICTIONARY_H
#define PXR_VT_DICTIONARY_H

/// \file vt/dictionary.h

#include "./api.h"
#include "./value.h"

#include <pxr/tf/diagnostic.h>
#include <pxr/tf/hash.h>
#include <pxr/tf/mallocTag.h>

#include <initializer_list>
#include <iosfwd>
#include <map>
#include <memory>

namespace pxr {

/// \defgroup group_vtdict_functions VtDictionary Functions
/// Functions for manipulating VtDictionary objects.
 
/// \class VtDictionary
///
/// A map with string keys and VtValue values.
///
/// VtDictionary converts to and from a python dictionary as long
/// as each element contains either
///   - another VtDictionary  (converts to a nested dictionary)
///   - std::vector<VtValue>  (converts to a nested list)
///   - VtValue with one of the supported Vt Types.
///
/// For a list of functions that can manipulate VtDictionary objects, see the  
/// \link group_vtdict_functions VtDictionary Functions \endlink group page .
///
class VtDictionary {
    typedef std::map<std::string, VtValue, std::less<>> _Map;
    std::unique_ptr<_Map> _dictMap;

public:
    // The iterator class, used to make both const and non-const iterators.
    // Currently only forward traversal is supported. In order to support lazy
    // allocation, VtDictionary's Map pointer (_dictMap) must be nullable,
    // but that would break the VtDictionary iterators. So instead, VtDictionary
    // uses this Iterator class, which considers an iterator to an empty
    // VtDictionary to be the same as an iterator at the end of a VtDictionary
    // (i.e. if Iterator's _dictMap pointer is null, that either means that the
    // VtDictionary is empty, or the Iterator is at the end of a VtDictionary
    // that contains values).
    template<class UnderlyingMapPtr, class UnderlyingIterator>
    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename UnderlyingIterator::value_type;
        using reference = typename UnderlyingIterator::reference;
        using pointer = typename UnderlyingIterator::pointer;
        using difference_type = typename UnderlyingIterator::difference_type;


        // Default constructor creates an Iterator equivalent to end() (i.e.
        // UnderlyingMapPtr is null)
        Iterator() = default;

        // Copy constructor (also allows for converting non-const to const).
        template <class OtherUnderlyingMapPtr, class OtherUnderlyingIterator>
        Iterator(Iterator<OtherUnderlyingMapPtr,
                          OtherUnderlyingIterator> const &other)
            : _underlyingIterator(other._underlyingIterator),
              _underlyingMap(other._underlyingMap) {}

        reference operator*() const { return *_underlyingIterator; }
        pointer operator->() const { return _underlyingIterator.operator->(); }

        Iterator& operator++() {
            increment();
            return *this;
        }

        Iterator operator++(int) {
            Iterator result = *this;
            increment();
            return result;
        }

        Iterator& operator--() {
            --_underlyingIterator;
            return *this;
        }

        Iterator operator--(int) {
            Iterator result = *this;
            --_underlyingIterator;
            return result;
        }

        template <class OtherUnderlyingMapPtr, class OtherUnderlyingIterator>
        bool operator==(const Iterator<OtherUnderlyingMapPtr,
                                       OtherUnderlyingIterator>& other) const {
            return equal(other);
        }

        template <class OtherUnderlyingMapPtr, class OtherUnderlyingIterator>
        bool operator!=(const Iterator<OtherUnderlyingMapPtr,
                                       OtherUnderlyingIterator>& other) const {
            return !equal(other);
        }

    private:

        // Private constructor allowing the find, begin and insert methods
        // to create and return the proper Iterator.
        Iterator(UnderlyingMapPtr m, UnderlyingIterator i)
            : _underlyingIterator(i),
              _underlyingMap(m) {
                if (m && i == m->end())
                    _underlyingMap = nullptr;
            }
       
        friend class VtDictionary;

        UnderlyingIterator GetUnderlyingIterator(UnderlyingMapPtr map)
        const {
            TF_AXIOM(!_underlyingMap || _underlyingMap == map);
            return (!_underlyingMap) ? map->end() : _underlyingIterator;
        }

        // Fundamental functionality to implement the iterator.
        // These will be invoked these as necessary to implement
        // the full iterator public interface.

        // Increments the underlying iterator, and sets the underlying map to
        // null when the iterator reaches the end of the map. 
        void increment() {
            if (!_underlyingMap) {
                TF_FATAL_ERROR("Attempted invalid increment operation on a "
                    "VtDictionary iterator");
                return;
            }
            if (++_underlyingIterator == _underlyingMap->end()) {
                _underlyingMap = nullptr;
            }
        }

        // Equality comparison. Iterators are considered equal if:
        // 1) They both point to empty VtDictionaries
        // 2) They both point to the end() of a VtDictionary
        // - or-
        // 3) They both point to the same VtDictionary and their
        //    underlying iterators are the same
        // In cases 1 and 2 above, _underlyingMap will be null
        template <class OtherUnderlyingMapPtr, class OtherUnderlyingIterator>
        bool equal(Iterator<OtherUnderlyingMapPtr,
                            OtherUnderlyingIterator> const& other) const {
            if (_underlyingMap == other._underlyingMap)
                if (!_underlyingMap ||
                    (_underlyingIterator == other._underlyingIterator))
                    return true;
            return false;
        }

        UnderlyingIterator _underlyingIterator;
        UnderlyingMapPtr _underlyingMap = nullptr;
    };

    TF_MALLOC_TAG_NEW("Vt", "VtDictionary");

    typedef _Map::key_type key_type;
    typedef _Map::mapped_type mapped_type;
    typedef _Map::value_type value_type;
    typedef _Map::allocator_type allocator_type;
    typedef _Map::size_type size_type;

    typedef Iterator<_Map*, _Map::iterator> iterator;
    typedef Iterator<_Map const*, _Map::const_iterator> const_iterator;

    /// Creates an empty \p VtDictionary.
    VtDictionary() {}

    /// Creates an empty \p VtDictionary with at least \p size buckets.
    explicit VtDictionary(int size) {}

    /// Creates a \p VtDictionary with a copy of a range. 
    template<class _InputIterator>
    VtDictionary(_InputIterator f, _InputIterator l){
        TfAutoMallocTag2 tag("Vt", "VtDictionary::VtDictionary (range)");
        insert(f, l);
    }

    /// Creates a copy of the supplied \p VtDictionary
    VT_API
    VtDictionary(VtDictionary const& other);

    /// Creates a new VtDictionary by moving the supplied \p VtDictionary.
    VT_API
    VtDictionary(VtDictionary && other) = default;

    /// Creates a new VtDictionary from a braced initializer list.
    VT_API
    VtDictionary(std::initializer_list<value_type> init);

    /// Copy assignment operator
    VT_API
    VtDictionary& operator=(VtDictionary const& other);

    /// Move assignment operator
    VT_API
    VtDictionary& operator=(VtDictionary && other) = default;

    /// Returns a reference to the \p VtValue that is associated with a 
    /// particular key.
    VT_API
    VtValue& operator[](const std::string& key);

    /// Counts the number of elements whose key is \p key. 
    VT_API
    size_type count(const std::string& key) const;

    /// Counts the number of elements whose key is \p key. 
    VT_API
    size_type count(const char* key) const;

    /// Erases the element whose key is \p key. 
    VT_API
    size_type erase(const std::string& key);

    /// Erases the element pointed to by \p it. 
    VT_API
    iterator erase(iterator it);

    /// Erases all elements in a range.
    VT_API
    iterator erase(iterator f, iterator l);

    /// Erases all of the elements. 
    VT_API
    void clear();

    /// Finds an element whose key is \p key. 
    VT_API
    iterator find(const std::string& key);

    /// Finds an element whose key is \p key. 
    VT_API
    iterator find(const char* key);

    /// Finds an element whose key is \p key. 
    VT_API
    const_iterator find(const std::string& key) const;

    /// Finds an element whose key is \p key. 
    VT_API
    const_iterator find(const char* key) const;

    /// Returns an \p iterator pointing to the beginning of the \p VtDictionary. 
    VT_API
    iterator begin();

    /// Returns an \p iterator pointing to the beginning of the \p VtDictionary. 
    VT_API
    const_iterator begin() const;

    /// Returns an \p iterator pointing to the end of the \p VtDictionary. 
    VT_API
    iterator end();
    
    /// Returns an \p iterator pointing to the end of the \p VtDictionary. 
    VT_API
    const_iterator end() const;

    /// Returns the size of the VtDictionary. 
    VT_API
    size_type size() const;
	
    /// \c true if the \p VtDictionary's size is 0. 
    VT_API
    bool empty() const;
    
    /// Swaps the contents of two \p VtDictionaries. 
    VT_API
    void swap(VtDictionary& dict); 

    // Global overload for swap for unqualified calls in generic code.
    friend void swap(VtDictionary &lhs, VtDictionary &rhs) {
        lhs.swap(rhs);
    }

    friend size_t hash_value(VtDictionary const &dict) {
        // Hash empty dict as zero.
        if (dict.empty())
            return 0;
        // Otherwise hash the map.
        return TfHash()(*dict._dictMap);
    }

    /// Inserts a range into the \p VtDictionary. 
    template<class _InputIterator>
    void insert(_InputIterator f, _InputIterator l) {
        TfAutoMallocTag2 tag("Vt", "VtDictionary::insert (range)");
        if (f != l) {
            _CreateDictIfNeeded();
            _dictMap->insert(f, l);
        }
    }

    /// Inserts \p obj into the \p VtDictionary. 
    VT_API
    std::pair<iterator, bool> insert(const value_type& obj);

    /// Return a pointer to the value at \p keyPath if one exists.  \p keyPath
    /// is a delimited string of sub-dictionary names.  Key path elements are
    /// produced by calling TfStringTokenize() with \p keyPath and
    /// \p delimiters.  \p keyPath may identify a leaf element or an entire
    /// sub-dictionary.  Return null if no such element at \p keyPath exists.
    VT_API
    VtValue const *
    GetValueAtPath(std::string const &keyPath,
                   char const *delimiters = ":") const;

    /// Return a pointer to the value at \p keyPath if one exists.  \p keyPath
    /// may identify a leaf element or an entire sub-dictionary.  Return null if
    /// no such element at \p keyPath exists.
    VT_API
    VtValue const *
    GetValueAtPath(std::vector<std::string> const &keyPath) const;

    /// Set the value at \p keyPath to \p value.  \p keyPath is a delimited
    /// string of sub-dictionary names.  Key path elements are produced by
    /// calling TfStringTokenize() with \p keyPath and \p delimiters.  Create
    /// sub-dictionaries as necessary according to the path elements in
    /// \p keyPath.  If \p keyPath identifies a full sub-dictionary, replace the
    /// entire sub-dictionary with \p value.
    VT_API
    void SetValueAtPath(std::string const &keyPath,
                        VtValue const &value, char const *delimiters = ":");

    /// Set the value at \p keyPath to \p value.  Create sub-dictionaries as
    /// necessary according to the path elements in \p keyPath.  If \p keyPath
    /// identifies a full sub-dictionary, replace the entire sub-dictionary with
    /// \p value.
    VT_API
    void SetValueAtPath(std::vector<std::string> const &keyPath,
                        VtValue const &value);

    /// Erase the value at \a keyPath.  \p keyPath is a delimited string of
    /// sub-dictionary names.  Key path elements are produced by calling
    /// TfStringTokenize() with \p keyPath and \p delimiters.  If no such
    /// element exists at \p keyPath, do nothing.  If \p keyPath identifies a
    /// sub-dictionary, erase the entire sub-dictionary.
    VT_API
    void EraseValueAtPath(std::string const &keyPath,
        char const *delimiters = ":");

    /// Erase the value at \a keyPath.  If no such element exists at \p keyPath,
    /// do nothing.  If \p keyPath identifies a sub-dictionary, erase the entire
    /// sub-dictionary.
    VT_API
    void EraseValueAtPath(std::vector<std::string> const &keyPath);

private:
    void
    _SetValueAtPathImpl(std::vector<std::string>::const_iterator curKeyElem,
                        std::vector<std::string>::const_iterator keyElemEnd,
                        VtValue const &value);

    void _EraseValueAtPathImpl(
        std::vector<std::string>::const_iterator curKeyElem,
        std::vector<std::string>::const_iterator keyElemEnd);

    void _CreateDictIfNeeded();

};

/// Equality comparison.
VT_API bool operator==(VtDictionary const &, VtDictionary const &);
VT_API bool operator!=(VtDictionary const &, VtDictionary const &);

/// Write the contents of a VtDictionary to a stream, formatted like "{ 'key1':
/// value1, 'key2': value2 }".
VT_API  std::ostream &operator<<(std::ostream &, VtDictionary const &);

//
// Return a const reference to an empty VtDictionary.
//
VT_API VtDictionary const &VtGetEmptyDictionary();

/// Returns true if \p dictionary contains \p key and the corresponding value
/// is of type \p T.
/// \ingroup group_vtdict_functions
///
template <typename T>
bool
VtDictionaryIsHolding( const VtDictionary &dictionary,
                       const std::string &key )
{
    VtDictionary::const_iterator i = dictionary.find(key);
    if ( i == dictionary.end() ) {
        return false;
    }

    return i->second.IsHolding<T>();
}

/// \overload
template <typename T>
bool
VtDictionaryIsHolding( const VtDictionary &dictionary,
                       const char *key )
{
    VtDictionary::const_iterator i = dictionary.find(key);
    if ( i == dictionary.end() ) {
        return false;
    }

    return i->second.IsHolding<T>();
}


/// Return a value held in a VtDictionary by reference.
///
/// If \p key is in \p dictionary and the corresponding value is of type
/// \p T, returns a reference to the value.
///
/// \remark If \p key is not in \p dictionary, or the value for \p key is of
/// the wrong type, a fatal error occurs, so clients should always call
/// VtDictionaryIsHolding first.
///
/// \ingroup group_vtdict_functions
template <typename T>
const T &
VtDictionaryGet( const VtDictionary &dictionary,
                 const std::string &key )
{
    VtDictionary::const_iterator i = dictionary.find(key);
    if (ARCH_UNLIKELY(i == dictionary.end())) {
        TF_FATAL_ERROR("Attempted to get value for key '" + key +
                       "', which is not in the dictionary.");
    }

    return i->second.Get<T>();
}

/// \overload
template <typename T>
const T &
VtDictionaryGet( const VtDictionary &dictionary,
                 const char *key )
{
    VtDictionary::const_iterator i = dictionary.find(key);
    if (ARCH_UNLIKELY(i == dictionary.end())) {
        TF_FATAL_ERROR("Attempted to get value for key '%s', "
                       "which is not in the dictionary.", key);
    }

    return i->second.Get<T>();
}


// This is an internal holder class that is used in the version of
// VtDictionaryGet that takes a default.
template <class T>
struct Vt_DefaultHolder {
    explicit Vt_DefaultHolder(T const &t) : val(t) {}    
    T const &val;
};

// This internal class has a very unusual assignment operator that returns an
// instance of Vt_DefaultHolder, holding any type T.  This is used to get the
// "VtDefault = X" syntax for VtDictionaryGet.
struct Vt_DefaultGenerator {
    template <class T>
    Vt_DefaultHolder<T> operator=(T const &t) {
        return Vt_DefaultHolder<T>(t);
    }
};

// This is a global stateless variable used to get the VtDefault = X syntax in
// VtDictionaryGet.
extern VT_API Vt_DefaultGenerator VtDefault;

/// Return a value held in a VtDictionary, or a default value either if the
/// supplied key is missing or if the types do not match.
///
/// For example, this code will get a bool value under key "key" if "key" has a
/// boolean value in the dictionary.   If there is no such key, or the value
/// under the key is not a bool, the specified default (false) is returned.
///
/// \code
///     bool val = VtDictionaryGet<bool>(dict, "key", VtDefault = false);
/// \endcode
///
/// \ingroup group_vtdict_functions
template <class T, class U>
T VtDictionaryGet( const VtDictionary &dictionary,
                   const std::string &key,
                   Vt_DefaultHolder<U> const &def )
{
    VtDictionary::const_iterator i = dictionary.find(key);
    if (i == dictionary.end() || !i->second.IsHolding<T>())
        return def.val;
    return i->second.UncheckedGet<T>();
}

/// \overload
template <class T, class U>
T VtDictionaryGet( const VtDictionary &dictionary,
                   const char *key,
                   Vt_DefaultHolder<U> const &def )
{
    VtDictionary::const_iterator i = dictionary.find(key);
    if (i == dictionary.end() || !i->second.IsHolding<T>())
        return def.val;
    return i->second.UncheckedGet<T>();
}



/// Creates a dictionary containing \p strong composed over \p weak.
///
/// The new dictionary will contain all key-value pairs from \p strong
/// together with the key-value pairs from \p weak whose keys are not in \p
/// strong.
///
/// If \p coerceToWeakerOpinionType is \c true then coerce a strong value to
/// the weaker value's type, if there is a weaker value.  This is mainly
/// intended to promote to enum types.
///
/// \ingroup group_vtdict_functions
VT_API VtDictionary
VtDictionaryOver(const VtDictionary &strong, const VtDictionary &weak,
                 bool coerceToWeakerOpinionType = false);

/// Updates \p strong to become \p strong composed over \p weak.
///
/// The updated contents of \p strong will be all key-value pairs from \p
/// strong together with the key-value pairs from \p weak whose keys are not in
/// \p strong.
///
/// If \p coerceToWeakerOpinionType is \c true then coerce a strong value to
/// the weaker value's type, if there is a weaker value.  This is mainly
/// intended to promote to enum types.
///
/// \ingroup group_vtdict_functions
VT_API void
VtDictionaryOver(VtDictionary *strong, const VtDictionary &weak,
                 bool coerceToWeakerOpinionType = false);

/// Updates \p weak to become \p strong composed over \p weak.
///
/// The updated contents of \p weak will be all key-value pairs from \p strong
/// together with the key-value pairs from \p weak whose keys are not in \p
/// strong.
///
/// If \p coerceToWeakerOpinionType is \c true then coerce a strong value to
/// the weaker value's type, if there is a weaker value.  This is mainly
/// intended to promote to enum types.
///
/// \ingroup group_vtdict_functions
VT_API void
VtDictionaryOver(const VtDictionary &strong, VtDictionary *weak,
                 bool coerceToWeakerOpinionType = false);

/// Returns a dictionary containing \p strong recursively composed over \p
/// weak.
///
/// The new dictionary will be all key-value pairs from \p strong together
/// with the key-value pairs from \p weak whose keys are not in \p strong.
///
/// If a value for a key is in turn a dictionary, and both \a strong and \a
/// weak have values for that key, then the result  may not contain strong's
/// exact value for the subdict. Rather, the result will contain a subdict
/// that is the result of a recursive call to this method.  Hence, the
/// subdict, too, will contain values from \a weak that are not found in \a
/// strong.
///
/// If \p coerceToWeakerOpinionType is \c true then coerce a strong value to
/// the weaker value's type, if there is a weaker value.  This is mainly
/// intended to promote to enum types.
///
/// \ingroup group_vtdict_functions
VT_API VtDictionary
VtDictionaryOverRecursive(const VtDictionary &strong, const VtDictionary &weak,
                          bool coerceToWeakerOpinionType = false);

/// Updates \p strong to become \p strong composed recursively over \p weak.
///
/// The updated contents of \p strong will be all key-value pairs from \p
/// strong together with the key-value pairs from \p weak whose keys are not
/// in \p strong.
///
/// If a value for a key is in turn a dictionary, and both \a strong and \a
/// weak have values for that key, then \a strong's subdict may not be left
/// untouched.  Rather, the dictionary will be replaced by the result of a
/// recursive call to this method in which \a strong's subdictionary will have
/// entries added if they are contained in \a weak but not in \a strong
///
/// If \p coerceToWeakerOpinionType is \c true then coerce a strong value to
/// the weaker value's type, if there is a weaker value.  This is mainly
/// intended to promote to enum types.
///
/// \ingroup group_vtdict_functions
VT_API void
VtDictionaryOverRecursive(VtDictionary *strong, const VtDictionary &weak,
                          bool coerceToWeakerOpinionType = false);

/// Updates \p weak to become \p strong composed recursively over \p weak.
///
/// The updated contents of \p weak will be all key-value pairs from \p strong
/// together with the key-value pairs from \p weak whose keys are not in \p
/// strong.
///
/// If a value is in turn a dictionary, the dictionary in \a weak may not be
/// replaced wholesale by that of \a strong. Rather, the dictionary will be
/// replaced by the result of a recursive call to this method in which \a
/// weak's subdictionary is recursively overlayed by \a strong's
/// subdictionary.
///
/// The result is that no key/value pairs of \a will be lost in nested
/// dictionaries. Rather, only non-dictionary values will be overwritten
///
/// If \p coerceToWeakerOpinionType is \c true then coerce a strong value to
/// the weaker value's type, if there is a weaker value.  This is mainly
/// intended to promote to enum types.
///
/// \ingroup group_vtdict_functions
VT_API void
VtDictionaryOverRecursive(const VtDictionary &strong, VtDictionary *weak,
                          bool coerceToWeakerOpinionType = false);


struct VtDictionaryHash {
    inline size_t operator()(VtDictionary const &dict) const {
        return hash_value(dict);
    }
};

}  // namespace pxr

#endif /* PXR_VT_DICTIONARY_H */
