/* -*-plt-c++-*- */
#ifndef PLT_RTTI_INCLUDED
#define PLT_RTTI_INCLUDED
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

#include "plt/debug.h"

#if PLT_SIMULATE_RTTI

//////////////////////////////////////////////////////////////////////
// Runtime type information is still not present in all 
// implementations of C++. Because the dynamic_cast feature
// is sometimes needed we provide some simulation of RTTI.
// This simulation needs help from class implementors.
//
// What will you have to do for every class containing new virtual 
// functions that might be referred to by pointers to base classes?
//
// You have to:
// - create a static null terminated array containing 
//   pointers to PltRTTI objects of all base classes.
// - Provide a static data member of class PltRTTI.
// - Bracket this code with #if PLT_SIMULATE_RTTI / #endif
// 
// TODO: Some example to make this clear.
//////////////////////////////////////////////////////////////////////


class PltRTTI {
public:
    PltRTTI(const char *name,const PltRTTI* bases[]);
    bool isSame(const PltRTTI *) const;
    bool isBaseOf(const PltRTTI *) const;
    bool canCastFrom(const PltRTTI *) const;
    const char *name() const;
private:
    const char *n;
    const PltRTTI **b;
};


//////////////////////////////////////////////////////////////////////
// INLINE IMPLEMENTATION
//////////////////////////////////////////////////////////////////////

inline bool
PltRTTI::isSame(const PltRTTI *p) const
{
    return this==p;
}

//////////////////////////////////////////////////////////////////////

inline bool
PltRTTI::canCastFrom(const PltRTTI *p) const
{
    return isSame(p) || isBaseOf(p);
}

//////////////////////////////////////////////////////////////////////
// MACROS
//////////////////////////////////////////////////////////////////////

#define PLT_DYNAMIC_PCAST(T,p)                                         \
           ((&T::RTTI)->canCastFrom((p)->getRTTI()) ? (T*)((p)) : 0)

#if 0
#define PLT_DYNAMIC_RCAST(T,r)                                         \
           (PLT_ASSERT((&T::RTTI)->canCastFrom((&(r))->getRTTI())),    \
           (T&)((r)))
#endif

#define PLT_DECL_RTTI                                                  \
     public:                                                           \
         virtual const PltRTTI* getRTTI() const                        \
             { return &RTTI; }                                         \
         static const PltRTTI RTTI

#define PLT_IMPL_RTTI0(clsname)                                        \
     const PltRTTI clsname::RTTI(#clsname,0)

#define PLT_IMPL_RTTI1(clsname,base1)                                  \
     static const PltRTTI * clsname##_bases [] = {                     \
         &base1::RTTI,                                                 \
         0 };                                                          \
     const PltRTTI clsname::RTTI(#clsname,clsname##_bases)

#define PLT_IMPL_RTTI2(clsname,base1,base2)                            \
     static const PltRTTI * clsname##_bases [] = {                     \
         &base1::RTTI,                                                 \
         &base2::RTTI,                                                 \
         0 };                                                          \
     const PltRTTI clsname::RTTI(#clsname,clsname##_bases)

#define PLT_IMPL_RTTI3(clsname,base1,base2,base3)                      \
     static const PltRTTI * clsname##_bases [] = {                     \
         &base1::RTTI,                                                 \
         &base2::RTTI,                                                 \
         &base3::RTTI,                                                 \
         0 };                                                          \
     const PltRTTI clsname::RTTI(#clsname,clsname##_bases)

#else // PLT_SIMULATE_RTTI

#define PLT_DECL_RTTI typedef void plt_rtti_dummy
#define PLT_IMPL_RTTI0(c) typedef void plt_rtti_dummy
#define PLT_IMPL_RTTI1(c,b1) typedef void plt_rtti_dummy
#define PLT_IMPL_RTTI2(c,b1,b2) typedef void plt_rtti_dummy
#define PLT_IMPL_RTTI3(c,b1,b2,b3) typedef void plt_rtti_dummy

#define PLT_DYNAMIC_PCAST(T,p) \
           (dynamic_cast<T*> (p))

#endif // PLT_SIMULATE_RTTI

#endif // PLT_RTTI_INCLUDED

