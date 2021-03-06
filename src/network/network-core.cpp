//
// C++ Implementation: network-core
//
// Description:
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "network-core.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/shared_array.hpp>

using namespace std;

#ifdef HAVE_WINSOCK
	WinSockClass winsockinstance;
	int WinSockClass::counter = 0;
#else
	SIGPipeClass sigpipeinstance;
#endif

tcp_socket::tcp_socket()
{
	sock = INVALID_SOCKET;
}

tcp_listen_socket::tcp_listen_socket()
{
	sock = INVALID_SOCKET;
}

udp_socket::udp_socket()
{
	sock = INVALID_SOCKET;
}

tcp_socket::tcp_socket( SOCKET s, ipv4_socket_addr addr) {
	sock = s;
	peer = addr;
}

tcp_socket::tcp_socket( const ipv4_socket_addr addr) {
	sock = INVALID_SOCKET;
	this->connect(addr.first, addr.second);
}

tcp_socket::tcp_socket( const ipv4_addr addr, const uint16 port )
{
	sock = INVALID_SOCKET;
	this->connect(addr, port);
}

void tcp_socket::connect( const ipv4_addr addr, const uint16 port )
{
	this->disconnect();
	try {
		sock = socket( AF_INET, SOCK_STREAM, 0 );

		if (sock == INVALID_SOCKET) throw std::runtime_error("failed to create tcp socket");

		sockaddr_in addr_in;
		addr_in.sin_family = AF_INET;
		addr_in.sin_addr.s_addr = addr.full;
		addr_in.sin_port = htons( port );

		int res = ::connect(sock, (sockaddr*)&addr_in, sizeof(addr_in));
		if (res == SOCKET_ERROR) throw std::runtime_error("failed to connect tcp socket");

		peer = ipv4_socket_addr(ipv4_addr(addr_in.sin_addr.s_addr), ntohs(addr_in.sin_port));
	} catch (...) {
		this->disconnect();
		throw;
	}
}

uint32 tcp_socket::send( const uint8* buf, const uint32 len )
{
	uint32 done = 0;
	while (done < len) {
		int sent = ::send(sock, (char*)buf+done, len-done, 0);
		if (sent <= 0)
			return done;
		done += sent;
	}
	return done;
}

uint32 tcp_socket::receive( const uint8* buf, const uint32 len )
{
	uint32 done = 0;
	while (done < len) {
		int sent = ::recv(sock, (char*)buf+done, len-done, 0);
		if (sent <= 0)
			return done;
		done += sent;
	}
	return done;
}

void tcp_listen_socket::disconnect()
{
	if (sock != INVALID_SOCKET) {
		shutdown(sock, SHUT_RDWR);
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
}

void tcp_socket::disconnect()
{
	if (sock != INVALID_SOCKET) {
		shutdown(sock, SHUT_RDWR);
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
}

tcp_socket::~tcp_socket()
{
	disconnect();
}

tcp_listen_socket::~tcp_listen_socket()
{
	disconnect();
}

ipv4_socket_addr tcp_socket::get_ipv4_socket_addr() {
	return peer;
}

tcp_listen_socket::tcp_listen_socket(const uint16 portnumber)
{
	sock = INVALID_SOCKET;
	listen(ipv4_addr(0), portnumber);
}

tcp_listen_socket::tcp_listen_socket(const ipv4_addr addr, const uint16 portnumber)
{
	sock = INVALID_SOCKET;
	listen(addr, portnumber);
}

void tcp_listen_socket::listen(const ipv4_addr addr, const uint16 portnumber)
{
	disconnect();
	sock = socket( AF_INET, SOCK_STREAM, 0 );

	sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.s_addr = (addr.full == 0) ? INADDR_ANY : addr.full;
	addr_in.sin_port = htons( portnumber );

	// We set SO_REUSEADDR, so that we may re-use the same portnumber
	// incase we need to quickly shutdown and restart.
	int optval = 1;
	if ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, ( char* )&optval, sizeof( optval )) == SOCKET_ERROR ) {
		stringstream ss;
		ss << "tcp_listen_socket: Could not set socket option: error " << NetGetLastError() << "(" << strerror(NetGetLastError()) << ")" << "\n";
		cout << ss.str();
		disconnect();
		throw std::runtime_error(ss.str().c_str());
	}

	if ( ::bind( sock, ( sockaddr* )&addr_in, sizeof( addr_in ) ) == SOCKET_ERROR ) {
		stringstream ss;
		ss << "tcp_listen_socket: Bind to network failed: error " << NetGetLastError() << "(" << strerror(NetGetLastError()) << ")" << "\n";
		cout << ss.str();
		disconnect();
		throw std::runtime_error(ss.str().c_str());
	}

	socklen_t addrlen = sizeof( addr_in );
	if (getsockname(sock, ( sockaddr* )&addr_in, &addrlen) == SOCKET_ERROR ) {
		disconnect();
		throw std::runtime_error("unable to retrieve port");
	}
	port = htons(addr_in.sin_port); // assume x = htons(htons(x))

	if(::listen(sock, 16)) { //Queue up to 16 connections
		cout << "tcp_listen_socket: Could not listen() on socket: error " << NetGetLastError() << "(" << strerror(NetGetLastError()) << ")" << "\n";
	}
}

tcp_socket* tcp_listen_socket::accept() {
	sockaddr_in peer;
	socklen_t len = sizeof(peer);
	SOCKET s = ::accept(sock, ( sockaddr* )&peer, &len);
	if(s != SOCKET_ERROR)
		return new tcp_socket(s, ipv4_socket_addr(peer.sin_addr.s_addr,peer.sin_port));
	return NULL;
}

udp_socket::udp_socket(const ipv4_addr addr, const uint16 portnumber) {
	bind(addr, portnumber);
}

bool udp_socket::bind(const ipv4_addr addr, const uint16 portnumber) {
	sock = socket( AF_INET, SOCK_DGRAM, 0 );

	sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.s_addr = (addr.full == 0) ? INADDR_ANY : addr.full;
	addr_in.sin_port = htons( portnumber );

	if (/*portnumber && */::bind( sock, ( sockaddr* )&addr_in, sizeof( addr_in ) ) == SOCKET_ERROR ) {
		close();
		stringstream ss;
		ss << "udp_socket: Bind to network failed: error " << NetGetLastError() << "(" << strerror(NetGetLastError()) << ")";
		throw std::runtime_error(ss.str().c_str());
	}

	// now enable broadcasting
	unsigned long bc = 1;
	if ( setsockopt( sock, SOL_SOCKET, SO_BROADCAST, ( char* )&bc, sizeof( bc )) == SOCKET_ERROR ) {
		close();
		stringstream ss;
		ss << "udp_socket: Unable to enable broadcasting: error " << NetGetLastError() << "(" << strerror(NetGetLastError()) << ")";
		throw std::runtime_error(ss.str().c_str());
	};
	return true;
}

uint32 udp_socket::send( const ipv4_addr dest_addr, const uint16 dest_port, const uint8* buf, const uint32 len ) {
	sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.s_addr = dest_addr.full;
	addr_in.sin_port = htons( dest_port );
	return sendto(sock, (char*)buf, len, 0, (sockaddr*) &addr_in, sizeof(addr_in));
}

uint32 udp_socket::receive( ipv4_addr* from_addr, uint16* from_port, const uint8* buf, const uint32 len ) {
	sockaddr_in addr_in;
	socklen_t addr_in_len = sizeof(addr_in);
	int32 retval = recvfrom( sock, (char*)buf, len, 0, (sockaddr*)&addr_in, &addr_in_len);
	from_addr->full = addr_in.sin_addr.s_addr;
	*from_port = ntohs(addr_in.sin_port);
	if (retval >0)
		return retval;
	return 0;
}

void udp_socket::close() {
	if (sock != INVALID_SOCKET) {
		shutdown(sock, SHUT_RDWR);
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
}

string ipv4_socket_addr::std_str() const {
	std::stringstream ss;
	ss << (*this);
	return ss.str();
};

/** Stream operators **/

std::ostream& operator<<(std::ostream& os, const ipv4_addr& addr) {
		return os << (int)addr.array[0] << "." << (int)addr.array[1] << "." << (int)addr.array[2] << "." << (int)addr.array[3];
}

std::ostream& operator<<(std::ostream& os, const ipv4_socket_addr& saddr) {
		return os << saddr.first << ":" << saddr.second;
}

ipv4_addr ipv4_lookup(const std::string& host)
{
	ipv4_addr address;
	{
		unsigned long hostaddr = inet_addr( host.c_str() );
		if( hostaddr == INADDR_NONE )
		{
			hostent *hostentry = gethostbyname( host.c_str() );
			if(hostentry)
				hostaddr = *reinterpret_cast<unsigned long *>( hostentry->h_addr_list[0] );
		}
		address.full = hostaddr;
	}
	return address;
}

void tcp_socket::swap(tcp_socket& o)
{
	ipv4_socket_addr taddr = peer;
	SOCKET tsock = sock;
	peer = o.peer;
	sock = o.sock;
	o.peer = taddr;
	o.sock = tsock;
}
