/* -*-plt-c++-*- */
/* $Header: /home/david/cvs/acplt/ks/include/ks/connection.h,v 1.2 1998-06-30 11:29:06 harald Exp $ */
/*
 * Copyright (c) 1998
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

/*
 * connection.h -- Abstract base class and basic classes for Internet-based
 *                 connections via UDP/IP and TCP/IP.
 *
 * Written by Harald Albrecht <harald@plt.rwth-aachen.de>
 */

#ifndef KS_CONNECTION_H_INCLUDED
#define KS_CONNECTION_H_INCLUDED


#include "ks/xdr.h"
#include "ks/rpcproto.h"
#include "ks/avticket.h"
#include "ks/result.h"


// ---------------------------------------------------------------------------
// Abstract base class for a connection: a connection has its own state (it's
// a finite automata), a socket to suck from, ... and other stuff I forget
// what it's good for.
//
class KssConnection {
public:
    //
    //
    //
    enum ConnectionType {
    	CNX_TYPE_SERVER,    	// it�s on the server side
	CNX_TYPE_CLIENT     	// it�s on the client side
    };
	
    KssConnection(int fd, bool autoDestroyable, unsigned long timeout,
	          ConnectionType type);
    virtual ~KssConnection() { }
    //
    // Shuts down the real connection encapsulated by this object. This is
    // where the old "You can't do late binding in your desctructor" comes
    // in again. So before you delete a connection, shut it down first.
    //
    virtual void shutdown() = 0;
        
    //
    // The states a connection can be in (a connection is regarded as a
    // finite state automata).
    // For servers: IDLE->RECEIVING->READY->SENDING->IDLE
    // For clients: (READY)->SENDING->WAITING->RECEIVING->READY
    //
    enum ConnectionState {
    	CNX_STATE_IDLE,   	// connection is waiting for next incom. request
	CNX_STATE_WAITING,  	// c. is waiting for reply (w/ timeout)
	CNX_STATE_PASSIVE,  	// c. is not doing any io yet
	CNX_STATE_READY,  	// c. is waiting to be served (has request/reply)
	CNX_STATE_RECEIVING,	// c. is receiving (maybe a request)
	CNX_STATE_SENDING,	// c. is sending (maybe a reply)
	CNX_STATE_DEAD	    	// c. is dead and should be destroyed
    }; // enum ConnectionState
    //
    // The I/O mode is a generalization of the connection state with
    // respect of information the connection manager is interested in.
    //
    enum ConnectionIoMode {
        CNX_IO_DORMANT = 0x0000,      // c. waits to be served
    	CNX_IO_READABLE = 0x0001,     // c. can receive input
	CNX_IO_WRITEABLE = 0x0002,    // c. can send output
	CNX_IO_ATTENTION = 0x0004,    // c. need to be served
	CNX_IO_DEAD = 0x0008, 	      // c. is dead
	CNX_IO_NEED_TIMEOUT = 0x8000, // c. needs a timeout control
	CNX_IO_HAD_ERROR = 0x4000,    // c. had an i/o error
	CNX_IO_HAD_TX_ERROR = 0x2000, // c. had i/o error during sending
	CNX_IO_HAD_RX_ERROR = 0x1000  // c. had i/o error during receiving
    }; // enum ConnectionIoMode

        
    inline bool isAutoDestroyable() const { return _auto_destroyable; }
    inline int getFd() const { return _fd; }
    inline long getTimeout() const { return _timeout; }
    inline ConnectionType getCnxType() const { return _cnx_type; }
    u_short getPort() const;
    
    ConnectionState getState() const { return _state; }
    virtual ConnectionIoMode getIoMode() const = 0;
        
    inline struct sockaddr_in *getPeerAddress(int &addrLen)
    	{ addrLen = _client_address_len; return &_client_address; }
    bool setPeerAddr(struct sockaddr_in *addr, int addrLen);

protected:
    //
    // These methods are called by the connection manager whenever there
    // is i/o to carry out.
    //
    friend class KssConnectionManager;
    virtual ConnectionIoMode receive() = 0;
    virtual ConnectionIoMode send() = 0;
    //
    // Bring connection back into its default state.
    //
    virtual ConnectionIoMode reset(bool hadTimeout) = 0;
    
    void thisIsMyConnectionManager(KssConnectionManager *mgr)
    	{ _manager = mgr; }

    ConnectionType        _cnx_type;    
    unsigned long         _timeout;
    bool                  _auto_destroyable;
    int                   _fd;
    ConnectionState       _state;
    struct sockaddr_in    _client_address;
    int                   _client_address_len;
    KssConnectionManager *_manager;
}; // class KssConnection


// ---------------------------------------------------------------------------
// Still abstract class for XDR connections. This class only introduces the
// XDR stream encapsulated by instances of this class, the proper shutdown
// of such XDR streams, and how to get your hands on them.
//
class KssXDRConnection : public KssConnection {
public:
    KssXDRConnection(int fd, bool autoDestroyable, unsigned long timeout,
	             ConnectionType type);
    
    virtual void shutdown();
    
    XDR *getXdr() { return &_xdrs; }
    
    //
    // And now for the missing parts of the KssTransport pseudo-interface
    // which is used by the KsServerBase class for processing RPC calls.
    //
    XDR *getDeserializingXdr() { return &_xdrs; }
    KS_RESULT finishRequestDeserialization(KsAvTicket &avt, bool ok);
    u_long getProgramId() const { return _rpc_header._prog_number; }
    u_long getServiceId() const { return _rpc_header._procedure; }
    u_long getVersion() const { return _rpc_header._prog_version; }
    virtual void sendPingReply() = 0;
    virtual void sendErrorReply(KsAvTicket &avt, KS_RESULT error) = 0;
    virtual void sendReply(KsAvTicket &avt, KsResult &result) = 0;
    virtual void personaNonGrata() = 0;

    virtual bool beginRequest() = 0;
    virtual void sendRequest() = 0;
    
protected:
    virtual void freeStreamMemory() { }
    
    bool makeNonblocking();
    bool enableKeepAlive();

    XDR         _xdrs;
    bool        _cleanup_xdr_stream;
    KsRpcHeader _rpc_header;
}; // class KssXDRConnection


#endif

/* End of connection.h */