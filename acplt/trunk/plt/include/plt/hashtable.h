/* -*-plt-c++-*- */
#ifndef PLT_HASHTABLE_INCLUDED
#define PLT_HASHTABLE_INCLUDED
/*
 * Copyright (c) 1996, 1997
 * Chair of Process Control Engineering,
 * Aachen University of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must print or display the above
 *    copyright notice either during startup or must have a means for
 *    the user to view the copyright notice.
 * 3. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the Chair of Process Control Engineering nor the
 *    name of the Aachen University of Technology may be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CHAIR OF PROCESS CONTROL ENGINEERING
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE CHAIR OF PROCESS CONTROL
 * ENGINEERING BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Author: Martin Kneissl <martin@plt.rwth-aachen.de> */

//////////////////////////////////////////////////////////////////////
// plt/hashtable.h provides a dictionary using hash table implementation
//////////////////////////////////////////////////////////////////////

#include "plt/dictionary.h"
#include "plt/handle.h"

#include <stdlib.h>

//////////////////////////////////////////////////////////////////////
// forward declarations
//////////////////////////////////////////////////////////////////////

class PltHashTable_base;           // "private" implementation class
class PltHashTable_;               // "private" implementation template
class PltHashIterator_base;        // "private" implementation class

//////////////////////////////////////////////////////////////////////
// class PltHashTable<K,V>
//////////////////////////////////////////////////////////////////////
//
// A PltHashTable<K,V> maps from keys of class K to values of 
// class V. It is a PltDictionary<K,V>.
//
// Keys must provide
// the hash function and a comparison function for K.
//
// K::hash and K::operator== must have the following property:
//    k1 == k2 => k1.hash() == k2.hash()
//
// K::hash should be 'almost injective'
//
// Constructor:
// ------------
// The constructor takes 3 optional arguments:
//     mincap: 
//         the minimum capacity of the table, it will never shrink
//         below this limit
//     highwater:
//         when there are more than highwater * capacity elements in
//         the table, the table will grow
//     lowwater:
//         when there are less then lowwater * capacity elements in
//         the table, the table will shrink
//
// Operations:
// -----------
// see PltDictionary<K,V>
//
//////////////////////////////////////////////////////////////////////

template <class K, class V>
class PltHashTable
: public PltHashTable_<K,V>
{
protected:
    virtual unsigned long keyHash(const K & ) const ;
    virtual bool keyEqual(const K &, const K &) const;
};
    
//////////////////////////////////////////////////////////////////////

#if 0
template <class K, class V>
class PltHandleHashTable
: public PltHashTable_< PltPtrHandle<K>, V >
{
protected:
    virtual unsigned long keyHash(const PltPtrHandle<K> & ) const ;
    virtual bool keyEqual(const PltPtrHandle<K> &, 
                          const PltPtrHandle<K> &) const;
};
#endif

//////////////////////////////////////////////////////////////////////

template <class T>
class PltKeyHandle
: public PltPtrHandle<T>
{
public:
    PltKeyHandle();
    PltKeyHandle(T *p, enum PltOwnership);  // no default to avoid conversion!
    PltKeyHandle(const PltPtrHandle<T> &);
    PltKeyHandle(const PltKeyHandle &);

    unsigned long hash() const;
    bool operator == (const PltKeyHandle & h) const;
};
    
//////////////////////////////////////////////////////////////////////

template <class K, class V>
class PltHashIterator 
: public PltIterator< PltAssoc<K,V> >, 
  private PltHashIterator_base
{
public:
    PltHashIterator(const PltHashTable_<K,V> & t);
    virtual operator const void * () const;                 // termination
    virtual const PltAssoc<K,V> * operator -> () const;     // current element

    virtual PltIterator< PltAssoc<K,V> > & operator ++ ();  // advance
    virtual void toStart();                                 // from beginning
};

//////////////////////////////////////////////////////////////////////
// IMPLEMENTATION PART -- clients should stop reading here...
//////////////////////////////////////////////////////////////////////

template <class K, class V>
class PltHashTable_
: public PltDictionary<K,V>,
  private PltHashTable_base
{
    friend PltHashIterator_base::PltHashIterator_base(const PltHashTable_base &);
public:

    PltHashTable_(size_t mincap=11, 
                 float highwater=0.8, 
                 float lowwater=0.4);
    // accessors
    virtual bool query(const K&, V&) const;
    
    // modifiers
    virtual bool add(const K&, const V&);
    virtual bool remove(const K&, V&);

    // container interface
    PltHashIterator<K,V> * newIterator() const;
    size_t size() const;

protected:
    virtual unsigned long keyHash(const K &) const = 0;
    virtual bool keyEqual(const K & , const K &) const = 0;

private:
    virtual unsigned long keyHash(const void * ) const;
    virtual bool keyEqual(const void *, const void *) const;
    PltHashTable_(const PltHashTable_ &); // forbidden
    PltHashTable_<K,V> & operator = ( const PltHashTable_ & ); // forbidden
};
    

//////////////////////////////////////////////////////////////////////
// (PltHashTable_base is a private class)
//////////////////////////////////////////////////////////////////////
// Implementation class for PltHashTable<K,V>
//////////////////////////////////////////////////////////////////////

class PltHashTable_base 
{
    friend class PltHashIterator_base;
protected:
    PltHashTable_base(size_t mincap=11, 
                      float highwater=0.8, 
                      float lowwater=0.4);
    virtual ~PltHashTable_base();

#if PLT_DEBUG_INVARIANTS
    virtual bool invariant() const;
#endif
    // accessors
    size_t size() const; // number of nondeleted elements
    PltAssoc_ *lookupAssoc(const void * key) const;

    // modifiers
    bool addAssoc(PltAssoc_ *p);
    PltAssoc_ *removeAssoc(const void * key);

    virtual unsigned long keyHash(const void *) const = 0;
    virtual bool keyEqual(const void *, const void *) const = 0;
private:
    PltAssoc_ **a_table;
    size_t a_capacity;       // current capacity of a_table
    size_t a_minCapacity;    // minimal capacity
    float a_lowwater;        // \  try to keep the capacity between
    float a_highwater;       // /  lowwater * size and highwater * size
    float a_medwater;        // median of high- and lowwater
    size_t a_used;           // current number of used entries;
    size_t a_deleted;        // number of deleted elements

    // accessors
    size_t locate(const void * key) const;
    size_t collidx(size_t i, size_t j) const;
    static bool usedSlot(const PltAssoc_ *);

    // modifiers
    bool insert(PltAssoc_ *);
    bool changeCapacity(size_t mincap);

    PltHashTable_base(const PltHashTable_base &); // forbidden
    PltHashTable_base & operator = ( const PltHashTable_base & ); // forbidden
};

//////////////////////////////////////////////////////////////////////

class PltHashIterator_base
{
protected:
    PltHashIterator_base(const PltHashTable_base &);
    bool inRange() const;
    const PltAssoc_ * pCurrent() const;
    void advance();
    void toStart();
private:
    const PltHashTable_base & a_container;
    size_t a_index;
#if PLT_DEBUG_INVARIANTS
public:
    virtual bool invariant() const;
    virtual ~PltHashIterator_base() { };
#endif
};
    

//////////////////////////////////////////////////////////////////////
// INLINE IMPLEMENTATION
//////////////////////////////////////////////////////////////////////

inline size_t
PltHashTable_base::size() const
{
    return a_used-a_deleted;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline
PltHashTable_<K,V>::PltHashTable_(size_t mincap, 
                                float highwater, 
                                float lowwater)
: PltHashTable_base(mincap, highwater, lowwater)
{
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline bool
PltHashTable_<K,V>::query(const K& key, V& value) const
{
    // This cast is safe, only we can put assocs into the table
    PltAssoc<K,V> *p = 
        ( PltAssoc<K,V> *) lookupAssoc(&key);
    if (p) {
        value = p->a_value;
        return true;
    } else {
        return false;
    }
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline bool
PltHashTable_<K,V>::add(const K& key, const V& value)
{
    PltAssoc<K,V> *p = new PltAssoc<K,V>(key,value);
    if (p) {
        if ( addAssoc(p) ) {
            return true;
        } else {
            delete p;
            return false;
        }
    } else {
        return false;
    }
}
    
//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline bool
PltHashTable_<K,V>::remove(const K& key, V& value)
{
    // This cast is safe, only we can put assocs into the table
    PltAssoc<K,V> *p = 
        (PltAssoc<K,V> *) removeAssoc(&key);
    if (p) {
        value = p->a_value;
        delete p;
        return true;
    } else {
        return false;
    }
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline PltHashIterator<K,V> * 
PltHashTable_<K,V>::newIterator() const
{
    return new PltHashIterator<K,V>(*this);
}


//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline size_t
PltHashTable_<K,V>::size() const
{
    return PltHashTable_base::size();
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline unsigned long
PltHashTable_<K,V>::keyHash(const void * key) const
{
    return keyHash(* (const K *) key);
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline bool
PltHashTable_<K,V>::keyEqual(const void * k1, const void * k2) const
{
    return keyEqual(* (const K *) k1, * (const K *) k2);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline unsigned long
PltHashTable<K,V>::keyHash(const K & key) const
{
    return key.hash();
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline bool
PltHashTable<K,V>::keyEqual(const K & k1, const K & k2) const
{
    return k1 == k2;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#if 0
template <class K, class V>
inline unsigned long
PltHandleHashTable<K,V>::keyHash(const PltPtrHandle<K> & key) const
{
    return key->hash();
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline bool
PltHandleHashTable<K,V>::keyEqual(const PltPtrHandle<K> & k1, 
                                  const PltPtrHandle<K> & k2) const
{
    return (* k1) == (* k2);
}
#endif
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T>
inline
PltKeyHandle<T>::PltKeyHandle()
: PltPtrHandle<T>()
{
}

//////////////////////////////////////////////////////////////////////

template <class T>
inline
PltKeyHandle<T>::PltKeyHandle(T *p, enum PltOwnership os)
: PltPtrHandle<T>(p,os)
{
}

//////////////////////////////////////////////////////////////////////

template <class T>
inline
PltKeyHandle<T>::PltKeyHandle(const PltKeyHandle &p)
: PltPtrHandle<T>(p)
{
}


//////////////////////////////////////////////////////////////////////

template <class T>
inline
PltKeyHandle<T>::PltKeyHandle(const PltPtrHandle<T> &p)
: PltPtrHandle<T>(p)
{
}

//////////////////////////////////////////////////////////////////////

template <class T>
inline unsigned long
PltKeyHandle<T>::hash() const
{
    return (*this)->hash();
}

//////////////////////////////////////////////////////////////////////

template <class T>
inline bool
PltKeyHandle<T>::operator == (const PltKeyHandle & rhs) const
{
    return this->operator*() == *rhs;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline
PltHashIterator<K,V>::PltHashIterator(const PltHashTable_<K,V> &t)
: PltHashIterator_base( (const PltHashTable_base &) t)
{
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline
PltHashIterator<K,V>::operator const void * () const 
{
    return inRange() ? this : 0;
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline const PltAssoc<K,V> *
PltHashIterator<K,V>::operator -> () const
{
    return (const PltAssoc<K,V> *) pCurrent();
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline void
PltHashIterator<K,V>::toStart()
{
    PltHashIterator_base::toStart();
}

//////////////////////////////////////////////////////////////////////

template <class K, class V>
inline PltIterator< PltAssoc<K,V> > &
PltHashIterator<K,V>::operator ++ ()
{
    advance();
    return *this;
}

//////////////////////////////////////////////////////////////////////


#endif // header file