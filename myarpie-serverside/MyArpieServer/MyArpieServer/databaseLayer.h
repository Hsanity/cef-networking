#pragma once
#include "databaseResources.h"

class Sqlite
{
public:
	Sqlite();
	~Sqlite();

	void ProcessQuery(std::string query, std::vector<std::string> args);
	bool hasResult();
	std::list<std::string> getResult();
	std::list<std::string> getResultClear();
	std::list<std::string> clearResults();

private:
	sqlite3* db;
	std::string DB_Path;
	std::string LOG_Path;
	
	void dailyBackup(std::string _path);
};