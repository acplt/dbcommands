/* -*-plt-c++-*- */
#ifndef KS_INADDRSET_INCLUDED
#define KS_INADDRSET_INCLUDED
/* $Header: /home/david/cvs/acplt/ks/include/ks/inaddrset.h,v 1.1 1997-09-02 15:08:39 martin Exp $ */
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

//////////////////////////////////////////////////////////////////////

#include <iostream.h>
#include "ks/rpc.h"
#include "plt/list.h"

//////////////////////////////////////////////////////////////////////

class KsInAddrSet
{
public:
    ~KsInAddrSet() { removeAll(); }

    // accessor

    bool isMember(in_addr addr) const;

    // modifier

    bool add(in_addr addr, in_addr mask);
    bool add(in_addr addr);
    bool remove(in_addr addr, in_addr mask);
    bool remove(in_addr addr);
    void removeAll();
    

private:
    bool addItem(in_addr addr, in_addr mask, bool incl);
    class Item 
        : public PltListNode_base 
            { 
            public:
                Item(in_addr addr, in_addr mask, bool incl)
                    : _addr(addr), _mask(mask), _incl(incl)
                        { }
                in_addr _addr;
                in_addr _mask; 
                bool _incl;
            };
    PltIList<Item> _list;
};

istream & operator >> (istream & istr, KsInAddrSet & set);

//////////////////////////////////////////////////////////////////////
// INLINE IMPLEMENTATION
//////////////////////////////////////////////////////////////////////

inline bool
KsInAddrSet::add(in_addr addr, in_addr mask)
{
    return addItem(addr, mask, true);
}

//////////////////////////////////////////////////////////////////////

inline bool
KsInAddrSet::remove(in_addr addr, in_addr mask)
{
    return addItem(addr, mask, false);
}

//////////////////////////////////////////////////////////////////////

inline bool
KsInAddrSet::add(in_addr addr)
{
    in_addr mask;
    mask.s_addr = 0xffffffff;
    return add(addr, mask);
}

//////////////////////////////////////////////////////////////////////

inline bool
KsInAddrSet::remove(in_addr addr)
{
    in_addr mask;
    mask.s_addr = 0xffffffff;
    return remove(addr, mask);
}

//////////////////////////////////////////////////////////////////////
#endif // KS_INADDRSET_INCLUDED




