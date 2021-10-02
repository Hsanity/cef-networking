#include "authenticator.h"
#include "libbcrypt/include/bcrypt/BCrypt.hpp"

namespace {

Authenticator* a_handle = nullptr;

}

Authenticator::Authenticator()
{
	a_handle = this;
	DB = nullptr;
}

Authenticator::Authenticator(Sqlite* DB)
{
	a_handle = this;
	this->DB = DB;
}
Authenticator::~Authenticator()
{
	a_handle = nullptr;
}
Authenticator* Authenticator::getInstance()
{
	return a_handle;
}
bool Authenticator::Authenticate(std::vector<std::string> args)
{
	//Authenticate the nigga
	//args[1] = BCrypt::generateHash(args[1], 12);

	DB->ProcessQuery("Authenticate", args);

	//std::cout << BCrypt::validatePassword(args[1], hash) << std::endl;


	if(DB->hasResult())
	{
		std::list<std::string> data = DB->getResult();
		std::vector<std::string> vdata(data.size());

		std::copy(data.begin(), data.end(), vdata.begin());

		if (strcmp(vdata.at(0).c_str(), "Success") == 0)
		{
			vdata.erase(vdata.begin());
		}
		else
		{
			if(BCrypt::validatePassword(args[1], vdata[2]))
			{
				role = vdata[0];
				username = vdata[1];

				//role = role.substr(0, role.find_first_of(';', 0));
				SessionStart = std::chrono::high_resolution_clock::now();

				//auto j = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - SessionStart).count();
				return true;
			}
			else
			{
				DB->clearResults();
			}
		}
	}

	return false;
}