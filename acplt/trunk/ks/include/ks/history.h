/* -*-plt-c++-*- */
/* $Header: /home/david/cvs/acplt/ks/include/ks/history.h,v 1.6 1999-09-06 06:51:28 harald Exp $ */
#ifndef KS_HISTORY_INCLUDED
#define KS_HISTORY_INCLUDED
/*
 * Copyright (c) 1996, 1997, 1998, 1999
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

/* Author : Markus Juergens <markusj@plt.rwth-aachen.de> */

/////////////////////////////////////////////////////////////////////////////

#include "ks/client.h"
#include "ks/commobject.h"
#include "ks/histparams.h"

/////////////////////////////////////////////////////////////////////////////

class KscHistory
    : public KscCommObject
{
public:
    KscHistory(const char *object_path);

    KS_OBJ_TYPE typeCode() const { return KS_OT_HISTORY; }

    bool setEngProps(KsEngPropsHandle) { return true; }
    const KsEngProps_THISTYPE *getEngProps() const;
    bool getEngPropsUpdate();

    // Add or update a selector
    bool setSelector(KsString id, KsSelectorHandle hsel);
    // Implicit handle creation with usual caveats
    bool setSelector(KsString id, KsSelector *psel);
    // Remove a selector
    bool removeSelector(KsString id);
    // Add a string selector(convenience function)
    bool setStringSelector(KsString id, KsString mask);
    // Add a none selector
    bool setNoneSelector(KsString id);
    // Get a selector if exists
    KsSelectorHandle getSelector(KsString id) const;

    // Read max entries at most
    void setMaxEntries(u_long max = ULONG_MAX);

    // Read parts of a history
    bool getParts(KsList<KsEngPropsHandle> &parts);

    // Read history with currently set parameters,
    // result is not stored in object
    bool getHist(KsGetHistResult &result);

    // Read history with current parameters and
    // store result
    bool updateHist();
    // Access selectors if possible
    KS_RESULT getSelectorValue(KsString selector, KsValueHandle &hval);
    KS_RESULT getTimeValue(KsString selector, KsTimeVecValue &val);
    KS_RESULT getStringValue(KsString selector, KsStringVecValue &val);
    KS_RESULT getDoubleValue(KsString selector, KsDoubleVecValue &val);
    KS_RESULT getUIntValue(KsString selector, KsUIntVecValue &val);
    KS_RESULT getIntValue(KsString selector, KsIntVecValue &val);
    KS_RESULT getStateValue(KsString selector, KsStateVecValue &val);

protected:
    u_long                 max_entries;

    KsList<KsGetHistItem>  selectors;

    KsGetHistSingleResult  gh_result;

    KsEngPropsHandle      hpp;
};

/////////////////////////////////////////////////////////////////////////////
// INLINE IMPLEMENTATION
/////////////////////////////////////////////////////////////////////////////

inline
KscHistory::KscHistory(const char *object_path)
    : KscCommObject(object_path),
      max_entries(ULONG_MAX)
{}

/////////////////////////////////////////////////////////////////////////////

inline bool 
KscHistory::setSelector(KsString id, KsSelectorHandle hsel)
{
    if( hsel ) {
        KsGetHistItem item(id,hsel);
        selectors.remove(item); // remove any item with same id
        return selectors.addLast(item);
    } else {
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////

inline bool 
KscHistory::setSelector(KsString id, KsSelector *psel)
{
    KsSelectorHandle hsel(psel, KsOsNew);

    return setSelector(id, hsel);
}

/////////////////////////////////////////////////////////////////////////////

inline 
bool
KscHistory::removeSelector(KsString id)
{
    return selectors.remove(KsGetHistItem(id, KsSelectorHandle()));
}

/////////////////////////////////////////////////////////////////////////////

inline 
bool
KscHistory::setNoneSelector(KsString id)
{
    return setSelector(id, new KsNoneSel());
}

/////////////////////////////////////////////////////////////////////////////

inline
void
KscHistory::setMaxEntries(u_long max)
{
    max_entries = max;
}


#endif // KS_HISTORY_INCLUDED
// End of history.h