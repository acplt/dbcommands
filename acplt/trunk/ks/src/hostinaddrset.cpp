/* -*-plt-c++-*- */
/* $Header: /home/david/cvs/acplt/ks/src/hostinaddrset.cpp,v 1.1 1997-10-28 10:39:42 harald Exp $ */
/*
 * Copyright (c) 1996, 1997
 * Chair of Process Control Engineering,
 * Aachen University of Technology.
 * All rights reserved.
 *
 * NOTE:
 * Some parts are (c) Sun Microsystems, see below for their copyright
 * and conditions.
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
/*
 * Adapted from:
 * @(#) from_local.c 1.3 96/05/31 15:52:57 (Linux networking base)
 */
/*
 * Extended for the "NT operating system" by Harald Albrecht of the
 * Chair of Process Control Engineering. The idea of how to get all
 * host IP addresses on NT was independently developed and then proofed
 * by the NT port of nntp.
 *
 * Also, some minor changes made in the old functions to take advantage
 * of the PltLog objects and to plug them into the KsHostInAddrSet
 * object class.
 */
/*
 * Check if an address belongs to the local system. Adapted from:
 * 
 * @(#)pmap_svc.c 1.32 91/03/11 Copyright 1984,1990 Sun Microsystems, Inc.
 * @(#)get_myaddress.c  2.1 88/07/29 4.0 RPCSRC.
 */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user or with the express written consent of
 * Sun Microsystems, Inc.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

// --------------------------------------------------------------------------
// First, some PLT stuff which helps us to survive in the various
// os environments.
//
#include "ks/hostinaddrset.h"
#include "plt/log.h"

// --------------------------------------------------------------------------
// First, the header(s) section
//

#if PLT_SYSTEM_NT
//
// The Windooze stuff...
//
#include <windows.h>
#include <winsock.h>
#include "plt/string.h"

#else
#if PLT_SYSTEM_OPENVMS
//
// The OpenVMS stuff...
// This looks seriously brain-damaged, but for OpenVMS < 7.1 its
// the only way to get our hands on the ioctl() stuff. Shamelessly
// stolen from the OpenVMS port of ntpd/xntpd.
//
#include <ucx$inetdef.h>
#define ifreq				IFREQDEF
#define ifr_addr			IFR$R_DUMMY.IFR$T_ADDR
#define ifr_flags			IFR$R_DUMMY.IFR$R_DUMMY_1_OVRL.IFR$W_FLAGS
#define IFF_UP				IFR$M_IFF_UP

struct ifconf {
    int ifc_len;
    union {
        caddr_t ifcu_buf;
        struct ifreq *ifcu_req;
    } ifc_ifcu;
}; // struct ifconf
#define ifc_buf ifc_ifcu.ifcu_buf
#define ifc_req ifc_ifcu.ifcu_req

extern int ioctl(int d, int request, char *argp);

#else
//
// Now, finally for the Un*x stuff...
//
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#if PLT_SYSTEM_SOLARIS
#include <sys/sockio.h>
#endif

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>

#endif
#endif

// --------------------------------------------------------------------------
// Now for the real code part...
//

#if PLT_SYSTEM_NT

/*
 * This is the find_local() variant for Windooze NT. It is really
 * brain-damaged, but -- honestly -- what else do you expect from Bill's
 * operating systems after all? Interestingly enough, NT doesn't get along
 * very well with the concept of network interfaces which can be brought up
 * and down during normal operation without rebooting and other lousy
 * problems. If Bill just have had a look at Linux, he would have thrown
 * NT out of his "Windows"...
 * ...like Marianne 013 said: "What a lousy system!"
 */
bool KsHostInAddrSet::findLocalAddresses()
{
    //
    // First, we must find out which devices (aka network cards) are
    // currently bound to the TCP/IP protocol. This is a really strange
    // way, because m$ has definetely NO CONCEPT about really how a
    // real network OS should be structured. If Billy Boy thinks that
    // the registry needs a change, everything will break, as there is
    // no official way to determine the IP addresses, etc. This is just
    // brain-damaged, thanks Bill!
    //
    HKEY     hLinkages;
    DWORD    RegValueType, RegValueLength;
    char    *TcpIpBindings;
    in_addr  inIp, mask;

    if ( RegOpenKey(HKEY_LOCAL_MACHINE,
                    "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Linkage", &hLinkages) 
         != ERROR_SUCCESS ) {
        PltLog::Error("KsHostInAddrSet::findLocalAddresses: Can't open TCP/IP linkage registry key");
        return false;
    }
    //
    // Next, find out how long the value of the "Bind" entry will be and
    // allocate enough memory to hold the value. Then retrieve the value
    // from the registry. Yeah, once again, programming NT is "easy as a
    // pie"... well, HAL 9000 proves this.
    //
    if ( RegQueryValueEx(hLinkages, "Bind",
                         0, &RegValueType,
                         0, &RegValueLength) != ERROR_SUCCESS ) {
        RegCloseKey(hLinkages);
        PltLog::Error("KsHostInAddrSet::findLocalAddresses: Can't get TCP/IP linkage binding value from the registry");
        return false;
    }
    if ( RegValueType != REG_MULTI_SZ ) {
        RegCloseKey(hLinkages);
        PltLog::Alert("KsHostInAddrSet::findLocalAddresses: The TCP/IP linkage binding value is not of type REG_MULTI_SZ. Looks like your registry was corrupted.");
        return false;
    }
    TcpIpBindings = new char[RegValueLength];
    if ( TcpIpBindings == 0 ) {
        RegCloseKey(hLinkages);
        PltLog::Error("KsHostInAddrSet::findLocalAddresses: Out of memory to get TCP/IP linkage binding value");
        return false;
    }
    if ( RegQueryValueEx(hLinkages, "Bind",
                         0, 0,
                         (LPBYTE) TcpIpBindings, &RegValueLength) != ERROR_SUCCESS ) {
        RegCloseKey(hLinkages);
        delete [] TcpIpBindings;
        PltLog::Error("KsHostInAddrSet::findLocalAddresses: Can't get TCP/IP linkage binding value from the registry");
        return false;
    }
    RegCloseKey(hLinkages);
    //
    // Now that we've got the bindings, we must scan them to find out
    // what services are bound to TCP/IP...
    //
    char  *device = TcpIpBindings;
    char  *service;
    int    serviceLength;
    char  *IpAddresses = 0;
    DWORD  IpAddressesLength, IpAddressesAllocated = 0;
    HKEY   hService;

    //
    // First, add the omnipresent local IP address. Unfortunately,
    // this is not listed in the service bindings by default.
    //
    inIp.s_addr = htonl(0x7F000001l);
    mask.s_addr = htonl(0xFFFFFFFFl);
    if ( !addItem(inIp, mask, true) ) {
        PltLog::Error("KsHostInAddrSet::findLocalAddresses: Cannot add local address 127.0.0.1");
    }

    for ( ; *device ; device += serviceLength + 1 ) {
        serviceLength = strlen(device);
        service       = strrchr(device, '\\');
        if ( service ) {
            //
            // Skip NDISWANxxx services as these are temporary interfaces
            // like RAS and we would not like to fight with all the
            // problems associated with such temporary things.
            //
            if ( (serviceLength >= 7) &&
                 (strncmpi(service, "NDISWAN", 7) == 0) ) {
                continue;
            }
            //
            // Okay, this seems to be a permanent service... We're silently
            // ignoring a failed access because we then can assume that it
            // is not a network card...
            //
            PltString TcpIp("SYSTEM\\CurrentControlSet\\Services", service);
            TcpIp += "\\Parameters\\Tcpip";
            if ( RegOpenKey(HKEY_LOCAL_MACHINE, TcpIp, &hService)
                 != ERROR_SUCCESS ) {
                continue;
            }
            //
            // First, get the list of IP addresses bound to this interface
            // (aka service, or whatever, who cares...). We do use here
            // dynamically allocated memory to hold the registry information.
            // This block of memory grows, if the current value is bigger
            // than the last. This way we can cope with (almost) arbitrary
            // sizes...
            //
            if ( RegQueryValueEx(hService, "IpAddress",
                                 0, &RegValueType,
                                 0, &IpAddressesLength) != ERROR_SUCCESS ) {
                RegCloseKey(hService);
                PltString errorMsg = "KsHostInAddrSet::findLocalAddresses: Can't get IP address for service: ";
                errorMsg += service;
                PltLog::Error(errorMsg);
                continue;
            }
            if ( RegValueType != REG_MULTI_SZ ) {
                RegCloseKey(hService);
                PltLog::Alert("KsHostInAddrSet::findLocalAddresses: The IP address' value is not of type REG_MULTI_SZ. Looks like your registry was corrupted.");
                continue;
            }
            if ( IpAddressesLength > IpAddressesAllocated ) {
                if ( IpAddresses ) {
                    delete [] IpAddresses;
                }
                IpAddressesAllocated = IpAddressesLength;
                IpAddresses = new char[IpAddressesAllocated];
                if ( IpAddresses == 0 ) {
                    break;
                }
            }
            if ( RegQueryValueEx(hService, "IpAddress",
                                 0, 0,
                                 (LPBYTE) IpAddresses, &IpAddressesLength)
                 != ERROR_SUCCESS ) {
                RegCloseKey(hService);
                PltString errorMsg = "KsHostInAddrSet::findLocalAddresses: Can't get IP address for service: ";
                errorMsg += service;
                PltLog::Error(errorMsg);
                continue;
            }
            RegCloseKey(hService);
            //
            // Now we can parse the IP addresses and store them into our
            // list of local addresses.
            //
            char          *pIp;
            int            IpLength;
            unsigned long  ip;

            for ( pIp = IpAddresses ; *pIp ; pIp += IpLength + 1 ) {
                IpLength = strlen(pIp);
                ip = inet_addr(pIp);
                if ( ip == INADDR_NONE ) {
                    continue;
                }
                inIp.s_addr = ip;
                if ( !addItem(inIp, mask, true) ) {
                    break;
                }
            }
        }
    }

    if ( IpAddresses ) {
        delete [] IpAddresses;
    }
    delete [] TcpIpBindings;
    return true;
} // KsHostInAddrSet::findLocalAddresses

#else /* Un*x & OpenVMS implementation */

/*
 * find_local - find all IP addresses for this host the Un*x way. Compared
 *              to what we need to do on "NT", this is considerably more
 *              straight and secure against new os releases.
 */
bool KsHostInAddrSet::findLocalAddresses()
{
    struct ifconf ifc;
    struct ifreq  ifreq;
    struct ifreq *ifr;
    struct ifreq *the_end;
    int           sock;
    char          buf[BUFSIZ];
    in_addr       mask;

    mask.s_addr = htonl(0xFFFFFFFFl);
    /*
     * Get list of network interfaces. We use a huge buffer to allow for the
     * presence of non-IP interfaces.
     */
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        PltLog::Error("KsHostInAddrSet::findLocalAddresses: can't allocate socket");
        return false;
    }
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, (char *) &ifc) < 0) {
        PltLog::Error("KsHostInAddrSet::findLocalAddresses: SIOCGIFCONF: can't get interface configurations");
        close(sock);
        return false;
    }

    /* Get IP address of each active IP network interface. */
    the_end = (struct ifreq *) (ifc.ifc_buf + ifc.ifc_len);
    for (ifr = ifc.ifc_req; ifr < the_end; ifr++) {
#if PLT_SYSTEM_OPENVMS
        if (((struct sockaddr *)&(ifr->ifr_addr))->sa_family == AF_INET) {	/* IP net interface */
#else
        if (ifr->ifr_addr.sa_family == AF_INET) {	/* IP net interface */
#endif
            ifreq = *ifr;
            if (ioctl(sock, SIOCGIFFLAGS, (char *) &ifreq) < 0) {
                PltLog::Error("KsHostInAddrSet::findLocalAddresses: SIOCGIFFLAGS: can't get interface configurations");
            } else if (ifreq.ifr_flags & IFF_UP) {	/* active interface */
                if (ioctl(sock, SIOCGIFADDR, (char *) &ifreq) < 0) {
                    PltLog::Error("KsHostInAddrSet::findLocalAddresses: SIOCGIFADDR: can't get interface address");
                } else {
                    if ( !addItem(((struct sockaddr_in *)
                                  & ifreq.ifr_addr)->sin_addr, mask, true) ) {
                        break;
                    }
                }
            }
        }
        /* Support for variable-length addresses. */
#ifdef HAS_SA_LEN
        ifr = (struct ifreq *) ((caddr_t) ifr
                                + ifr->ifr_addr.sa_len - sizeof(struct sockaddr));
#endif
    }
    close(sock);
    return true;
} // KsHostInAddrSet::findLocalAddresses

#endif /* Un*x & OpenVMS implementation */


KsHostInAddrSet::KsHostInAddrSet()
    : KsInAddrSet_base()
{
    findLocalAddresses();
} // KsHostInAddrSet::KsHostInAddrSet

bool KsHostInAddrSet::update()
{
    //
    // Get rid of old information...
    //
    removeAll();
    return findLocalAddresses();
} // KsHostInAddrSet::update

// End of hostinaddrset.cpp