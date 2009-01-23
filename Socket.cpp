/* Copyright (C) 2005  Christoph Helma <helma@in-silico.de>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>

#include "Socket.h"
#include "string.h"

using namespace std;

Socket::Socket() :
        m_sock ( -1 )
{

    memset ( &m_addr,
             0,
             sizeof ( m_addr ) );

}

Socket::~Socket()
{
    if ( is_valid() )
        ::close ( m_sock );
}

bool Socket::create()
{
    m_sock = socket ( AF_INET,	// AF_INET im original
                      SOCK_STREAM,
                      0 );

    if ( ! is_valid() )
        return false;


    // TIME_WAIT - argh
    int on = 1;
    if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
        return false;


    return true;

}



bool Socket::bind ( const int port )
{

    if ( ! is_valid() )
    {
        return false;
    }



    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // INADDR_ANY;
    m_addr.sin_port = htons ( port );

    int bind_return = ::bind ( m_sock,
                               ( struct sockaddr * ) &m_addr,
                               sizeof ( m_addr ) );


    if ( bind_return == -1 )
    {
        return false;
    }

    return true;
}


bool Socket::listen() const
{
    if ( ! is_valid() )
    {
        return false;
    }

    int listen_return = ::listen ( m_sock, MAXCONNECTIONS );


    if ( listen_return == -1 )
    {
        return false;
    }

    return true;
}


bool Socket::accept ( Socket& new_socket ) const
{
    int addr_length = sizeof ( m_addr );
    new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );

    //return ( new_socket );
    if ( new_socket.m_sock <= 0 )
        return false;
    else
        return true;
}

bool Socket::remove ( Socket& new_socket ) const {
    ::close( new_socket.m_sock );
    /*
    if ( new_socket.m_sock > 0 )
    return false;
    else
    return true;
    */
    return true;
}

bool Socket::send ( const std::string s ) const
{
    int status = ::send ( m_sock, s.c_str(), s.size(), MSG_NOSIGNAL );
    if ( status == -1 )
    {
        return false;
    }
    else
    {
        return true;
    }
}


int Socket::recv ( std::string& s ) const
{
    char buf [ MAXRECV + 1 ];

    s = "";

    memset ( buf, 0, MAXRECV + 1 );

    int status = ::recv ( m_sock, buf, MAXRECV, 0 );

    if ( status == -1 )
    {
        cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
        return 0;
    }
    else if ( status == 0 )
    {
        return 0;
    }
    else
    {
        s = buf;
        return status;
    }
}



bool Socket::connect ( const std::string host, const int port )
{
    if ( ! is_valid() ) return false;

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons ( port );

    int status = inet_pton ( AF_INET, host.c_str(), &m_addr.sin_addr );

    if ( errno == EAFNOSUPPORT ) return false;

    status = ::connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );

    if ( status == 0 )
        return true;
    else
        return false;
}

void Socket::set_non_blocking ( const bool b )
{

    int opts;

    opts = fcntl ( m_sock,
                   F_GETFL );

    if ( opts < 0 )
    {
        return;
    }

    if ( b )
        opts = ( opts | O_NONBLOCK );
    else
        opts = ( opts & ~O_NONBLOCK );

    fcntl ( m_sock,
            F_SETFL,opts );

}
