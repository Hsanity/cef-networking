#include "NetworkHandler.h"

NetworkHandler::NetworkHandler()
{
	DB = new Sqlite();

	PEP = new PolicyEnforcementPoint();

	//if standalone
	auth = new Authenticator(DB);
}

NetworkHandler::~NetworkHandler()
{
	delete PEP;
	delete DB;
}

bool NetworkHandler::PostQuery(std::string query, std::vector<std::string> args)
{
	RequestBuffer.push(query);
	ArgsBuffer.push(args);

	if (!HandleConnection())
		return false;


	return true;
}

bool NetworkHandler::HandleConnection()
{
#ifdef OS_WIN
	//If connection is active...
	if (true)
		SendRequest();
	//Attempt reconnect
#endif // OS_WIN

#ifdef OS_MACOSX

#endif // OS_MACOSX

	return true;
}

bool NetworkHandler::SendRequest()
{
	std::string query = std::string(RequestBuffer.top());
	std::vector<std::string> args = ArgsBuffer.top();
	//if standalone
	if (true)
	{
		//Authenticator auth;

		//Proceed to PEP/PDP
		if(PEP->RequestDecision(query))
		{
			//Proceed to SQL
			DB->ProcessQuery(query, args);
		}
	}
	//else: Send to server

	RequestBuffer.pop();

	return true;
}

std::list<std::string> NetworkHandler::ReceiveResult()
{
	std::list<std::string> result = DB->getResultClear();

	return result;
}