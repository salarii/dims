// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

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

	static CCommandLine* getInstance();
protected:
	CCommandLine(){};

	void request( std::string const & _command );

	bool parseCommandLine( std::vector<std::string> & _args, std::string const & _strCommand );

	void reply( CMessageClass::Enum _category, std::string const & _commandResponse );

	void addOutputMessage( std::string const & _outputMessage )
	{
		boost::lock_guard<boost::mutex> lock( m_lock );
		m_outputs.push_back( _outputMessage );
	}
protected:
	mutable boost::mutex m_lock;

	std::list< std::string > m_outputs;

	static CCommandLine * ms_instance;

	std::vector< std::string > m_history;
	int m_historyPtr;
};

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
