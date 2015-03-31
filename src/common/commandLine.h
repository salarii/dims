#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include "rpcserver.h"
#include "rpcclient.h"

#include "json/json_spirit_value.h"
#include <openssl/crypto.h>

#include <boost/foreach.hpp>

namespace common
{

class CCommandLine
{
public:
	struct CMessageClass
	{
		enum Enum
		{
			MC_ERROR,
			MC_DEBUG,
			CMD_REQUEST,
			CMD_REPLY,
			CMD_ERROR
		};
	};
public:
	void workLoop();
protected:
	void request( std::string const & _command );

	bool parseCommandLine( std::vector<std::string> & _args, std::string const & _strCommand );

	void reply( int _category, std::string const & _command ){};
protected:
	std::vector< std::string > m_communicatesToPrint;

	std::vector< std::string > m_history;
	int m_historyPtr;
};

void
CCommandLine::workLoop()
{

	while ( 1 )
	{
		std::string line;
		std::cout << "APP > " && std::getline(std::cin, line);

		if ( !line.empty() )
		;
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

			ostringstream errorStream;
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
/*
void RPCConsole::clear()
{
	history.clear();
	historyPtr = 0;

	message(CMD_REPLY, (tr("Welcome to the Bitcoin RPC console.") + "<br>" +
						tr("Use up and down arrows to navigate history, and <b>Ctrl-L</b> to clear screen.") + "<br>" +
						tr("Type <b>help</b> for an overview of available commands.")), true);
}


void RPCConsole::message(int category, const QString &message, bool html)
{
	QTime time = QTime::currentTime();
	QString timeString = time.toString();
	QString out;
	out += "<table><tr><td class=\"time\" width=\"65\">" + timeString + "</td>";
	out += "<td class=\"icon\" width=\"32\"><img src=\"" + categoryClass(category) + "\"></td>";
	out += "<td class=\"message " + categoryClass(category) + "\" valign=\"middle\">";
	if(html)
		out += message;
	else
		out += GUIUtil::HtmlEscape(message, true);
	out += "</td></tr></table>";
	ui->messagesWidget->append(out);
}

void RPCConsole::on_lineEdit_returnPressed()
{
	QString cmd = ui->lineEdit->text();
	ui->lineEdit->clear();

	if(!cmd.isEmpty())
	{
		message(CMD_REQUEST, cmd);
		cmdRequest(cmd);
		// Truncate history from current position
		history.erase(history.begin() + historyPtr, history.end());
		// Append command to history
		history.append(cmd);
		// Enforce maximum history size
		while(history.size() > CONSOLE_HISTORY)
			history.removeFirst();
		// Set pointer to end of history
		historyPtr = history.size();
		// Scroll console view to end
		scrollToEnd();
	}
}

void RPCConsole::browseHistory(int offset)
{
	historyPtr += offset;
	if(historyPtr < 0)
		historyPtr = 0;
	if(historyPtr > history.size())
		historyPtr = history.size();
	QString cmd;
	if(historyPtr < history.size())
		cmd = history.at(historyPtr);
	ui->lineEdit->setText(cmd);
}
*/
}

#endif // COMMAND_LINE_H
