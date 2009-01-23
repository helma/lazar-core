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
// Implementation of the ServerSocket class

#include "ServerSocket.h"

ServerSocket::ServerSocket ( int port )
{
    if ( ! Socket::create() )
    {
        throw "Could not create server socket." ;
    }

    if ( ! Socket::bind ( port ) )
    {
        throw "Could not bind to port." ;
    }

    if ( ! Socket::listen() )
    {
        throw "Could not listen to socket." ;
    }

}

ServerSocket::~ServerSocket()
{
}


const ServerSocket& ServerSocket::operator << ( const std::string& s ) const
{
    if ( ! Socket::send ( s ) )
    {
        throw "Could not write to socket." ;
    }

    return *this;

}


const ServerSocket& ServerSocket::operator >> ( std::string& s ) const
{

    if ( ! Socket::recv ( s ) )
    {
        throw "Could not read from socket." ;
    }

    return *this;
}

void ServerSocket::accept ( ServerSocket& sock )
{
    if ( ! Socket::accept ( sock ) )
    {
        throw "Could not accept socket." ;
    }
}

void ServerSocket::remove ( ServerSocket& sock )
{
    if ( ! Socket::remove ( sock ) )
    {
        throw "Could not remove socket." ;
    }
}
