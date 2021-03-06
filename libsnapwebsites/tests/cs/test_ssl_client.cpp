// Snap Websites Server -- test SSL on sockets (client side)
// Copyright (c) 2014-2019  Made to Order Software Corp.  All Rights Reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//
// This test works along the test_ssl_server.cpp and implements the
// client's side. It connects to the server, sends a few messages (START,
// PAUSE x 4, STOP) and expects a HUP before quitting.
//
// To run the test, you first have to start the server otherwise the
// client won't be able to connect.
//


// snapwebsites lib
//
#include <snapwebsites/log.h>
#include <snapwebsites/qstring_stream.h>
#include <snapwebsites/snap_communicator.h>


// snapdev lib
//
#include <snapdev/not_reached.h>


// Qt lib
//
#include <QDir>


// C++ lib
//
#include <iostream>


// C lib
//
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <unistd.h>


// last include
//
#include <snapdev/poison.h>






class messenger_connection
        : public snap::snap_communicator::snap_tcp_client_message_connection
{
public:
    messenger_connection()
        : snap_tcp_client_message_connection(
                    "127.0.0.1",
                    4030,
                    tcp_client_server::bio_client::mode_t::MODE_SECURE)     // create an encrypted connection, no need for strong verification of certificate though
    {
        set_name("messenger");
    }

    virtual void process_message(snap::snap_communicator_message const & message)
    {
        SNAP_LOG_INFO("process_message() client side: [")(message.to_message())("]");
    }

    virtual void process_timeout()
    {
        SNAP_LOG_INFO("process_timeout() called.");

        snap::snap_communicator_message msg;
        switch(f_state)
        {
        case 0:
            // sent to server
            msg.set_command("START");
            break;

        case 1:
        case 2:
        case 3:
        case 4:
            // sent "stuff" to client
            msg.set_command("PAUSE");
            break;

        case 5:
            // end everything (server closes socket)
            msg.set_command("STOP");

            // stop the timer and just wait on HUP now
            set_timeout_delay(-1);
            break;

        default:
            throw std::runtime_error("unknown f_state in client.");

        }
        SNAP_LOG_INFO("client sending message: [")(msg.to_message())("]");
        send_message(msg);

        ++f_state;
    }

    int f_state = 0;
};



int main(int /*argc*/, char * /*argv*/[])
{
    try
    {
        snap::logging::set_progname("test_ssl_server");
        snap::logging::configure_console();
        snap::logging::set_log_output_level(snap::logging::log_level_t::LOG_LEVEL_TRACE);

        messenger_connection::pointer_t mc(new messenger_connection);
        mc->set_timeout_delay(1LL * 1000000LL);

        int flag(1);
        if(setsockopt(mc->get_socket(), IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) < 0)
        {
            int const e(errno);
            SNAP_LOG_WARNING("setsockopt() with TCP_NODELAY failed. (errno: ")(e)(" -- ")(strerror(e))(")");
        }

        snap::snap_communicator::instance()->add_connection(mc);

        snap::snap_communicator::instance()->run();

        SNAP_LOG_INFO("exited run() loop...");

        return 0;
    }
    catch( std::exception const & e )
    {
        SNAP_LOG_FATAL("Caught exception [")(e.what())("].");
    }

    return 1;
}

// vim: ts=4 sw=4 et
