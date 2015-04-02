// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcserver.h"
#include "rpcclient.h"
#include "util.h"
#include "json/json_spirit_value.h"
#include <openssl/crypto.h>

#include "common/commandLine.h"

#include <boost/thread.hpp>

namespace common
{

CCommandLine * CCommandLine::ms_instance = NULL;

CCommandLine*
CCommandLine::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CCommandLine();
	};
	return ms_instance;
}

void
CCommandLine::reply( CMessageClass::Enum _category, std::string const & _commandResponse )
{
	std::string comandResponse;
	switch( _category )
	{
	case CMessageClass::MC_ERROR:
		comandResponse = "CODE ERROR: ";
		break;
	case CMessageClass::MC_DEBUG:
		comandResponse = "CODE DEBUG: ";
		break;
	case CMessageClass::CMD_REQUEST:
		comandResponse = "COMMAND REQUEST: ";
		break;
	case CMessageClass::CMD_REPLY:
		comandResponse = "COMMAND REPLY: ";
		break;
	case CMessageClass::CMD_ERROR:
		comandResponse = "COMMAND ERROR: ";
		break;
	default:
		break;
	}

	addOutputMessage( comandResponse + _commandResponse );
}

void
CCommandLine::workLoop()
{
	fd_set read_fds;
	timeval waitTime;
	waitTime.tv_sec = 0;
	waitTime.tv_usec = 0;

	bool promptPrinted = false;
	while ( 1 )
	{

		if ( !promptPrinted )
		{
			promptPrinted = true;
			std::cout << "PROMPT > ";
			std::cout.flush();
		}

		std::string line;

		FD_ZERO(&read_fds);
		FD_SET( STDIN_FILENO, &read_fds );

		int result = select( FD_SETSIZE, &read_fds, NULL, NULL, &waitTime );
		if (result == -1 && errno != EINTR)
		{
			assert(!"Error in select");
		}
		else if (result == -1 && errno == EINTR)
		{
			assert(!"problem");
		}
		else
		{
			if (FD_ISSET(STDIN_FILENO, &read_fds))
			{
				promptPrinted = false;
				std::getline(std::cin, line);
			}
		}

		if ( !line.empty() )
		{
			request( line );
		}

		boost::lock_guard<boost::mutex> lock( m_lock );
		BOOST_FOREACH( std::string const & out, m_outputs )
		{
			std::cout << out << "\n\n";
		}

		m_outputs.clear();
		MilliSleep(10);
		boost::this_thread::interruption_point();
	}
}

bool
CCommandLine::parseCommandLine( std::vector<std::string> & _args, const std::string & _strCommand )
{
	enum CmdParseState
	{
		STATE_EATING_SPACES,
		STATE_ARGUMENT,
		STATE_SINGLEQUOTED,
		STATE_DOUBLEQUOTED,
		STATE_ESCAPE_OUTER,
		STATE_ESCAPE_DOUBLEQUOTED
	} state = STATE_EATING_SPACES;
	std::string curarg;
	BOOST_FOREACH( char const & ch, _strCommand )
	{
		switch(state)
		{
		case STATE_ARGUMENT: // In or after argument
		case STATE_EATING_SPACES: // Handle runs of whitespace
			switch(ch)
			{
			case '"': state = STATE_DOUBLEQUOTED; break;
			case '\'': state = STATE_SINGLEQUOTED; break;
			case '\\': state = STATE_ESCAPE_OUTER; break;
			case ' ': case '\n': case '\t':
				if( state == STATE_ARGUMENT ) // Space ends argument
				{
					_args.push_back(curarg);
					curarg.clear();
				}
				state = STATE_EATING_SPACES;
				break;
			default: curarg += ch; state = STATE_ARGUMENT;
			}
			break;
		case STATE_SINGLEQUOTED: // Single-quoted string
			switch(ch)
			{
			case '\'': state = STATE_ARGUMENT; break;
			default: curarg += ch;
			}
			break;
		case STATE_DOUBLEQUOTED: // Double-quoted string
			switch(ch)
			{
			case '"': state = STATE_ARGUMENT; break;
			case '\\': state = STATE_ESCAPE_DOUBLEQUOTED; break;
			default: curarg += ch;
			}
			break;
		case STATE_ESCAPE_OUTER: // '\' outside quotes
			curarg += ch; state = STATE_ARGUMENT;
			break;
		case STATE_ESCAPE_DOUBLEQUOTED: // '\' in double-quoted text
			if(ch != '"' && ch != '\\') curarg += '\\'; // keep '\' for everything but the quote and '\' itself
			curarg += ch; state = STATE_DOUBLEQUOTED;
			break;
		}
	}
	switch(state) // final state
	{
	case STATE_EATING_SPACES:
		return true;
	case STATE_ARGUMENT:
		_args.push_back( curarg );
		return true;
	default: // ERROR to end in one of the other states
		return false;
	}
}

void
CCommandLine::request( std::string const & _command )
{
	std::vector<std::string> args;

	if( !parseCommandLine( args, _command ) )
	{
		reply( CMessageClass::CMD_ERROR, "Parse error: unbalanced ' or \"" );
		return;
	}
	if(args.empty())
		return; // Nothing to do
	try
	{
		std::string strPrint;
		// Convert argument list to JSON objects in method-dependent way,
		// and pass it along with the method name to the dispatcher.
		json_spirit::Value result = tableRPC.execute(
			args[0],
			RPCConvertValues(args[0], std::vector<std::string>(args.begin() + 1, args.end())));

		// Format result reply
		if (result.type() == json_spirit::null_type)
			strPrint = "";
		else if (result.type() == json_spirit::str_type)
			strPrint = result.get_str();
		else
			strPrint = write_string(result, true);

		reply( CMessageClass::CMD_REPLY, strPrint );
	}
	catch (json_spirit::Object& objError)
	{
		try // Nice formatting for standard-format error
		{
			int code = find_value(objError, "code").get_int();
			std::string message = find_value(objError, "message").get_str();

			std::ostringstream errorStream;
			errorStream << code;

			reply(CMessageClass::CMD_ERROR, message + " (code " + errorStream.str() + ")");
		}
		catch(std::runtime_error &) // raised when converting to invalid type, i.e. missing code or message
		{   // Show raw JSON object
			reply( CMessageClass::CMD_ERROR, write_string( json_spirit::Value(objError), false) );
		}
	}
	catch (std::exception& e)
	{
		reply( CMessageClass::CMD_ERROR, std::string("Error: ") + e.what());
	}
}

}
