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

#include "Socket.h"
#include "Exceptions.h"
#include "Log.h"
#include "SocketMultiplexer.h"
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

Flux::string Socket::GetAddress(sockaddr_t saddr)
{
	static char str[INET6_ADDRSTRLEN + 1];
	memset(str, 0, sizeof(str));
	switch (saddr.sa.sa_family)
	{
		case AF_INET:
			return inet_ntop(AF_INET, &saddr.ipv4.sin_addr, str, INET6_ADDRSTRLEN);
		case AF_INET6:
			return inet_ntop(AF_INET6, &saddr.ipv6.sin6_addr, str, INET6_ADDRSTRLEN);
		default:
			return "";
	}

	return "";
}

short Socket::GetPort(sockaddr_t s)
{
	switch (s.sa.sa_family)
	{
		case AF_INET:
			return ntohs(s.ipv4.sin_port);
		case AF_INET6:
			return ntohs(s.ipv6.sin6_port);
		default:
			return -1;
	}
}

sockaddr_t Socket::GetSockAddr(int type, const Flux::string &address, int port)
{
	sockaddr_t ret;
	memset(&ret, 0, sizeof(sockaddr_t));

	switch (type)
	{
		case AF_INET:
		{
			int i = inet_pton(type, address.c_str(), &ret.ipv4.sin_addr);
			if (i <= 0)
				return ret;
			else
			{
				ret.ipv4.sin_family = type;
				ret.ipv4.sin_port   = htons(port);
			}
		}
		case AF_INET6:
		{
			int i = inet_pton(type, address.c_str(), &ret.ipv6.sin6_addr);
			if (i <= 0)
				return ret;
			else
			{
				ret.ipv6.sin6_family = type;
				ret.ipv6.sin6_port   = htons(port);
			}
		}
		default:
			break;
	};

	return ret;
}

Socket::Socket(int sock, int type, int protocol)
{
	memset(&this->sa, 0, sizeof(sockaddr_t));
	if (sock == -1)
		this->sock_fd = ::socket(type, protocol, 0);
	else
		this->sock_fd = sock;
	// We pretty much ALWAYS want non-blocking sockets for this system.
	this->SetNonBlocking(true);

	SocketMultiplexer::AddSocket(this);
	this->SetFlags(MX_READABLE, MX_WRITABLE);
	SocketMultiplexer::UpdateSocket(this);
}

Socket::~Socket()
{
	"[Socket Engine] Destroying socket {}"_l(this->sock_fd);

	SocketMultiplexer::RemoveSocket(this);

	::close(this->sock_fd);
}

void Socket::SetNonBlocking(bool status)
{
	// Get the socket flags
	int flags = fcntl(this->sock_fd, F_GETFL, 0);

	if (status)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	if (fcntl(this->sock_fd, F_SETFL, flags) == -1)
		"Cannot set socket {} as non-blocking with flag O_NONBLOCK: {}"_lw(this->sock_fd, strerror(errno));
}

size_t Socket::Write(const void *data, size_t len)
{
	if (!data || !len)
		return 0;

	return ::send(this->sock_fd, data, len, 0);
}

size_t Socket::Read(void *data, size_t len) { return ::recv(this->sock_fd, data, len, 0); }

bool Socket::MultiplexEvent() { return true; }
void Socket::MultiplexError() {}
bool Socket::MultiplexRead() { return true; }
bool Socket::MultiplexWrite() { return true; }

ConnectionSocket::ConnectionSocket(bool ipv6) : Socket(-1, ipv6) {}
ConnectionSocket::~ConnectionSocket() {}
void ConnectionSocket::OnError(const Flux::string &) {}

bool ConnectionSocket::MultiplexEvent()
{
	if (this->HasFlag(SS_CONNECTED))
		return true;
	else if (this->HasFlag(SS_CONNECTING))
	{
		int		  optval = 0;
		socklen_t optlen = sizeof(int);
		if (!getsockopt(this->sock_fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&optval), &optlen) && !optval)
		{
			this->RemoveFlag(SS_CONNECTING);
			this->SetFlag(SS_CONNECTED);
			this->OnConnect();
		}
		else
		{
			errno = optval;
			this->OnError(optval ? strerror(errno) : "");
			"Socket {} being killed before connect() could finish."_l(this->sock_fd);
			this->SetFlag(SS_DEAD);
		}
	}
	else
		this->SetFlag(SS_DEAD);

	return false;
}

void ConnectionSocket::MultiplexError()
{
	int		  optval = 0;
	socklen_t optlen = sizeof(int);
	getsockopt(this->sock_fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&optval), &optlen);
	errno = optval;
	this->OnError(optval ? strerror(errno) : "");
}

void ConnectionSocket::Connect(const Flux::string &conaddr, short port)
{
	// Set some variables.
	this->address = conaddr;
	this->port	= port;

	// Get our address family.
	this->sa.sa.sa_family = (conaddr.find(":") != Flux::string::npos ? AF_INET6 : AF_INET);
	// Set our port
	*(this->sa.sa.sa_family == AF_INET ? &this->sa.ipv4.sin_port : &this->sa.ipv6.sin6_port) = htons(port);

	// Convert our address to a sockaddr_t structure.
	int value = -1;
	if (this->sa.sa.sa_family == AF_INET)
		value = inet_pton(this->sa.sa.sa_family, conaddr.c_str(), &this->sa.ipv4.sin_addr);
	else if (this->sa.sa.sa_family == AF_INET6)
		value = inet_pton(this->sa.sa.sa_family, conaddr.c_str(), &this->sa.ipv6.sin6_addr);

	switch (value)
	{
		case 1: // success
			break;
		case 0:
			throw SocketException("Invalid host");
		default:
			throw SocketException("Invalid host: %s", strerror(errno));
	}

	if (::connect(this->sock_fd, &this->sa.sa, sizeof(struct sockaddr)) == -1)
	{
		if (errno != EINPROGRESS)
			this->OnError(strerror(errno));
		else
		{
			this->SetFlags(SS_CONNECTING, MX_WRITABLE);
			SocketMultiplexer::UpdateSocket(this);
		}
	}
	else
	{
		this->SetFlag(SS_CONNECTED);
		this->OnConnect();
	}
}

ListeningSocket::ListeningSocket(const Flux::string &bindaddr, short port, bool ipv6) : Socket(-1, ipv6)
{
retry:
	memset(&this->sa, 0, sizeof(sockaddr_t));
	const char op = 1;
	setsockopt(this->sock_fd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op));

	// Set some variables.
	this->address = bindaddr;
	this->port	= port;

	// Get our address family.
	this->sa.sa.sa_family = (ipv6 ? AF_INET6 : AF_INET);

	// Convert our address to a sockaddr_t structure.
	int value = -1;
	if (this->sa.sa.sa_family == AF_INET)
	{
		this->sa.ipv4.sin_family = AF_INET;
		this->sa.ipv4.sin_port   = htons(port);
		value					 = inet_pton(AF_INET, bindaddr.c_str(), &this->sa.ipv4.sin_addr);
	}
	else if (this->sa.sa.sa_family == AF_INET6)
	{
		this->sa.ipv6.sin6_family = AF_INET6;
		this->sa.ipv6.sin6_port   = htons(port);
		value					  = inet_pton(AF_INET, bindaddr.c_str(), &this->sa.ipv6.sin6_addr);
	}
	else
		"Unknown family type: {}"_lw(this->sa.sa.sa_family);

	switch (value)
	{
		case 1: // success
			break;
		case 0:
			throw SocketException("Invalid host");
		default:
			throw SocketException("Invalid host: %s", strerror(errno));
	}

	try
	{
		// Log(LOG_VERBOSE) << "Stupid fucking address family: " << (this->sa.sa.sa_family);
		if (::bind(this->sock_fd, &this->sa.sa, sizeof(sockaddr_t)) == -1)
			throw SocketException("Unable to bind to %s:%s: %s (is ipv6? %s)",
								  bindaddr,
								  port,
								  strerror(errno),
								  this->sa.sa.sa_family == AF_INET ? "yes" : "no");
	}
	catch (SocketException &e)
	{
		"[Socket Engine]: {}"_le(e.what());
		goto retry;
	}

	if (::listen(this->sock_fd, SOMAXCONN) == -1)
		throw SocketException("Unable to listen: %s", strerror(errno));
}

ListeningSocket::~ListeningSocket() {}

bool ListeningSocket::MultiplexRead()
{
	sockaddr_t addr;

	socklen_t size	= sizeof(sockaddr_t);
	int		  newsock = accept(this->sock_fd, &addr.sa, &size);

	if (newsock >= 0)
	{
		ClientSocket *cs = this->OnAccept(newsock, addr);
		if (cs)
		{
			cs->SetFlag(SS_ACCEPTED);
			cs->OnAccept();
		}
		else
			throw SocketException("ListenSocket::OnAccept() returned nullptr for client socket");
	}
	else
		"Unable to accept connection: {}"_lw(strerror(errno));

	return true;
}

ClientSocket::ClientSocket(ListeningSocket *ls, int fd, const sockaddr_t &addr, bool ipv6) : Socket(fd, false), ls(ls)
{
	this->SetFlag(SS_ACCEPTING);
	memcpy(&this->sa, &addr, sizeof(sockaddr_t));
}

bool ClientSocket::MultiplexEvent()
{
	if (this->HasFlag(SS_ACCEPTED))
		return true;
	else if (this->HasFlag(SS_ACCEPTING))
	{
		this->SetFlag(SS_ACCEPTED);
		this->RemoveFlag(SS_ACCEPTING);
	}
	else
		this->SetFlag(SS_DEAD);

	return false;
}

void ClientSocket::MultiplexError()
{
	int		  optval = 0;
	socklen_t optlen = sizeof(int);
	getsockopt(this->sock_fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&optval), &optlen);
	errno = optval;
	this->OnError(optval ? strerror(errno) : "");
}

void ClientSocket::OnAccept() {}
void ClientSocket::OnError(const Flux::string &str) {}
