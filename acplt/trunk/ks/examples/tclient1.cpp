/* -*-plt-c++-*- */

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

/* Author: Markus Juergens <markusj@plt.rwth-aachen.de> */

//////////////////////////////////////////////////////////////////////
// sample client
//////////////////////////////////////////////////////////////////////

#include <iostream.h>
#include <plt/list.h>

#include "ks/client.h"
#include "ks/commobject.h"
#include "ks/package.h"
#include "ks/avsimplemodule.h"

//////////////////////////////////////////////////////////////////////

KsString host, server, hostAndServer;
KscAvSimpleModule av_simple_read("reader");
KscAvSimpleModule av_simple_write("writer");

typedef PltList<KscVariableHandle> VariableList;
typedef PltListIterator<KscVariableHandle> VariableIterator;

PltList<KscVariableHandle> allVariables;
PltList<KscVariableHandle> messung_temperatur1;
PltList<KscVariableHandle> messung_verriegelung;
PltList<KscVariableHandle> messung_tempregler;

//////////////////////////////////////////////////////////////////////

KscPackageHandle
buildDirectoryPackage(KsString);

//////////////////////////////////////////////////////////////////////

void
errMsg(const KsString &msg) 
{
    cout << "Error: " << msg << endl;
}

//////////////////////////////////////////////////////////////////////

void 
addVariableToList(const KsString &name,
                  VariableList &lst)
{
    KscVariableHandle vh(
        new KscVariable(name),
        KsOsNew);
    
    PLT_ASSERT(vh->hasValidPath());

    if(vh && lst.addLast(vh)) {
        return;
    } else {
        errMsg(KsString("Failed to create variable ", name));
    }
}

//////////////////////////////////////////////////////////////////////

KscPackageHandle
listToPackage(const VariableList &lst)
{
    KscPackageHandle hpkg(new KscPackage, KsOsNew);

    if(hpkg) {
        PltListIterator<KscVariableHandle> *it = lst.newIterator();
        if(it) {
            while(*it) {
                hpkg->add(**it);
                ++(*it);
            }
            delete it;
        } else {
            errMsg("Cannot create iterator");
        }
    } else {
        errMsg("Failed to allocate package");
    }

    return hpkg;
}

//////////////////////////////////////////////////////////////////////

void
generateAINVariables(KsString path,
                     VariableList &vars)
{
    addVariableToList(path + "NAME", vars);
    addVariableToList(path + "TYPE", vars);
    addVariableToList(path + "MA", vars);
    addVariableToList(path + "INHIB", vars);
    addVariableToList(path + "INHALM", vars);
    addVariableToList(path + "FTIM", vars);
    addVariableToList(path + "XREFIN", vars);
    addVariableToList(path + "KSCALE", vars);
    addVariableToList(path + "BSCALE", vars);
    addVariableToList(path + "BAT", vars);

    addVariableToList(path + "BAD", vars);
}
    
//////////////////////////////////////////////////////////////////////

void 
dumpCommObjectErrCode(const KscCommObject *pco)
{
    if(pco) {
        cout << "Error code (commobject/server): " 
             << pco->getLastResult()
             << " / "
             << pco->getServer()->getLastResult();
    }
}

//////////////////////////////////////////////////////////////////////

void
dumpValue(KsValueHandle hval)
{
    if(hval) {
        switch(hval->xdrTypeCode()) {
        case KS_VT_VOID :
            cout << "void";
            break;

        case KS_VT_INT : {
            KsIntValue *pintval =
                PLT_DYNAMIC_PCAST(KsIntValue, hval.getPtr());
            PLT_ASSERT(pintval);
            cout << "int " << ((long)(*pintval));
        }
        break;

        case KS_VT_UINT : {
            KsUIntValue *puintval =
                PLT_DYNAMIC_PCAST(KsUIntValue, hval.getPtr());
            PLT_ASSERT(puintval);
            cout << "u_int " << ((u_long)(*puintval));
        }
        break;

        case KS_VT_SINGLE : {
            KsSingleValue *psingleval =
                PLT_DYNAMIC_PCAST(KsSingleValue, hval.getPtr());
            PLT_ASSERT(psingleval);
            cout << "single " << ((float)(*psingleval));
        }
        break;

        case KS_VT_DOUBLE : {
            KsDoubleValue *pdblval =
                PLT_DYNAMIC_PCAST(KsDoubleValue, hval.getPtr());
            PLT_ASSERT(pdblval);
            cout << "double " << ((double)(*pdblval));
        }
        break;

        case KS_VT_STRING : {
            KsStringValue *pstrval =
                PLT_DYNAMIC_PCAST(KsStringValue, hval.getPtr());
            PLT_ASSERT(pstrval);
            cout << "string " << ((const char *)(*pstrval));
        }
        break;

        case KS_VT_TIME :
            cout << "time";
            break;

        case KS_VT_BYTE_VEC :
            cout << "byte vec";
            break;

        case KS_VT_INT_VEC :
            cout << "int vec";
            break;

        case KS_VT_UINT_VEC :
            cout << "u_int vec";
            break;

        case KS_VT_SINGLE_VEC :
            cout << "single vec";
            break;

        case KS_VT_DOUBLE_VEC :
            cout << "double vec";
            break;

        case KS_VT_STRING_VEC :
            cout << "string vec";
            break;

        case KS_VT_TIME_VEC :
            cout << "time vec";
            break;

        default :
            cout << "unknown";
        }
    } else {
        cout << "unbound value handle";
    }
}

//////////////////////////////////////////////////////////////////////

void 
dumpVariable(KscVariableHandle hvar) 
{
    if(hvar) {
        cout << "Full path of variable:" << hvar->getFullPath() << endl;
        dumpCommObjectErrCode(hvar.getPtr());
        cout << endl << "Value: ";
        dumpValue(hvar->getValue());
        cout << endl;
    } else {
        cout << "unbound variable handle" << endl;
    }
}

//////////////////////////////////////////////////////////////////////

void
dumpDomain(KscDomainHandle hdom)
{
    if(hdom) {
        cout << "Full path of domain: " << hdom->getFullPath() << endl;
        dumpCommObjectErrCode(hdom.getPtr());
        cout << endl;
    } else {
        cout << "unbound domain handle" << endl;
    }
}

//////////////////////////////////////////////////////////////////////

void
dumpPackage(KscPackageHandle hpkg, bool printVariables = false)
{
    if(hpkg) {
        cout << "Package containing "
             << hpkg->sizeVariables(false)
             << " Variables and "
             << hpkg->sizeSubpackages()
             << " Subpackages, summing up to "
             << hpkg->sizeVariables(true)
             << " Variables" << endl;
        if(printVariables) {
            KscPkgVariableIterator *it =
                hpkg->newVariableIterator(true);
            PLT_ASSERT(it);
            while(*it) {
                dumpVariable(**it);
                ++(*it);
            }
            delete it;
        }
    } else {
        cout << "unbound package handle" << endl;
    }
}

//////////////////////////////////////////////////////////////////////

void
readVariable(KscVariableHandle hvar)
{
    if(hvar) {
        if(hvar->getUpdate()) {
            cout << hvar->getFullPath() 
                 << " successfully read" << endl;
            dumpVariable(hvar);
        } else {
            cout << hvar->getFullPath()
                 << " failed to read, ";
            dumpCommObjectErrCode(hvar.getPtr());
            cout << endl;
        }
    } else {
        errMsg("unbound variable handle");
    }
}

//////////////////////////////////////////////////////////////////////

void
readList(VariableList &lst)
{
    VariableIterator *it = lst.newIterator();
    
    if(it) {
        while(*it) {
            readVariable(**it);
            ++(*it);
        }
        delete it;
    } else {
        errMsg("failed to create iterator");
    }
}

//////////////////////////////////////////////////////////////////////

void
insertVariables(KscPackageHandle hpkg,
                KscDomainHandle hdom,
                KsString path)
{
    KscChildIterator *it = hdom->newChildIterator(KS_OT_VARIABLE);
    if(it) {
        while(*it) {
            KsProjPropsHandle hpp = **it;
            KscVariableHandle hvar(
                new KscVariable(path + hpp->identifier),
                KsOsNew);
            if(!hpkg->add(hvar)) {
                errMsg("Unable to add " + hvar->getFullPath() + " to package");
            }
            ++(*it);
        }
        delete it;
    } else {
        errMsg("Unable to allocate iterator");
        dumpDomain(hdom);
    }

}

//////////////////////////////////////////////////////////////////////

void
insertDomains(KscPackageHandle hpkg,
              KscDomainHandle hdom,
              KsString path)
{
    KscChildIterator *it = hdom->newChildIterator(KS_OT_DOMAIN);
    if(it) {
        while(*it) {
            KsProjPropsHandle hpp = **it;
            KscPackageHandle hsubpkg = 
                buildDirectoryPackage(path + hpp->identifier);
            if(!hpkg->add(hsubpkg)) {
                errMsg("Unable to add new subpackage");
            }
            ++(*it);
        }
        delete it;
    } else {
        errMsg("Unable to allocate iterator");
        dumpDomain(hdom);
    }
}

//////////////////////////////////////////////////////////////////////

KscPackageHandle
buildDirectoryPackage(KsString path)
{
    KscDomainHandle thisDomain(
        new KscDomain(path),
        KsOsNew);
    KscPackageHandle hpkg(
        new KscPackage,
        KsOsNew);

    if(thisDomain && hpkg) {
        if(thisDomain->hasValidPath()) {
            path += "/";
            insertVariables(hpkg, thisDomain, path);
            insertDomains(hpkg, thisDomain, path);
        } else {
            errMsg("invalid path");
        }
    } else {
        errMsg("out of mem");
    }
    return hpkg;
}

//////////////////////////////////////////////////////////////////////

KscPackageHandle
buildRootPackage() 
{
    KscDomainHandle thisDomain(
        new KscDomain(hostAndServer + "/"),
        KsOsNew);
    KscPackageHandle hpkg(
        new KscPackage,
        KsOsNew);

    if(thisDomain && hpkg) {
        if(thisDomain->hasValidPath()) {
            KsString temp(hostAndServer, "/");
            insertVariables(hpkg, thisDomain, temp);
            insertDomains(hpkg, thisDomain, temp);
        } else {
            errMsg("invalid path");
        }
    } else {
        errMsg("out of mem");
    }
    return hpkg;
}

//////////////////////////////////////////////////////////////////////

KscPackageHandle
buildAllPackage()
{
    KscPackageHandle result = buildRootPackage();
    if(server == "tserver") {
#if 0
        KsString path(hostAndServer+"/restricted");
        KscDomainHandle hdom(
            new KscDomain(path),
            KsOsNew);
        hdom->setAvModule(&av_simple_read);
        path += "/";
        insertVariables(result, hdom, path);
#else
        KsString path(hostAndServer+"/restricted/writeme");
        KscVariableHandle hvar(new KscVariable(path), KsOsNew);
//        result->add(hvar);
        hvar->setAvModule(&av_simple_write);
        KscPackageHandle hsub(new KscPackage, KsOsNew);
        hsub->add(hvar);
        result->add(hsub);
//        hsub->setAvModule(&av_simple_write);
#endif
    }
    return result;
}

//////////////////////////////////////////////////////////////////////

void
readPackage(KscPackageHandle hpkg)
{
    bool ok = hpkg->getUpdate();

    if(ok) {
        cout << "Package successfully read" << endl;
    } else {
        cout << "Failed to get update of package" << endl;
    }

    dumpPackage(hpkg, true);
}
    
//////////////////////////////////////////////////////////////////////

int
main(int argc, char *argv[])
{
    if(argc != 3) {
        cout << "usage: tclient.exe host server" << endl;
        exit(-1);
    }

    host = argv[1];
    server = argv[2];
    hostAndServer = "//";
    hostAndServer += argv[1];
    hostAndServer += "/";
    hostAndServer += argv[2];

    KscClient::getClient()->setTimeouts(PltTime(30, 0), PltTime(0,0), 0);

    KscPackageHandle all = buildAllPackage();
    cout << "************************************************************" << endl;
    cout << "Going to read package without AV-Module" << endl;
    readPackage(all);

#if 0
    all->setAvModule(&av_simple_write);
    cout << "************************************************************" << endl;
    cout << "Going to read package with AVSimple-Module" << endl;
    readPackage(all);
#endif

#if 0
    // deletes package, creates a new one
    // just a test of the memory management
    //
    all = buildAllPackage();
    all->setAvModule(&av_simple_write);
    cout << "************************************************************" << endl;
    cout << "Going to read package with AVSimple-Module" << endl;
    readPackage(all);
#endif

    cout << "finished" << endl;
}






