/* -*-plt-c++-*- */
/* $Header: /home/david/cvs/acplt/ks/src/xdrudpcon.cpp,v 1.2 1998-06-30 11:29:08 harald Exp $ */
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
 * xdrudpcon.cpp -- Implements an UDP connection with XDR data as its
 *                  data representation for the mess we'll transport.
 *
 * Written by Harald Albrecht <harald@plt.rwth-aachen.de>
 */

#include "ks/xdrudpcon.h"
#include "ks/connectionmgr.h"
#include "plt/time.h"

#if PLT_SYSTEM_NT

#define EINTR       WSAEINTR
#define EWOULDBLOCK WSAEWOULDBLOCK

#else

#include <unistd.h>
#include <errno.h>

#endif


// ---------------------------------------------------------------------------
// Constructs an UDP connection with XDR for data representation. After con-
// structing such a connection, check the instance's state for CNX_STATE_DEAD,
// as this indicates a failure to initialize the object.
//
KssUDPXDRConnection::KssUDPXDRConnection(int fd, unsigned long timeout,
                                         ConnectionType type,
					 unsigned long buffsize)
    : KssXDRConnection(fd, false, timeout, type),
      _buffer_size(buffsize)
{
    if ( _buffer_size < 1024 ) {
    	_buffer_size = 1024;
    }
    if ( _buffer_size > 65500 ) {
    	_buffer_size = 65500;
    }
    //
    // Now set up a XDR (static) memory stream which will be used for
    // receiving and sending UDP datagrams. Then, put the udp socket
    // into the non-blocking mode (this makes the whole thing more
    // robust against arriving potential trouble between select() and
    // read() or write().
    //
    _buffer = new char[_buffer_size];
    if ( _buffer ) {
    	xdrmem_create(&_xdrs, (caddr_t) _buffer, _buffer_size, XDR_DECODE);
    	_cleanup_xdr_stream = true;
	
	if ( !makeNonblocking() ) {
	    _state = CNX_STATE_DEAD;
	} else {
	    if ( type == CNX_TYPE_CLIENT ) {
		PltTime jetzt = PltTime::now();
		_rpc_header._xid = getpid() ^ jetzt.tv_sec ^ jetzt.tv_usec;
	    }
	}
    } else {
    	_state = CNX_STATE_DEAD;
    }
} // KssUDPXDRConnection::KssUDPXDRConnection


// ---------------------------------------------------------------------------
// Ask an UDP XDR connection about its i/o mode: this way, a connection
// manager can find out in what fdsets it must put the connection.
//
KssConnection::ConnectionIoMode KssUDPXDRConnection::getIoMode() const
{
    switch ( _state ) {
    case CNX_STATE_DEAD:      return CNX_IO_DEAD;
    case CNX_STATE_IDLE:      return CNX_IO_READABLE;
    case CNX_STATE_WAITING:   return (ConnectionIoMode)(CNX_IO_READABLE |
					                    CNX_IO_NEED_TIMEOUT);
    case CNX_STATE_PASSIVE:   return CNX_IO_DORMANT;
    case CNX_STATE_READY:     return (ConnectionIoMode)(CNX_IO_DORMANT | CNX_IO_ATTENTION);
    case CNX_STATE_SENDING:   return CNX_IO_WRITEABLE;
    default:                  return CNX_IO_READABLE;
    }
} // KssUDPXDRConnection::getIoMode


// ---------------------------------------------------------------------------
// Try to receive an UDP datagram and make sure it has at least some bytes in
// it. If this was successful, then this connection will go into the state
// CNX_STATE_READY and signal for CNX_STATE_ATTENTION. Otherwise you will
// probably see a CNX_STATE_HAD_ERROR. If there was some trouble and the
// socket signalled a possible blocking condition, then the connection will
// remain in the CNX_STATE_IDLE state.
//
KssConnection::ConnectionIoMode KssUDPXDRConnection::receive()
{
    int received;
#if PLT_USE_XTI
    int  flags;
    bool flush = false;
#endif

    //
    // First, we don't want to deal with dead connections -- it just makes
    // no fun. And we can work only on idle connections...
    //
    if ( _state == CNX_STATE_DEAD ) {
    	return CNX_IO_DEAD;
    }
    if ( (_state != CNX_STATE_IDLE) && (_state != CNX_STATE_WAITING) ) {
    	return reset(false);
    }
    //
    // reset the read pointer of the underlaying XDR stream back to the
    // start of the buffer.
    //
    _xdrs.x_op = XDR_DECODE;
    XDR_SETPOS(&_xdrs, 0);
    
    for ( ;; ) {
	if ( _cnx_type == CNX_TYPE_SERVER ) {
#if !PLT_USE_XTI
    	    _client_address_len = sizeof(_client_address);
	    received = recvfrom(_fd, _buffer, _buffer_size,
		        	0, // no special flags
	                	(struct sockaddr *) &_client_address,
#if PLT_SYSTEM_OPENVMS
				(unsigned int *)
#endif
		        	&_client_address_len);
#else
    	    struct t_unitdata udta;
    	    _client_address_len = sizeof(_client_address);
	    udta.addr.maxlen  = _client_address_len;
	    udta.addr.buf     = (char *) &_client_address;
	    udta.opt.maxlen   = 0;
	    udta.udata.maxlen = _buffer_size;
	    udta.udata.buf    = (char *) _buffer;
	    received = t_rcvudata(_fd, &udta, &flags);
	    if ( received == 0 ) {
	    	//
		// Make this piece of code behave like the socket functions:
		// return number of bytes read or -1 as an error indication.
		//
	    	received = udta.udata.len;
	    }
#endif
	} else {
	    //
	    // In case of a client-side connection, don�t overwrite the
	    // _client_address as it contains the address of the server.
	    // To be compatible with Sun�s ONC/RPC we�re just ignoring
	    // the sender�s IP address -- well, I don�t like this idea
	    // very much...
	    //
#if !PLT_USE_XTI
	    received = recvfrom(_fd, _buffer, _buffer_size,
		        	0, // no special flags
		    	        0, 0);
#else
    	    struct t_unitdata udta;
	    udta.addr.maxlen  = 0; // Not interested in the sender's
	    udta.opt.maxlen   = 0; // address...
	    udta.udata.maxlen = _buffer_size;
	    udta.udata.len    = 0;
	    udta.udata.buf    = (char *) _buffer;
	    received = t_rcvudata(_fd, &udta, &flags);
	    if ( received == 0 ) {
	    	// Same as above...
	    	received = udta.udata.len;
	    }
#endif
	}
	if ( received < 0 ) {
	    //
	    // An error occured, so find out what has happened. In case of
	    // an interrupted call, let's try again... For all other errors
	    // ignore them silently and remain in the idle state.
	    //
#if PLT_SYSTEM_NT
    	    int errno = WSAGetLastError();
#elif PLT_USE_XTI
    	    //
	    // Make sure that a pending event on the XTI endpoint is read.
	    //
    	    switch ( t_errno ) {
	    case TLOOK: {
	    	int events = t_look(_fd);
		if ( events & T_UDERR ) {
		    t_rcvuderr(_fd, 0); // clear error for previous datagram
		}
	    	if ( events & T_DATA ) {
		    continue; // next try...
		}
		break; // fall through and return an io error indication
		}
	    case TNODATA:
	    	return getIoMode();
	    case TSYSERR:
	    	break; // fall through and let the code below handle it
	    default:
		return getIoMode();
	    }
#endif
	    switch ( errno ) {
	    case EINTR:
		continue;
	    case EWOULDBLOCK:
	    	return getIoMode();
	    }
#if PLT_USE_XTI
	} else if ( flags & T_MORE ) {
	    //
	    // When flushing and there is more data to read, then just
	    // take the next loop slurping in the data...
	    //
	    flush = true;
	    continue;
    	} else if ( flush ) {
	    //
	    // Silently ignore this oversized packet and just remember one
	    // more io error by falling out of the loop.
	    //
	    break;
#endif
	} else if ( received >= 4 ) {
	    //
	    // There seems to be enough data to process at least the first
	    // entry of the header, so report a ready request. Remember that
	    // the stream already has been reset before entering the for loop.
	    // In case of a client-side connection, make sure that this is a
	    // valid answer.
	    //
	    if ( _cnx_type == CNX_TYPE_CLIENT ) {
		long *ppp = (long *) _buffer;
		if ( ((u_long) IXDR_GET_LONG(ppp)) != 
		         _rpc_header._xid ) {
		    //
		    // try once again next time as this reply had the wrong
		    // transaction id.
		    //
		    return getIoMode();
		}
		if ( !_rpc_header.xdrDecode(&_xdrs) ) {
	    	    return (ConnectionIoMode)(getIoMode() | CNX_IO_HAD_ERROR);
		}
	    } else {
		if ( !_rpc_header.xdrDecode(&_xdrs) ) {
		    _rpc_header.setDecodeError();
		    _state = CNX_STATE_SENDING;
	    	    return (ConnectionIoMode)(getIoMode() | CNX_IO_HAD_ERROR);
		}
	    }
    	    _state = CNX_STATE_READY;
	    return getIoMode();
	}
	break; // some error...
    }
    return (ConnectionIoMode)(getIoMode() | CNX_IO_HAD_RX_ERROR);
} // KssUDPXDRConnection::receive


// ---------------------------------------------------------------------------
// Try to get rid of an UDP datagram by sending it to someone out there on the
// Internet. If this was successful, then this connection will go into the
// CNX_STATE_IDLE. Otherwise you will probably see a CNX_STATE_HAD_ERROR. If
// there was some trouble and the socket signalled a possible blocking
// condition, then the connection will remain in the CNX_STATE_SENDING state.
//
KssConnection::ConnectionIoMode KssUDPXDRConnection::send()
{
    int sent, tosend;
    
    //
    // First, we don't want to deal with dead connections -- it just makes
    // no fun.
    //
    if ( _state == CNX_STATE_DEAD ) {
    	return CNX_IO_DEAD;
    }
    if ( _state != CNX_STATE_SENDING ) {
    	return reset(false);
    }
    
    for ( ;; ) {
	tosend = (int) XDR_GETPOS(&_xdrs);
#if !PLT_USE_XTI
	sent = sendto(_fd, _buffer, tosend,
		      0,
		      (struct sockaddr *) &_client_address,
		      _client_address_len);
#else
    	    struct t_unitdata udta;
	    udta.addr.len     = _client_address_len;
	    udta.addr.buf     = (char *) &_client_address;
	    udta.opt.maxlen   = 0;
	    udta.udata.len    = tosend;
	    udta.udata.buf    = (char *) _buffer;
	    sent = t_sndudata(_fd, &udta);
#endif
	if ( sent < 0 ) {
#if PLT_SYSTEM_NT
    	    int errno = WSAGetLastError();
#elif PLT_USE_XTI
    	    //
	    // Make sure that a pending event on the XTI endpoint is read.
	    //
    	    switch ( t_errno ) {
	    case TLOOK: {
	    	int events = t_look(_fd);
		if ( events & T_UDERR ) {
		    t_rcvuderr(_fd, 0); // clear error for previous datagram
		    continue;
		}
		break; // fall through and return an io error indication
		}
	    case TFLOW:
	    	continue;
	    case TSYSERR:
	    	break; // fall through and let the code below handle it
	    default:
		return getIoMode();
	    }
#endif
	    switch ( errno ) {
	    case EINTR:
		continue;
	    case EWOULDBLOCK:
	    	return getIoMode();
	    default:
	    	return (ConnectionIoMode)(reset(false) | CNX_IO_HAD_TX_ERROR);
	    }
	}
    	//
    	// Go back into the idle mode and don't care any more about this
    	// datagram.
    	//
    	_state = _cnx_type == CNX_TYPE_SERVER ? 
		                  CNX_STATE_IDLE : CNX_STATE_WAITING;
	break;
    }
    return getIoMode();
} // KssUDPXDRConnection::send


// ---------------------------------------------------------------------------
// Reset this connection to a well-known state, which is in our case the
// CNX_STATE_IDLE state where we'll wait for arriving datagrams. Also, don't
// be afraid of timeouts...
//
KssConnection::ConnectionIoMode KssUDPXDRConnection::reset(bool)
{
    if ( _state == CNX_STATE_DEAD ) {
    	//
	// Can't reset a dead connection.
	//
    	return CNX_IO_DEAD;
    } else {
    	_state = _cnx_type == CNX_TYPE_SERVER ? 
		                  CNX_STATE_IDLE : CNX_STATE_PASSIVE;
    	return getIoMode();
    }
} // KssUDPXDRConnection::reset


// ---------------------------------------------------------------------------
//
void KssUDPXDRConnection::sendPingReply()
{
    if ( _state != CNX_STATE_DEAD ) {
	_xdrs.x_op = XDR_ENCODE;
	XDR_SETPOS(&_xdrs, 0);
	_rpc_header.acceptCall();
	if ( _rpc_header.xdrEncode(&_xdrs) ) {
    	    _state = CNX_STATE_SENDING;
	} else {
    	    reset(false);
	}
    }    
} // KssUDPXDRConnection::sendPingReply


// ---------------------------------------------------------------------------
//
void KssUDPXDRConnection::sendErrorReply(KsAvTicket &avt, KS_RESULT error)
{
    if ( _state != CNX_STATE_DEAD ) {
    	u_long e = error == KS_ERR_OK ? KS_ERR_GENERIC : error;
	_xdrs.x_op = XDR_ENCODE;
	XDR_SETPOS(&_xdrs, 0);
	_rpc_header.acceptCall();
	if ( _rpc_header.xdrEncode(&_xdrs) &&
	     avt.xdrEncode(&_xdrs) &&
	     xdr_u_long(&_xdrs, &e) &&
	     avt.xdrEncodeTrailer(&_xdrs) ) {
    	    _state = CNX_STATE_SENDING;
	} else {
    	    reset(false);
	}
    }    
} // KssUDPXDRConnection::sendErrorReply


// ---------------------------------------------------------------------------
//
void KssUDPXDRConnection::sendReply(KsAvTicket &avt, KsResult &result)
{
    if ( _state != CNX_STATE_DEAD ) {
	_xdrs.x_op = XDR_ENCODE;
	XDR_SETPOS(&_xdrs, 0);
	_rpc_header.acceptCall();
	if ( _rpc_header.xdrEncode(&_xdrs) &&
	     avt.xdrEncode(&_xdrs) &&
	     result.xdrEncode(&_xdrs) &&
	     avt.xdrEncodeTrailer(&_xdrs) ) {
    	    _state = CNX_STATE_SENDING;
	} else {
    	    reset(false);
	}
    }    
} // KssUDPXDRConnection::sendReply


// ---------------------------------------------------------------------------
//
void KssUDPXDRConnection::personaNonGrata()
{
    reset(false);
} // KssUDPXDRConnection::personaNonGrata


// ---------------------------------------------------------------------------
//
bool KssUDPXDRConnection::beginRequest()
{
    if ( _state == CNX_STATE_DEAD ) {
	return false;
    }
    //
    // reset the read pointer of the underlaying XDR stream back to the
    // start of the buffer and fill in request header.
    //
    _xdrs.x_op = XDR_DECODE;
    XDR_SETPOS(&_xdrs, 0);
    _state = CNX_STATE_PASSIVE;
    ++_rpc_header._xid;
    return _rpc_header.xdrEncode(&_xdrs);
} // KssUDPXDRConnection::beginRequest


// ---------------------------------------------------------------------------
//
void KssUDPXDRConnection::sendRequest()
{
    if ( _state != CNX_STATE_DEAD ) {
    	_state = CNX_STATE_SENDING;
    	_manager->reactivateConnection(*this);
    }
} // KssUDPXDRConnection::sendRequest


/* End of xdrudpcon.cpp */