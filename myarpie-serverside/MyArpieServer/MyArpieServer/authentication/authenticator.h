#pragma once
#include "../databaseLayer.h"

class Authenticator
{
public:
	Authenticator();
	Authenticator(Sqlite* DB);
	~Authenticator();

	static Authenticator* getInstance();

	bool SessionAlive()
	{
		return true;
	}

	std::string getUsername()
	{
		return username;
	}

	std::string getRole()
	{
		return role;
	}

	bool Authenticate(std::vector<std::string> args);

private:
	bool test = true;
	Sqlite* DB;

	std::string username = std::string("default");
	std::string role = std::string("default");
	std::chrono::time_point<std::chrono::high_resolution_clock> SessionStart;
};