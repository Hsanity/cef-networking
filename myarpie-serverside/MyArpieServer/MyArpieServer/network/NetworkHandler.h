#pragma once
#include <string>

#include "../abac/policyEnforcement.h"

#include <stack>

class NetworkHandler
{
public:
	NetworkHandler();
	~NetworkHandler();
	
	//Query Methods
	bool PostQuery(std::string query, std::vector<std::string> args);

	//Network Methods
	bool HandleConnection();
	bool SendRequest();
	std::list<std::string> ReceiveResult();

	bool Authenticate(std::vector<std::string> args)
	{
		return auth->Authenticate(args);
	}

	//temp
	std::string getLast()
	{
		return RequestBuffer.top();
	}

	bool hasResult()
	{
		return DB->hasResult();
	}

private:
	Sqlite* DB; //standalone
	Authenticator* auth; //standalone
	PolicyEnforcementPoint* PEP;

	std::stack<std::string> RequestBuffer = std::stack<std::string>();
	std::stack<std::vector<std::string>> ArgsBuffer = std::stack<std::vector<std::string>>();

	std::stack<std::string> ResultBuffer = std::stack<std::string>();
};