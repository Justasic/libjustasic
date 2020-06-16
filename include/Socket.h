// Copyright (c) 2014-2020, Justin Crawford <Justin@stacksmash.net>
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.

// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include "Flux.h"
#include "Utilities.h"
#include <arpa/inet.h>

typedef union
{
	struct sockaddr_in  ipv4;
	struct sockaddr_in6 ipv6;
	struct sockaddr		sa;
} sockaddr_t;

// clang-format off
typedef enum
{
	// Misc socket statuses.
	SS_DEAD       = 1 << 1,
	SS_WRITABLE   = 1 << 2,
	// For connecting sockets
	SS_CONNECTING = 1 << 3,
	SS_CONNECTED  = 1 << 4,
	// For Binding sockets.
	SS_ACCEPTING  = 1 << 5,
	SS_ACCEPTED   = 1 << 6,
	// Multiplexer statuses
	MX_WRITABLE   = 1 << 7,
	MX_READABLE   = 1 << 8
} socketstatus_t;
// clang-format on

class Socket : public Flags<socketstatus_t>
{
  protected:
	int		   sock_fd;
	sockaddr_t sa;

  public:
	// Delete the default constructor because it causes all kinds of fucking issues.
	Socket() = delete;
	Socket(int sock, int type, int protocol = SOCK_STREAM);
	virtual ~Socket();

	// Socket Flags
	void SetNonBlocking(bool status);

	// I/O functions, these are overwritten in the sub-class sockets.
	virtual size_t Write(const void *data, size_t len);
	virtual size_t Read(void *data, size_t len);

	// Getters/Setters
	inline int GetFD() { return this->sock_fd; }

	// Interaction with the SocketMultiplexer
	virtual bool MultiplexEvent();
	virtual void MultiplexError();
	virtual bool MultiplexRead();
	virtual bool MultiplexWrite();

	static Flux::string GetAddress(sockaddr_t saddr);
	static short		GetPort(sockaddr_t s);
	static sockaddr_t   GetSockAddr(int type, const Flux::string &addr, int port);
};

class ConnectionSocket : public virtual Socket
{
	Flux::string address;
	short		 port;

  public:
	ConnectionSocket(bool ipv6);
	virtual ~ConnectionSocket();

	bool MultiplexEvent();
	void MultiplexError();

	void Connect(const Flux::string &address, short port);

	virtual void OnConnect() = 0;
	virtual void OnError(const Flux::string &str);
};

class ClientSocket;

class ListeningSocket : public virtual Socket
{
  protected:
	Flux::string address;
	short		 port;
	bool		 ipv6;

  public:
	ListeningSocket(const Flux::string &bindaddr, short port, bool ipv6);
	virtual ~ListeningSocket();
	bool MultiplexRead();

	virtual ClientSocket *OnAccept(int fd, const sockaddr_t &addr) = 0;
};

class ClientSocket : public virtual Socket
{
  public:
	ListeningSocket *ls;
	ClientSocket(ListeningSocket *ls, int sock_fd, const sockaddr_t &addr, bool ipv6);
	bool		 MultiplexEvent();
	void		 MultiplexError();
	virtual void OnAccept();
	virtual void OnError(const Flux::string &str);
};
