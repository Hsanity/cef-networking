#include "databaseLayer.h"

typedef std::string(*FnPtr)(std::vector<std::string>);

namespace
{
	std::string result;
	std::list<std::string> data;
}

std::string Authenticate(std::vector<std::string> args)
{
	return	"SELECT Roles.Role, 'Employee Access'.UserName, 'Employee Access'.Password"
		" FROM 'Employee Access'"
		" INNER JOIN Roles"
		" ON 'Employee Access'.'Role ID' = Roles.'Role ID'"
		" WHERE 'Employee Access'.UserName = \"" + args[0] + "\"";
}

std::string SetEntry(std::vector<std::string> args)
{
	time_t rawtime;
	time(&rawtime);
	struct tm* timeinfo;
	timeinfo = localtime(&rawtime);

	char day[3], month[3], year[5], sec[3], min[3], hour[3];

	itoa(timeinfo->tm_mday, day, 10);
	itoa(timeinfo->tm_mon + 1, month, 10);
	itoa(timeinfo->tm_year + 1900, year, 10);
	itoa(timeinfo->tm_min, min, 10);
	itoa(timeinfo->tm_hour, hour, 10);
	itoa(timeinfo->tm_sec, sec, 10);

	std::string query = "BEGIN TRANSACTION; INSERT INTO 'Journal Entry' VALUES "
		"(NULL, "
		"(SELECT `Ledger ID` FROM 'Ledgers' WHERE 'Ledgers'.`Name` = \"" + args[0] + "\"), " +
		args[1] + ", " + args[2] + ", " + args[3] + ", "
		"0, " //Employee ID
		"\""+ args[5] + "\", \"" + args[args.size() - 1] + "\"); COMMIT; ";

	query +=	"BEGIN TRANSACTION; "
				"INSERT INTO 'Journal Entry Submission' VALUES "
				"((SELECT MAX(`Journal ID`) FROM 'Journal Entry'), ";
	query += day;
	query += ", ";
	query += month;
	query += ", ";
	query += year;
	query += ", ";
	query += hour;
	query += ", ";
	query += min;
	query += ", ";
	query += sec;
	query +=	");"
				" COMMIT; ";

	query +=	"BEGIN TRANSACTION; UPDATE 'Ledgers' SET `Count` = `Count` + 1 "
				"WHERE 'Ledgers'.`Name` = \"" + args[0] + "\"; COMMIT; ";

	int fileCount = std::stoi(args[6]);
	std::vector<std::string> files;

	if(fileCount > 0)
		query += "BEGIN TRANSACTION; INSERT INTO 'Attached Documents' VALUES ";

	for(int i = 0; i < fileCount; i++)
	{
		std::string temp = args[7 + i];
		std::replace(temp.begin(), temp.end(), '?', '\\');

		files.push_back(temp);
	}

	for (int i = 0; i < fileCount; i++)
	{
		query += "((SELECT MAX(`Journal ID`) FROM 'Journal Entry'),\"" + files[i] + "\"), ";
	}

	if (fileCount > 0)
	{
		query = query.substr(0, query.size() - 2);
		query +=	"; COMMIT; ";
	}


	query +=	"BEGIN TRANSACTION; INSERT INTO 'Journal Row' VALUES ";

	int rows = std::stoi(args[7 + fileCount]);
	int invRows = std::stoi(args[args.size() - 2]);

	std::string calc = "; COMMIT; ";
	std::string log = "";
	std::string invMod = "";
	std::string invCalc = "";
	std::string invLog = "";
	std::string invBatch = "";

	for(int i = 0; i < rows - invRows; i++)
	{
		query += 
			"(NULL, "
			"(SELECT MAX(`Journal ID`) FROM 'Journal Entry'), "
			+ args[(i * 7) + 8 + fileCount] + ", "
			"(SELECT `Account ID` FROM 'Accounts' WHERE "
			"'Accounts'.`Name`=\"" + args[(i * 7) + 9 + fileCount] + "\" ), "
			+ args[(i * 7) + 10 + fileCount] + ", "
			+ args[(i * 7) + 11 + fileCount] + ", " //Day
			+ args[(i * 7) + 12 + fileCount] + ", " //Month
			+ args[(i * 7) + 13 + fileCount] + ", " //Year
			"\""+ args[(i * 7) + 14 + fileCount] + "\"), "; //Description

		calc +=
			" BEGIN TRANSACTION; UPDATE 'Accounts' SET `Amount` = `Amount` + " + args[(i * 7) + 10 + fileCount] + " "
			"WHERE `Name`= \"" + args[(i * 7) + 9 + fileCount] + "\"; "
			"COMMIT; ";

		log +=
			" BEGIN TRANSACTION; "
			"INSERT INTO 'Account Log'(`Year`,`Month`,`Day`,`Account ID`,`Amount`) "

			"SELECT " + args[(i * 7) + 13 + fileCount] + ", " + args[(i * 7) + 12 + fileCount] + ", " + args[(i * 7) + 11 + fileCount] + ", "

			"(SELECT `Account ID` FROM 'Accounts' WHERE 'Accounts'.`Name`= \"" + args[(i * 7) + 9 + fileCount] + "\"), "
			"(SELECT `Amount` FROM 'Account Log' "

			"WHERE( "

			"(`Day` < " + args[(i * 7) + 11 + fileCount] + " AND `Month` = " + args[(i * 7) + 12 + fileCount] + " AND `Year` = " + args[(i * 7) + 13 + fileCount] + ") OR "
			"(`Day` <= 31 AND `Month` = " + args[(i * 7) + 12 + fileCount] + "  AND `Year` = " + args[(i * 7) + 13 + fileCount] + ") OR "

			"(`Day` <= 31 AND `Month` < " + args[(i * 7) + 12 + fileCount] + " AND `Year` = " + args[(i * 7) + 12 + fileCount] + ") OR "

			"(`Day` <= 31 AND `Month` <= 12 AND `Year` < " + args[(i * 7) + 13 + fileCount] + ") "

			") AND "

			"(`Account ID` = (SELECT `Account ID` FROM 'Accounts' WHERE `Name`=\"" + args[(i * 7) + 9 + fileCount] + "\")) "
			"Order BY `Year` DESC, `Month` DESC, `Day` DESC LIMIT 1) "
			"WHERE NOT EXISTS "
			"( "
			"SELECT * FROM 'Account Log' WHERE `Account ID` = "
			"(SELECT `Account ID` FROM 'Accounts' WHERE "


			"`Name`=\"" + args[(i * 7) + 9 + fileCount] + "\" AND(`Day` = " + args[(i * 7) + 11 + fileCount] + " AND `Month` = " + args[(i * 7) + 12 + fileCount] + " AND `Year` = " + args[(i * 7) + 13 + fileCount] + ")) "


			"); "


			"COMMIT; "
			"BEGIN TRANSACTION; "
			"UPDATE 'Account Log' SET `Amount` = `Amount` + " + args[(i * 7) + 10 + fileCount] + " "

			"WHERE EXISTS "
			"( "
			"SELECT * FROM 'Account Log' WHERE `Account ID` = "
			"( "
			"SELECT `Account ID` FROM 'Accounts' WHERE `Name`=\"" + args[(i * 7) + 9 + fileCount] + "\" AND "
			"( "
			"(`Day` = " + args[(i * 7) + 11 + fileCount] + " AND `Month` = " + args[(i * 7) + 12 + fileCount] + " AND `Year` = " + args[(i * 7) + 13 + fileCount] + ") "
			") "
			") "
			") AND( "

			"(`Day` >= " + args[(i * 7) + 11 + fileCount] + " AND `Month` = " + args[(i * 7) + 12 + fileCount] + " AND `Year` = " + args[(i * 7) + 13 + fileCount] + ") OR "
			"(`Month` > " + args[(i * 7) + 12 + fileCount] + " AND `Year` = " + args[(i * 7) + 13 + fileCount] + ") OR "
			"(`Year` > " + args[(i * 7) + 13 + fileCount] + ") "

			") AND `Account ID` = (SELECT `Account ID` FROM 'Accounts' WHERE `Name`=\"" + args[(i * 7) + 9 + fileCount] + "\" "
			"); "
			"COMMIT; ";
	}

	for(int i = 0; i < invRows; i++)
	{
		int offset = ((rows - invRows) * 7) + 8 + fileCount + (i * 6);

		invMod +=	" BEGIN TRANSACTION; INSERT INTO 'Inventory Mod' VALUES (NULL, "
					"(SELECT `Entry ID` FROM 'Journal Row' WHERE "
					"`Journal ID` = (SELECT MAX(`Journal ID`) FROM 'Journal Entry') AND "
					"`Type` = 1 LIMIT 1 OFFSET " + std::to_string(i) + "), "
					"(SELECT `Product ID` From (SELECT * FROM 'Products' INNER JOIN 'Accounts' ON "
					"'Products'.`Account ID` = 'Accounts'.`Account ID`) WHERE Name = \"" + args[offset + 1] + "\"), "
					+ args[offset + 2] + ","
					"(SELECT `Retail Price` FROM 'Products' WHERE `Account ID` = "
					"(SELECT `Account ID` FROM 'Accounts' WHERE 'Accounts'.`Name`=\"" + args[offset + 1] + "\" "
					"))); COMMIT; ";

		if(std::stoi(args[offset + 2]) > 0)
		{
			invBatch += " BEGIN TRANSACTION; INSERT INTO 'Inventory Batch' VALUES "
						"(NULL, (SELECT `Entry ID` FROM 'Journal Row' WHERE "
						"`Journal ID` = (SELECT MAX(`Journal ID`) FROM 'Journal Entry') AND "
						"`Type` = 1 LIMIT 1 OFFSET " + std::to_string(i) + "), "
						"(SELECT `Product ID` From (SELECT * FROM 'Products' INNER JOIN 'Accounts' ON "
						"'Products'.`Account ID` = 'Accounts'.`Account ID`) WHERE Name = \"" + args[offset + 1] + "\"), "
						+ args[offset + 2] + "); "
						"COMMIT; ";
		}

		invCalc +=	" BEGIN TRANSACTION; UPDATE 'Products' SET `Current Stock` = `Current Stock` + " + args[offset + 2] + " WHERE "
					"`Account ID` = (SELECT `Account ID` FROM Accounts WHERE `Name` = \"" + args[offset + 1] + "\"); COMMIT; ";

		invLog +=	" BEGIN TRANSACTION; "
					"INSERT INTO 'Inventory Log'(`Year`,`Month`,`Day`,`Account ID`,`Stock`) "

					"SELECT " + args[offset + 5] + ", " + args[offset + 4] + ", " + args[offset + 3] + ", "

					"(SELECT `Account ID` FROM 'Accounts' WHERE 'Accounts'.`Name`= \"" + args[offset + 1] + "\"), "
					"(SELECT `Stock` FROM 'Inventory Log' "

					"WHERE( "

					"(`Day` < " + args[offset + 3] + " AND `Month` = " + args[offset + 4] + " AND `Year` = " + args[offset + 5] + ") OR "
					"(`Day` <= 31 AND `Month` = " + args[offset + 4] + "  AND `Year` = " + args[offset + 5] + ") OR "

					"(`Day` <= 31 AND `Month` < " + args[offset + 4] + " AND `Year` = " + args[offset + 5] + ") OR "

					"(`Day` <= 31 AND `Month` <= 12 AND `Year` < " + args[offset + 5] + ") "

					") AND "

					"(`Account ID` = (SELECT `Account ID` FROM 'Accounts' WHERE `Name`=\"" + args[offset + 1] + "\")) "
					"Order BY `Year` DESC, `Month` DESC, `Day` DESC LIMIT 1) "
					"WHERE NOT EXISTS "
					"( "
					"SELECT * FROM 'Inventory Log' WHERE `Account ID` = "
					"(SELECT `Account ID` FROM 'Accounts' WHERE "


					"`Name`=\"" + args[offset + 1] + "\" AND(`Day` = " + args[offset + 3] + " AND `Month` = " + args[offset + 4] + " AND `Year` = " + args[offset + 5] + ")) "


					"); "


					"COMMIT; "
					"BEGIN TRANSACTION; "
					"UPDATE 'Inventory Log' SET `Stock` = `Stock` + " + args[offset + 2] + " "

					"WHERE EXISTS "
					"( "
					"SELECT * FROM 'Inventory Log' WHERE `Account ID` = "
					"( "
					"SELECT `Account ID` FROM 'Accounts' WHERE `Name`=\"" + args[offset + 1] + "\" AND "
					"( "
					"(`Day` = " + args[offset + 3] + " AND `Month` = " + args[offset + 4] + " AND `Year` = " + args[offset + 5] + ") "
					") "
					") "
					") AND( "

					"(`Day` >= " + args[offset + 3] + " AND `Month` = " + args[offset + 4] + " AND `Year` = " + args[offset + 5] + ") OR "
					"(`Month` > " + args[offset + 4] + " AND `Year` = " + args[offset + 5] + ") OR "
					"(`Year` > " + args[offset + 5] + ") "

					") AND `Account ID` = (SELECT `Account ID` FROM 'Accounts' WHERE `Name`=\"" + args[offset + 1] + "\" "
					"); "
					"COMMIT; ";
	}

	return query.substr(0, query.size() - 2) + calc + log + invMod + invBatch + invCalc + invLog;
}

std::string SetEntryContinued(std::vector<std::string> args)
{
	std::string result = "";
	
	for(int i = 0; i < args.size() / 2; i++)
	{
		result +=	"BEGIN TRANSACTION; "
					"UPDATE 'Inventory Batch' SET `Left` = " + args[(i * 2) + 1] +
					" WHERE `Batch ID` = " + args[(i * 2)] + "; COMMIT;";
	}

	return result;
}

std::string SetBatchProcessed(std::vector<std::string> args)
{
	std::string result =	"BEGIN TRANSACTION;"
							"UPDATE 'POS Entry' SET `Processed` = 1 WHERE ";
	
	for(int i = 0; i < args.size(); i++)
	{
		result += "`Entry ID` = " + args[i] + " OR ";
	}

	result.replace(result.end() - 3, result.end(), "");

	result += "; COMMIT;";

	return result;
}

std::string SetARAP(std::vector<std::string> args)
{
	return	"INSERT INTO 'ARAP Status' VALUES ( "
			"(SELECT `Entry ID` FROM 'Journal Row' WHERE "
			"`Journal ID` = (SELECT MAX(`Journal ID`) FROM 'Journal Entry') "
			"LIMIT 1 OFFSET " + args[3] + "), "
			+ args[0] + " ,"
			+ args[1] + " ,"
			+ args[2] + " ,"
			"NULL)";
}

std::string GetEntry(std::vector<std::string> args)
{
	return "SELECT 'Journal Row'.`Type`, 'Accounts'.`Name`, 'Journal Row'.`Amount`, "
		"'Journal Row'.`Day`, 'Journal Row'.`Month`, 'Journal Row'.`Year`, 'Journal Row'.`Description`, "
		"'Inventory Mod'.`Product ID`, 'Inventory Mod'.`Quantity`, 'Inventory Mod'.`Retail Price`, 'Inventory Batch'.`Batch ID`"
		"FROM 'Journal Row' "
		"INNER JOIN "
		"'Accounts' ON 'Accounts'.`Account ID` = 'Journal Row'.`Account ID` "
		"LEFT JOIN "
		"'Inventory Mod' ON 'Inventory Mod'.`Entry ID` = 'Journal Row'.`Entry ID` "
		"LEFT JOIN "
		"'Inventory Batch' USING(`Entry ID`) "
		"WHERE `Journal ID` = " + args[0];
}

std::string GetEntryHTML(std::vector<std::string> args)
{
	return "SELECT `HTML` FROM 'Journal Entry' WHERE `Journal ID` = " + args[0];
}

std::string GetEntryHistory(std::vector<std::string> args)
{
	return "SELECT 'Journal Entry'.`Journal ID`, 'Ledgers'.`Name`, 'Journal Entry'.`Index`, 'Journal Entry'.`Journal Type`, "
		"'Journal Entry'.`Day`, 'Journal Entry'.`Month`, 'Journal Entry'.`Year`, 'Journal Entry'.`Description`, "
		"'Journal Entry Submission'.`Day`, 'Journal Entry Submission'.`Month`, 'Journal Entry Submission'.`Year`, "
		"'Journal Entry Submission'.`Hour`, 'Journal Entry Submission'.`Minute`, 'Journal Entry Submission'.`Second` FROM 'Journal Entry' "
		"INNER JOIN 'Ledgers' on 'Journal Entry'.`Ledger ID` = 'Ledgers'.`Ledger ID` "
		"INNER JOIN 'Journal Entry Submission' on 'Journal Entry'.`Journal ID` = 'Journal Entry Submission'.`Journal ID`";
}

std::string SetOrdersEntry(std::vector<std::string> args)
{
	time_t rawtime;
	time(&rawtime);
	struct tm* timeinfo;
	timeinfo = localtime(&rawtime);

	char day[3], month[3], year[5], sec[3], min[3], hour[3];

	itoa(timeinfo->tm_mday, day, 10);
	itoa(timeinfo->tm_mon + 1, month, 10);
	itoa(timeinfo->tm_year + 1900, year, 10);
	itoa(timeinfo->tm_min, min, 10);
	itoa(timeinfo->tm_hour, hour, 10);
	itoa(timeinfo->tm_sec, sec, 10);

	std::string res = 
		"BEGIN TRANSACTION; INSERT INTO 'Orders Entry' VALUES("
		"NULL,"
		"" + args[0] + ", "
		"IFNULL((SELECT MAX(`Index`) + 1 FROM 'Orders Entry' WHERE `isSale` = " + args[0] + "), 0), "
		"" + args[1] + "," //employee
		"" + args[2] + "," //driver
		"" + args[3] + "," //customer
		"" + args[4] + "," //supplier
		"\"" + args[5] + "\"," //add1
		"\"" + args[6] + "\"," //add2
		"\"" + args[7] + "\"," //phone
		"" + args[8] + "," //day
		"" + args[9] + "," //month
		"" + args[10] + "," //year
		"" + args[11] + "," //hour
		"" + args[12] + "," //min
		"" + args[13] + //receipt
		"" + args[14] + //subtotal
		"); COMMIT; ";

	res += "BEGIN TRANSACTION; INSERT INTO 'Orders Row' VALUES";
	int count = std::stoi(args[15]);

	for(int i = 0; i < count; i++)
	{
		res += 
			"(NULL, "
			"(SELECT MAX(`Order ID`) FROM 'Orders Entry'), "
			"(SELECT `Account ID` FROM 'Accounts' WHERE `Name` = \"" + args[16 + (i * 3)] + "\"), "
			+ args[17 + (i * 3)] + ", "
			+ args[18 + (i * 3)] + "),";
	}

	res.pop_back();

	res += "; COMMIT;";

	res += 
			"BEGIN TRANSACTION; INSERT INTO 'Order Entry Submission' VALUES "
			"((SELECT MAX (`Order ID`) FROM 'Orders Entry'), "; 

	res += day;
	res += ", ";
	res += month;
	res += ", ";
	res += year;
	res += ", ";
	res += hour;
	res += ", ";
	res += min;
	res += ", ";
	res += sec;
	res +=	"); ";

	res +=	"COMMIT;";

	return res;
}

std::string GetOrdersEntry(std::vector<std::string> args)
{
	return	"SELECT * FROM 'Orders Entry' "
			"INNER JOIN 'Order Entry Submission' ON 'Order Entry Submission'.`Order ID` = 'Orders Entry'.`Order ID`";
}

std::string GetOrderRows(std::vector<std::string> args)
{
	return	"SELECT * FROM 'Orders Row' "
			"INNER JOIN 'Accounts' ON 'Accounts'.`Account ID` = 'Orders Row'.`Account ID` "
			"WHERE 'Orders Row'.`Order ID` = " + args[0];
}

std::string GetARAP(std::vector<std::string> args)
{
	return	"SELECT `Entry ID`, `Day`, `Month`, `Year`, `Bill ID`, `Journal ID`, 'Accounts'.`Name`, `A` "
		"FROM (SELECT 'ARAP Status'.`Entry ID`, 'ARAP Status'.`Day`, "
		"'ARAP Status'.`Month`, 'ARAP Status'.`Year`, 'ARAP Status'.`Bill ID`, "
		"'Journal Row'.`Journal ID`, 'Journal Row'.`Account ID`, 'Journal Row'.`Amount` AS `A` FROM 'ARAP Status' "
		"INNER JOIN 'Journal Row' ON 'ARAP Status'.`Entry ID` = 'Journal Row'.`Entry ID`) "
		"INNER JOIN 'Accounts' USING(`Account ID`) ORDER BY Name";
}

std::string UpdateARAP(std::vector<std::string> args)
{
	return "UPDATE 'ARAP Status' SET `Bill ID` = "
		"(SELECT MAX(`Journal ID`) FROM 'Journal Entry') WHERE `Entry ID` = " + args[0];
}

std::string SetAccounts(std::vector<std::string> args)
{
	std::string query;

	if (args[3] == "0")
	{
		query = "BEGIN TRANSACTION; INSERT INTO 'Accounts' VALUES "
			"(NULL, "
			"(SELECT 'Account Types'.`Type ID` FROM 'Account Types' "
			"WHERE 'Account Types'.`Name` = \"" + args[0] + "\" AND 'Account Types'.`Parent ID` = "
			"(SELECT `ID` FROM 'Account Parent Types' WHERE `Category` = \"" + args[5] + "\")), "
			"\"" + args[1] + "\", " + args[2] + ", " + args[3] + ", \"" + args[4] + "\" ); COMMIT; ";
	}
	else
	{
		query = "BEGIN TRANSACTION; INSERT INTO 'Accounts' VALUES "
			"(NULL, "
			"(SELECT 'Account Types'.`Type ID` FROM 'Account Types' "
			"WHERE 'Account Types'.`Name` = \"" + args[0] + "\" AND 'Account Types'.`Parent ID` = "
			"(SELECT `ID` FROM 'Account Parent Types' WHERE `Category` = \"" + args[5] + "\")), "
			"\"" + args[1] + "\", " + args[2] + ", 1, \"" + args[4] + "\" ); COMMIT; ";
	}

	query +=	"BEGIN TRANSACTION; INSERT INTO 'Account Log' VALUES "
				"(NULL, 0, 0, 0, "
				"(SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name=\"" + args[1] + "\"), "
				+ args[2] + "); "
				"COMMIT; ";

	if(args[3] != "0")
	{
		query +=	"BEGIN TRANSACTION; INSERT INTO 'Accounts Contra' VALUES "
					"((SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name=\"" + args[1] + "\"),"
					"(SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name=\"" + args[3] + "\"));"
					"COMMIT;";
	}

	/*if(args.size() - 5 > 0)
	{
		query += "INSERT INTO 'Suppliers' VALUES (NULL,"
			"(SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name=\"" + args[1] + "\"), "
			"\"" + args[1] + "\")";
	}*/

	return query;
}

std::string GetAccounts(std::vector<std::string> args)
{
	return	"SELECT 'Account Types'.`Name`, 'Account Types'.`Type`, 'Accounts'.`Name`, 'Account Parent Types'.`Category`, "
			"'Accounts'.`Amount`, 'Accounts'.`Contra`, 'Accounts'.`Description`, "
			"'Account Log'.`Amount`, 'Inventory Log'.`Stock` "
			"FROM 'Accounts' INNER JOIN 'Account Types' ON "
			"Accounts.Type = 'Account Types'.`Type ID` "
			"INNER JOIN 'Account Parent Types' ON 'Account Parent Types'.`ID` = 'Account Types'.`Parent ID` "
			"LEFT JOIN 'Account Log' ON 'Accounts'.`Account ID` = 'Account Log'.`Account ID` AND "
			"'Account Log'.`Year` = 0 AND 'Account Log'.`Month` = 0 AND 'Account Log'.`Day` = 0 "
			"LEFT JOIN 'Inventory Log' ON 'Accounts'.`Account ID` = 'Inventory Log'.`Account ID` AND "
			"'Inventory Log'.`Year` = 0 AND 'Inventory Log'.`Month` = 0 AND 'Inventory Log'.`Day` = 0 "
			"ORDER BY 'Account Types'.Type";
}

std::string CloseAccount(std::vector<std::string> args)
{
	return	"UPDATE Accounts "
			"SET Name = (\"" + args[0] + " \" || (SELECT `Account ID` WHERE `Name` = \"" + args[0] + "\") || \"!\") "
			"WHERE Name = \"" + args[0] + "\"";
}

std::string GetAccountTypes(std::vector<std::string> args)
{
	return	"SELECT 'Account Types'.`Name`, 'Account Types'.`Type`, "
			"'Account Parent Types'.`Category` FROM 'Account Types' "
			"INNER JOIN 'Account Parent Types' ON 'Account Types'.`Parent ID` = 'Account Parent Types'.`ID` "
			"ORDER BY Type, Category, Name";
}

std::string GetAccountHistory(std::vector<std::string> args)
{
	//return	"SELECT 'Accounts'.Name , 'Account Log'.`Amount`, " 
	//		"'Account Log'.`Year`,'Account Log'.`Month`, Max('Account Log'.`Day`) From 'Account Log' "
	//		"INNER JOIN 'Accounts' on 'Account Log'.`Account ID` = 'Accounts'.`Account ID` "
	//		"GROUP BY 'Accounts'.Name, 'Account Log'.`Year`, 'Account Log'.`Month` "
	//		"ORDER BY 'Account Log'.`Year` ASC, 'Account Log'.`Month` ASC,'Account Log'.`Day` ASC";

	return "SELECT 'Accounts'.Name , 'Account Log'.`Amount`, 'Account Log'.`Year`,'Account Log'.`Month`, "
		   "'Account Log'.`Day` From 'Account Log' INNER JOIN "
		   "'Accounts' on 'Account Log'.`Account ID` = 'Accounts'.`Account ID`  ORDER BY "
           "'Account Log'.`Year` ASC, 'Account Log'.`Month` ASC,'Account Log'.`Day` ASC";

}

std::string ViewAccounts(std::vector<std::string> args)
{
	if(args[1] == "0")
	{
		return	"SELECT 'Journal Entry'.`Journal ID`, 'Journal Entry'.`Index`, 'Journal Row'.`Day`, "
				"'Journal Row'.`Month`, 'Journal Row'.`Year`, "
				"'Journal Row'.`Type`, 'Ledgers'.`Name`, 'Journal Row'.`Amount`, "
				"'Account Log'.`Amount`, 'Journal Row'.`Description`, 'Inventory Mod'.`Retail Price`, 'Journal Row'.`Entry ID` "
				"FROM 'Journal Row' "
				"INNER JOIN 'Journal Entry' ON 'Journal Entry'.`Journal ID` = 'Journal Row'.`Journal ID` "
				"INNER JOIN 'Ledgers' ON 'Journal Entry'.`Ledger ID` = 'Ledgers'.`Ledger ID` "
				"INNER JOIN 'Accounts' ON 'Accounts'.`Account ID` = 'Journal Row'.`Account ID` "
				"INNER JOIN 'Account Log' ON('Account Log'.`Account ID` = 'Journal Row'.`Account ID` AND "
				"'Journal Row'.`Day` = 'Account Log'.`Day` AND "
				"'Journal Row'.`Month` = 'Account Log'.`Month` AND "
				"'Journal Row'.`Year` = 'Account Log'.`Year`) "
				"LEFT JOIN "
				"'Inventory Mod' ON 'Inventory Mod'.`Entry ID` = 'Journal Row'.`Entry ID` "
				"WHERE 'Accounts'.`Name` = \"" + args[0] + "\" "
				"ORDER BY 'Journal Row'.`Year` ASC, 'Journal Row'.`Month` ASC, "
				"'Journal Row'.`Day` ASC";
	}
	else
	{
		return	"SELECT 'Journal Entry'.`Journal ID`, 'Journal Entry'.`Index`, 'Journal Row'.`Day`, "
				"'Journal Row'.`Month`, 'Journal Row'.`Year`, "
				"'Journal Row'.`Type`, 'Ledgers'.`Name`, 'Journal Row'.`Amount`, "
				"'Account Log'.`Amount`, 'Journal Row'.`Description`, 'Inventory Mod'.`Retail Price`, 'Journal Row'.`Entry ID` "
				"FROM 'Journal Row' "
				"INNER JOIN 'Journal Entry' ON 'Journal Entry'.`Journal ID` = 'Journal Row'.`Journal ID` "
				"INNER JOIN 'Ledgers' ON 'Journal Entry'.`Ledger ID` = 'Ledgers'.`Ledger ID` "
				"INNER JOIN 'Accounts' ON 'Accounts'.`Account ID` = 'Journal Row'.`Account ID` "
				"INNER JOIN 'Account Log' ON('Account Log'.`Account ID` = 'Journal Row'.`Account ID` AND "
				"'Journal Row'.`Day` = 'Account Log'.`Day` AND "
				"'Journal Row'.`Month` = 'Account Log'.`Month` AND "
				"'Journal Row'.`Year` = 'Account Log'.`Year`) "
				"LEFT JOIN "
				"'Inventory Mod' ON 'Inventory Mod'.`Entry ID` = 'Journal Row'.`Entry ID` "
				"WHERE 'Accounts'.`Name` = \"" + args[0] + "\" "
				"ORDER BY 'Journal Row'.`Year` ASC, 'Journal Row'.`Month` ASC, "
				"'Journal Row'.`Day` ASC";
	}

	/*"GROUP BY 'Journal Entry'.`Journal ID` ";
	"SELECT 'Journal Entry'.`Journal ID`, 'Journal Entry'.`Index`, 'Journal Row'.`Day`, "
	"'Journal Row'.`Month`, 'Journal Row'.`Year`, "
	"'Journal Row'.`Type`, 'Ledgers'.`Name`, "
	"'Journal Row'.`Amount`, 'Journal Row'.`Description`, 'Journal Row'.`Sale Price` "
	"FROM 'Journal Row' "
	"INNER JOIN 'Journal Entry' ON 'Journal Entry'.`Journal ID` = 'Journal Row'.`Journal ID` "
	"INNER JOIN 'Ledgers' ON 'Journal Entry'.`Ledger ID` = 'Ledgers'.`Ledger ID` "
	"INNER JOIN 'Accounts' ON 'Accounts'.`Account ID` = 'Journal Row'.`Account ID` "
	"WHERE 'Accounts'.`Name` = \"" + args[0] + "\" "
	"ORDER BY 'Journal Row'.`Year` DESC, 'Journal Row'.`Month` DESC, "
	"'Journal Row'.`Day` DESC, 'Journal Entry'.`Index` DESC";*/
}

std::string SetLedgers(std::vector<std::string> args)
{
	return "INSERT INTO 'Ledgers' (Name) VALUES (\"" + args[0] + "\")";
}

std::string GetLedgers(std::vector<std::string> args)
{
	return "SELECT * FROM 'Ledgers'";
}

std::string ViewLedgers(std::vector<std::string> args)
{
	return  "SELECT 'Journal Entry'.`Index`, 'Journal Row'.`Day`, 'Journal Row'.`Month`, "
			"'Journal Row'.`Year`, 'Journal Row'.`Type`, "
			"'Accounts'.`Name`, 'Journal Row'.`Description`, 'Journal Row'.`Amount`, "
			"'Inventory Mod'.`Quantity` FROM 'Journal Row' INNER JOIN 'Journal Entry' ON "
			"'Journal Entry'.`Journal ID` = 'Journal Row'.`Journal ID` "
			"AND 'Journal Entry'.`Ledger ID` = "
			"(SELECT 'Ledgers'.`Ledger ID` FROM 'Ledgers' WHERE 'Ledgers'.`Name` = \"" + args[0] + "\") "
			"INNER JOIN 'Accounts' "
			"ON 'Journal Row'.`Account ID` = 'Accounts'.`Account ID` "
			"LEFT JOIN 'Inventory Mod' ON 'Inventory Mod'.`Entry ID` = 'Journal Row'.`Entry ID` "
			"WHERE (('Journal Row'.`Year` = " + args[2] + "  AND 'Journal Row'.`Month` >= " + args[1] + ") OR ('Journal Row'.`Year` >= " + args[2] + ")) AND "
			"(('Journal Row'.`Year` = " + args[4] + " AND 'Journal Row'.`Month` <= " + args[3] + ") OR ('Journal Row'.`Year` < " + args[4] + ")) "
			"ORDER BY 'Journal Row'.`Journal ID` DESC";	
}

std::string SetInventory(std::vector<std::string> args)
{
	std::string result =
		"BEGIN TRANSACTION;"
		"INSERT INTO 'Accounts' VALUES (NULL, "
		"(SELECT `Type ID` FROM 'Account Types' WHERE `Name`=\"Inventory\"), "
		"\"" + args[0] + "\", " + args[1] + ", 0, \"" + args[2] + "\");"
		"COMMIT; "

		/*
		"BEGIN TRANSACTION;"
		"INSERT INTO 'Accounts' VALUES (NULL, "
		"(SELECT `Type ID` FROM 'Account Types' WHERE `Name`=\"Inventory\"), "
		"\"" + args[0] + " Discount\", 0, 1, NULL);"
		"COMMIT; "

		"BEGIN TRANSACTION;"
		"INSERT INTO 'Accounts' VALUES (NULL, "
		"(SELECT `Type ID` FROM 'Account Types' WHERE `Name`=\"Inventory\"), "
		"\"" + args[0] + " Tax\", 0, 1, NULL);"
		"COMMIT; "
		*/

		"BEGIN TRANSACTION; INSERT INTO 'Products' VALUES "
		"(NULL, "
		"(SELECT Categories.`Category ID` FROM Categories WHERE Name=\"" + args[3] + "\"), "
		"(SELECT `Account ID` FROM Accounts WHERE Name=\"" + args[0] + "\"), " +
		"(SELECT `Account ID` FROM Accounts WHERE Name=\"" + args[9] + "\"), " +
		"(SELECT `Account ID` FROM Accounts WHERE Name=\"" + args[10] + "\"), " +
		args[4] + ", " + args[5] + ", \"" + args[6] + "\", " + args[7] + ", \"" + args[8] + "\", " + args[11] + "); COMMIT;"

		"BEGIN TRANSACTION; "
		"INSERT INTO 'Inventory Batch' VALUES (NULL, "
		"NULL, "
		"(SELECT `Product ID` FROM Products WHERE `Account ID` = (SELECT `Account ID` FROM Accounts WHERE Name=\"" + args[0] + "\")), "
		+ args[5] + ");"
		"COMMIT; "

		"BEGIN TRANSACTION; "
		"INSERT OR IGNORE INTO 'Accounts' VALUES (NULL, "
		"(SELECT `Type` FROM 'Accounts' WHERE `Name`=\"" + args[9] + "\"), "
		"\"" + args[9] + " Tax\", 0, 1, \"Revenue Tax Account\"); COMMIT;"

		"BEGIN TRANSACTION; "
		"INSERT OR IGNORE INTO 'Accounts' VALUES (NULL, "
		"(SELECT `Type` FROM 'Accounts' WHERE `Name`=\"" + args[9] + "\"), "
		"\"" + args[9] + " Discount\", 0, 1, \"Revenue Discount Account\"); COMMIT;"

		"BEGIN TRANSACTION; "
		"INSERT INTO 'Account Log' VALUES "
		"(NULL, 0, 0, 0, "
		"(SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name=\"" + args[0] + "\"), "
		+ args[1] + "); "
		"COMMIT; "

		"BEGIN TRANSACTION; "
		"INSERT OR IGNORE INTO 'Account Log' (`Year`,`Month`,`Day`,`Account ID`,`Amount`) VALUES "
		"(0, 0, 0, "
			"(SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name = \"" + args[9] + " Tax\" AND NOT EXISTS "
			"(SELECT * FROM 'Account Log' WHERE `Account ID`= (SELECT `Account ID` From 'Accounts' WHERE `Name` = \"" + args[9] + " Tax\") "
				"AND `Year` = 0 AND `Month` = 0 AND `Day` = 0)) "
			", 0);"
		"COMMIT; "

		"BEGIN TRANSACTION; "
		"INSERT OR IGNORE INTO 'Account Log' (`Year`,`Month`,`Day`,`Account ID`,`Amount`) VALUES "
		"(0, 0, 0, "
			"(SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name = \"" + args[9] + " Discount\" AND NOT EXISTS "
			"(SELECT * FROM 'Account Log' WHERE `Account ID`= (SELECT `Account ID` From 'Accounts' WHERE `Name` = \"" + args[9] + " Discount\") "
				"AND `Year` = 0 AND `Month` = 0 AND `Day` = 0)) "
			", 0);"
		"COMMIT; "

		/*
		"BEGIN TRANSACTION; "
		"INSERT INTO 'Account Log' VALUES "
		"(NULL, 0, 0, 0, "
		"(SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name=\"" + args[0] + " Discount\"), "
		"0); "
		"COMMIT; "

		"BEGIN TRANSACTION; "
		"INSERT INTO 'Account Log' VALUES "
		"(NULL, 0, 0, 0, "
		"(SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name=\"" + args[0] + " Tax\"), "
		"0); "
		"COMMIT; "
		*/

		"BEGIN TRANSACTION; "
		"INSERT INTO 'Inventory Log' VALUES "
		"(NULL, 0, 0, 0, "
		"(SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name=\"" + args[0] + "\"), "
		+ args[5] + "); "
		"COMMIT; ";

	return result;
		
}

std::string EditInventory(std::vector<std::string> args)
{
	time_t rawtime;
	time(&rawtime);
	struct tm* timeinfo;
	timeinfo = localtime(&rawtime);

	char day[3], month[3], year[5];

	itoa(timeinfo->tm_mday, day, 10);
	itoa(timeinfo->tm_mon + 1, month, 10);
	itoa(timeinfo->tm_year + 1900, year, 10);

	return

	"BEGIN TRANSACTION; "
	"INSERT INTO 'Inventory Edit' VALUES( "
	"NULL , " 
	"(SELECT Products.'Product ID' FROM Products WHERE (Products.'Account ID' = (SELECT Accounts.'Account ID' FROM Accounts WHERE Accounts.'Name' = \"" + args[0] + "\"))), "
	"" + day + " , " + month + " , " + year + "," + args[3] + ",(SELECT `Retail Price` FROM Products WHERE `Account ID` = (SELECT Accounts.'Account ID' FROM Accounts WHERE Accounts.'Name' = \"" + args[0] + "\"))); "
	"COMMIT; UPDATE Products "
	"SET 'Retail Price' = " + args[1] + " , "
	"'Category ID' = (SELECT Categories.'Category ID' FROM Categories WHERE Categories.'Name' = \"" + args[2] + "\") "
	"WHERE Products.'Account ID' = (SELECT Accounts.'Account ID' FROM Accounts WHERE Accounts.'Name' = \"" + args[0] + "\");";

}

std::string GetEditHistory(std::vector<std::string> args)
{
	return

	"SELECT `Edit ID`, Accounts.`Name`, Day, Month, Year, `Retail Price` FROM "
	"(SELECT `Edit ID`, Products.`Account ID`, Day, Month, Year, 'Inventory Edit'.`Retail Price` FROM 'Inventory Edit' "
	"INNER JOIN Products USING(`Product ID`)) "
	"INNER JOIN Accounts USING(`Account ID`);";
}

std::string GetInventory(std::vector<std::string> args)
{
	return	"SELECT A, B, C, D, E, F, G, H, I, J, `Name`, L FROM(SELECT A, B, C, D, E, F, G, H, I, `Name` AS J, K, L FROM "
			"(SELECT Accounts.Name AS A, Categories.Name AS B, Accounts.Amount AS C, Accounts.Description AS D, "
			"Products.'Retail Price' AS E, Products.'Current Stock' AS F, Products.'Image' AS G, "
			"Products.`Cost Flow` AS H, Products.`Barcode` AS I, Products.`Revenue Account ID` AS J, Products.`COGS Account ID` AS K, Products.`Alert` AS L "
			"FROM 'Products' INNER JOIN Categories ON Products.'Category ID' = Categories.'Category ID' "
			"INNER JOIN Accounts ON Products.'Account ID' = Accounts.'Account ID' "
			"ORDER BY Accounts.Name) "
			"LEFT JOIN Accounts ON J = Accounts.'Account ID') "
			"LEFT JOIN Accounts ON K = Accounts.'Account ID'";
}

std::string GetInventoryServices(std::vector<std::string> args)
{
	return	"SELECT 'Accounts'.`Name`, ServiceName FROM "
			"(SELECT 'Products'.`Account ID`, 'Services'.`Name` AS ServiceName FROM 'Product Services' "
			"INNER JOIN 'Products' ON 'Product Services'.`Product ID` = 'Products'.`Product ID` "
			"INNER JOIN 'Services' USING(`Service ID`)) "
			"INNER JOIN 'Accounts' USING(`Account ID`)";
}

std::string GetInventoryHistory(std::vector<std::string> args)
{
	return	"SELECT `Name`, 'Journal Row'.`Amount`, 'Inventory Mod'.`Quantity`, `Retail Price`, 'Journal Row'.`Day`, 'Journal Row'.`Month`, 'Journal Row'.`Year`, 'Inventory Batch'.`Batch ID`, 'Inventory Batch'.`Left` FROM 'Inventory Mod' "
			"INNER JOIN 'Journal Row' "
			"USING(`Entry ID`) "
			"INNER JOIN 'Accounts' "
			"USING(`Account ID`) "
			"LEFT JOIN 'Inventory Batch' "
			"USING(`Entry ID`) "
			"UNION "
			"SELECT 'Accounts'.`Name`, NULL, NULL, NULL, NULL, NULL, NULL, `Batch ID`, `Left` FROM "
			"(SELECT 'Products'.`Account ID`, `Batch ID`, `Left` FROM 'Inventory Batch' "
			"INNER JOIN 'Products' USING(`Product ID`) "
			"WHERE `Entry ID` IS NULL) "
			"INNER JOIN 'Accounts' USING(`Account ID`) "
			"ORDER BY 'Journal Row'.`Year` ASC, 'Journal Row'.`Month` ASC, 'Journal Row'.`Day` ASC; ";
}

std::string GetInventoryGraph(std::vector<std::string> args)
{
	return	"SELECT 'Accounts'.Name , 'Inventory Log'.`Stock`, " 
			"'Inventory Log'.`Year`,'Inventory Log'.`Month`, Max('Inventory Log'.`Day`) From 'Inventory Log' "
			"INNER JOIN 'Accounts' on 'Inventory Log'.`Account ID` = 'Accounts'.`Account ID` "
			"GROUP BY 'Accounts'.Name, 'Inventory Log'.`Year`, 'Inventory Log'.`Month` "
			"ORDER BY 'Inventory Log'.`Year` ASC, 'Inventory Log'.`Month` ASC,'Inventory Log'.`Day` ASC";
}

std::string SetServices(std::vector<std::string> args)
{
	std::string services = "";

	if(args[0] == "Service" || args[0] == "Service Bundle")
	{
		services += "BEGIN TRANSACTION; INSERT INTO 'Accounts' VALUES "
			"(NULL, "
			"(SELECT 'Account Types'.`Type ID` FROM 'Account Types' "
			"WHERE 'Account Types'.`Name` = \"" + args[0] + "\"), "
			"\"" + args[1] + "\", 0 , 0, \"" + args[3] + "\" ); COMMIT; ";

		services += "BEGIN TRANSACTION; INSERT INTO 'Account Log' VALUES "
			"(NULL, 0, 0, 0, "
			"(SELECT Accounts.`Account ID` FROM 'Accounts' WHERE Name=\"" + args[1] + "\"), "
			"0); "
		"COMMIT; ";
	}

	services += "BEGIN TRANSACTION; INSERT INTO 'Services' VALUES "
		"(NULL, "
		"(SELECT 'Accounts'.`Account ID` FROM 'Accounts' WHERE Name=\"" + args[1] + "\"),"
		"(SELECT 'Service Type'.`Type ID` FROM 'Service Type' WHERE Name=\"" + args[0] + "\"), "
		"\"" + args[1] + "\"," + args[2] + ",\"" + args[3] + "\", 0); COMMIT;";

	std::string products;

	if(args.size() - 4 > 0)
		products = " INSERT INTO 'Product in Services' VALUES ";

	for(int i = 0; i < (args.size() - 4); i+=2)
	{
		products += 
			"((SELECT `Service ID` FROM 'Services' WHERE Name=\"" + args[1] + "\"), "
			"(SELECT `Product ID` FROM Products WHERE `Account ID` = (SELECT `Account ID` FROM 'Accounts' WHERE Name =\"" + args[4 + (i) + 0] + "\"))," + args[4 + (i) + 1] + "), ";
	}
	
	return services + products.substr(0, products.size() - 2);
}

std::string CloseServiceAccount(std::vector<std::string> args)
{
	return	"BEGIN TRANSACTION; "
			"UPDATE Accounts "
			"SET Name = (\"" + args[0] + " \" || (SELECT `Account ID` WHERE `Name` = \"" + args[0] + "\") || \"!\") "
			"WHERE Name = \"" + args[0] + "\" "
			";COMMIT; "
			"UPDATE Services "
			"SET Name = (\"" + args[0] + " \" || (SELECT `Service ID` WHERE `Name` = \"" + args[0] + "\") || \"!\") "
			"WHERE Name = \"" + args[0] + "\"";
}

std::string CloseService(std::vector<std::string> args)
{
	return	"UPDATE Services "
			"SET Closed = 1 "
			"WHERE Name = \"" + args[0] + "\"";
}

std::string GetServices(std::vector<std::string> args)
{
	return "SELECT 'Services'.`Service ID`, "
		"(SELECT 'Service Type'.`Name` FROM 'Service Type' WHERE 'Service Type'.`Type ID` = 'Services'.`Category ID`), "
		"'Services'.`Name`, 'Services'.`Retail Price`, "
		"'Services'.`Description`, 'Services'.`Closed` FROM 'Services'";
}

std::string GetProductsInServices(std::vector<std::string> args)
{
	return	"SELECT 'Services'.`Service ID` , "
			"(SELECT Name From Accounts WHERE `Account ID` = (SELECT 'Products'.`Account ID` "
			"FROM 'Products' WHERE 'Products'.`Product ID` = 'Product in Services'.`Product ID`)), " 
			"'Product in Services'.`Quantity` FROM 'Services' "
			"LEFT JOIN 'Product in Services' USING(`Service ID`) Order By `Service ID`";
}

std::string AddService(std::vector<std::string> args)
{
	std::string res = "INSERT INTO 'Product Services' VALUES ";
	int invCount = std::stoi(args[0]);

	for(int i = 1; i <= invCount; i++) //for each inv
	{
		for (int j = invCount + 1; j < args.size(); j++) //for each service
		{
			//apply service to each inv
			res += "((SELECT `Product ID` FROM 'Products' WHERE `Account ID` = "
					"(SELECT `Account ID` from 'Accounts' WHERE `Name` = \"" + args[i] + "\")), "
					"(SELECT `Service ID` FROM 'Services' WHERE `Name` = \"" + args[j] + "\")), ";
		}
	}
	res.replace(res.end() - 2, res.end(), ";");
	return res;
}

std::string RemoveService(std::vector<std::string> args)
{
	std::string res = "DELETE FROM 'Product Services' WHERE ";
	int invCount = std::stoi(args[0]);

	for (int i = 1; i <= invCount; i++) //for each inv
	{
		for (int j = invCount + 1; j < args.size(); j++) //for each service
		{
			//apply service to each inv
			res += "(`Product ID` = (SELECT `Product ID` FROM 'Products' WHERE `Account ID` = "
				"(SELECT `Account ID` from 'Accounts' WHERE `Name` = \"" + args[i] + "\")) AND "
				"`Service ID` = (SELECT `Service ID` FROM 'Services' WHERE `Name` = \"" + args[j] + "\")) OR ";
		}
	}
	res.replace(res.end() - 4, res.end(), ";");
	return res;
}

std::string SetCategories(std::vector<std::string> args)
{
	return "INSERT INTO 'Categories' VALUES (NULL, \"" + args[0] + "\")";
}

std::string GetCategories(std::vector<std::string> args)
{
	return "SELECT Name FROM Categories";
}

std::string SetSupplier(std::vector<std::string> args)
{
	return "INSERT INTO Suppliers VALUES(NULL,\"" + args[0] + "\","
		"\"" + args[1] + "\","
		"\"" + args[2] + "\","
		"\"" + args[3] + "\","
		"\"" + args[4] + "\","
		"\"" + args[5] + "\")";
}

std::string GetSuppliers(std::vector<std::string> args)
{
	return "SELECT * FROM Suppliers";
}

std::string ReportIncome(std::vector<std::string> args)
{
	return	
			"SELECT Type, Name, SUM(Amount) FROM "
			"(SELECT Type, Name, MIN(Day || ' ' || Month || ' ' || Year), -Amount AS Amount "
			"FROM(SELECT 'Accounts'.`Account ID`, 'Account Types'.Name as Type, 'Accounts'.`Name`, "
			"'Account Log'.Day, 'Account Log'.Month, 'Account Log'.Year, 'Account Log'.Amount FROM 'Accounts' INNER JOIN "
			"'Account Types' ON 'Accounts'.Type = 'Account Types'.`Type ID` INNER JOIN "
			"'Account Log' ON 'Accounts'.`Account ID` = 'Account Log'.`Account ID`) "
			"WHERE Month <= " + args[0] + " AND Year <= " + args[1] + " "
			"GROUP BY Name "
			"UNION "
			"SELECT Type, Name, MAX(Day || ' ' || Month || ' ' || Year), Amount "
			"FROM(SELECT 'Accounts'.`Account ID`, 'Account Types'.Name as Type, 'Accounts'.`Name`, "
			"'Account Log'.Day, 'Account Log'.Month, 'Account Log'.Year, 'Account Log'.Amount FROM 'Accounts' INNER JOIN "
			"'Account Types' ON 'Accounts'.Type = 'Account Types'.`Type ID` INNER JOIN "
			"'Account Log' ON 'Accounts'.`Account ID` = 'Account Log'.`Account ID`) "
			"WHERE Month <= " + args[2] + " AND Year <= " + args[3] + " "
			"GROUP BY Name) "
			"GROUP BY Name";
}

std::string ReportStartCash(std::vector<std::string> args)
{
	return	"SELECT PType, Type, Name, Amount FROM "
			"(SELECT PType, Type, Name, MAX(Day || ' ' || Month || ' ' || Year), Amount "
			"FROM(SELECT 'Accounts'.`Account ID`, 'Account Types'.Name as Type, 'Account Parent Types'.`Category` as PType, 'Accounts'.`Name`, "
			"'Account Log'.Day, 'Account Log'.Month, 'Account Log'.Year, 'Account Log'.Amount FROM 'Accounts' INNER JOIN "
			"'Account Types' ON 'Accounts'.Type = 'Account Types'.`Type ID` INNER JOIN "
			"'Account Parent Types' ON 'Account Types'.`Parent ID` = 'Account Parent Types'.`ID` INNER JOIN "
			"'Account Log' ON 'Accounts'.`Account ID` = 'Account Log'.`Account ID`) "
			"WHERE Month <= " + args[0] + " AND Year <= " + args[1] + " "
			"GROUP BY Name) ORDER BY PType, Type";
}

std::string ReportEndCash(std::vector<std::string> args)
{
	return	"SELECT PType, Type, Name, Amount FROM "
			"(SELECT PType, Type, Name, MAX(Day || ' ' || Month || ' ' || Year), Amount "
			"FROM(SELECT 'Accounts'.`Account ID`, 'Account Types'.Name as Type, 'Account Parent Types'.`Category` as PType, 'Accounts'.`Name`, "
			"'Account Log'.Day, 'Account Log'.Month, 'Account Log'.Year, 'Account Log'.Amount FROM 'Accounts' INNER JOIN "
			"'Account Types' ON 'Accounts'.Type = 'Account Types'.`Type ID` INNER JOIN "
			"'Account Parent Types' ON 'Account Types'.`Parent ID` = 'Account Parent Types'.`ID` INNER JOIN "
			"'Account Log' ON 'Accounts'.`Account ID` = 'Account Log'.`Account ID`) "
			"WHERE Month <= " + args[0] + " AND Year <= " + args[1] + " "
			"GROUP BY Name) ORDER BY PType, Type";
}

std::string SetCustomers(std::vector<std::string> args)
{
	return	"BEGIN TRANSACTION;"
		"INSERT INTO 'Customers' VALUES (NULL, "
		"\"" + args[0] + "\", "
		"\"" + args[1] + "\", "
		"\"" + args[2] + "\", "
		"\"" + args[3] + "\", "
		"\"" + args[4] + "\", "
		"\"" + args[5] + "\", "
		"\"" + args[6] + "\", "
		"\"" + args[7] + "\" );"
		"COMMIT;";
}

std::string GetCustomers(std::vector<std::string> args)
{
	return	"SELECT * FROM 'Customers'";
}

std::string GetSimpCustomers(std::vector<std::string> args)
{
	return	"SELECT `Customer ID`, `FirstName`, `LastName`, `Phone` FROM 'Customers'";
}

std::string SetSalesEntry(std::vector<std::string> args)
{
	//auto time = std::chrono::system_clock::now();
	time_t rawtime;
	time (&rawtime);
	struct tm* timeinfo;
	timeinfo = localtime(&rawtime);

	char day[3], month[3], year[5], min[3], hour[3];

	itoa(timeinfo->tm_mday, day, 10);
	itoa(timeinfo->tm_mon + 1, month, 10);
	itoa(timeinfo->tm_year + 1900, year, 10);
	itoa(timeinfo->tm_min, min, 10);
	itoa(timeinfo->tm_hour, hour, 10);

	std::string entry = "BEGIN TRANSACTION;"
		"INSERT INTO 'POS Entry' VALUES (NULL, 0, "
		+ args[0] + ", "
		+ ((args[1] == "-1") ? "(SELECT MAX(`Customer ID`) FROM 'Customers')" : args[1]) + ", "
		+ day + ", "
		+ month + ", "
		+ year + ", "
		+ min + ", "
		+ hour + ", "
		+ args[2] + ", "
		"NULL, "
		+ args[3] + ");"
		"COMMIT;";

	std::string rows = "BEGIN TRANSACTION;";

	for (int i = 0; i < (args.size() - 4) / 4; i++)
	{
		std::string c = args[(4 * i) + 4];

		if(c == "0")
		{
			rows += "INSERT INTO 'POS Row' VALUES (NULL, "
				"(SELECT MAX(`Entry ID`) FROM 'POS Entry'), "
				"(SELECT `Product ID` FROM 'Products' WHERE `Account ID` = "
				"(SELECT `Account ID` FROM 'Accounts' WHERE `Name` = \"" + args[(4 * i) + 5] + "\")), "
				"NULL,"
				+ args[(4 * i) + 6] + ", "
				+ args[(4 * i) + 7] + ");";
		}
		else
		{
			rows += "INSERT INTO 'POS Row' VALUES (NULL, "
				"(SELECT MAX(`Entry ID`) FROM 'POS Entry'), "
				"NULL,"
				"(SELECT `Service ID` FROM 'Services' WHERE `Name` = \"" + args[(4 * i) + 5] + "\"), "
				+ args[(4 * i) + 6] + ", "
				+ args[(4 * i) + 7] + ");";
		}
	}

	rows +=	"COMMIT; ";

	//std::string account = "BEGIN TRANSACTION; UPDATE 'Accounts' SET `Amount` = `Amount` + " + args[2] + " WHERE `Name` = 'POS'; COMMIT;";

	return entry + rows;// + account;
}

std::string GetSalesEntry(std::vector<std::string> args)
{
	std::string ids;

	for(int i = 0; i < args.size(); i++)
	{
		ids += args[i] + ',';
	}

	ids.replace(ids.end() - 1, ids.end(), "");

	return "SELECT 'POS Row'.`Row ID`, 'Accounts'.`Name`, 'Services'.`Name`,"
		"'POS Row'.`Retail Price`, SUM('POS Row'.`Quantity`) "
		"FROM 'POS Row' "
		"LEFT JOIN 'Products' ON 'POS Row'.`Product ID` = 'Products'.`Product ID` "
		"LEFT JOIN 'Accounts' ON 'Accounts'.`Account ID` = 'Products'.`Account ID` "
		"LEFT JOIN 'Services' ON 'Services'.`Service ID` = 'POS Row'.`Service ID` "
		"WHERE `Entry ID` IN ( " + ids + ") "
		"GROUP BY 'Accounts'.`Name`, 'POS Row'.`Retail Price` "
		"ORDER BY 'Accounts'.`Name` ASC";
}

std::string RevertSalesEntry(std::vector<std::string> args)
{
	return	"UPDATE 'POS Entry' SET `Processed` = -1 "
			"WHERE `Entry ID` = " + args[0];
}

std::string GetSalesEntryHistory(std::vector<std::string> args)
{
	return "SELECT 'POS Entry'.`Entry ID`, 'POS Entry'.`Processed`, 'POS Entry'.`Employee ID`, "
		"'POS Entry'.`Subtotal`, 'POS Entry'.`Day`, 'POS Entry'.`Month`, 'POS Entry'.`Year`, "
		"'POS Entry'.`Hour`, 'POS Entry'.`Minute`, 'Customers'.`FirstName`, "
		"'Customers'.`LastName`,  'Customers'.`Phone`, 'POS Entry'.`Batch ID`, 'POS Entry'.`Tax`"
		"FROM 'POS Entry' "
		"INNER JOIN `Customers` USING('Customer ID')";
}

std::string GetTransactionHistory(std::vector<std::string> args)
{
	return  "SELECT `Entry ID`, `Subtotal`, `Day`, `Month`, `Year` FROM 'POS Entry' "
			"WHERE `Customer ID` = " + args[0];
}

std::string GetCustomerOrderHistory(std::vector<std::string> args)
{
	return	"SELECT * FROM 'Orders Entry' "
			"INNER JOIN 'Order Entry Submission' ON 'Order Entry Submission'.`Order ID` = 'Orders Entry'.`Order ID` "
			"WHERE 'Orders Entry'.`Customer ID` = " + args[0];
}

std::string GetSupplierOrderHistory(std::vector<std::string> args)
{
	return	"SELECT * FROM 'Orders Entry' "
			"INNER JOIN 'Order Entry Submission' ON 'Order Entry Submission'.`Order ID` = 'Orders Entry'.`Order ID` "
			"WHERE 'Orders Entry'.`Supplier ID` = " + args[0];
}

std::string GetReceipt(std::vector<std::string> args)
{
	return "SELECT 'Accounts'.`Name`, 'Services'.`Name`, 'POS Row'.`Retail Price`, 'POS Row'.`Quantity` FROM 'POS Row' "
		"LEFT JOIN 'Products' ON 'POS Row'.`Product ID` = 'Products'.`Product ID` "
		"LEFT JOIN 'Accounts' ON 'Accounts'.`Account ID` = 'Products'.`Account ID` "
		"LEFT JOIN 'Services' ON 'Services'.`Service ID` = 'POS Row'.`Service ID` "
		"WHERE `Entry ID` = " + args[0];
}

std::string SetEmployee(std::vector<std::string> args)
{
	return "INSERT INTO 'Employees' VALUES(NULL, "
		"\"" + args[0] + "\", "
		"\"" + args[1] + "\", "
		+ args[2] + ", "
		"\"" + args[3] + "\", "
		"\"" + args[4] + "\", "
		"\"" + args[5] + "\", "
		"\"" + args[6] + "\", "
		+ args[7] + ", "
		+ args[8] + ")";
}

std::string GetEmployees(std::vector<std::string> args)
{
	return "SELECT 'Employees'.`Employee ID`, 'Employees'.`First Name`, 'Employees'.`Last Name`, "
		"'Employees'.`Phone Number`, 'Employees'.`Address`, 'Employees'.`Email`, "
		"'Employees'.`Personal ID`, 'Employees'.`Date Joined`, 'Enrollment'.`Enrollment Status`, "
		"'Jobs'.`Job Title` "
		"FROM 'Employees' "
		"INNER JOIN 'Enrollment' ON 'Employees'.`Enrollment ID` = 'Enrollment'.`Enrollment ID` "
		"INNER JOIN 'Jobs' ON 'Jobs'.`Job ID` = 'Employees'.`Job ID`";
}

std::string SetJobs(std::vector<std::string> args)
{
	return "INSERT INTO 'Jobs' VALUES(NULL, "
		"\"" + args[0] + "\");";
}

std::string GetJobs(std::vector<std::string> args)
{
	return "SELECT `Job Title` FROM 'Jobs'";
}

std::string BatchExport(std::vector<std::string> args)
{
	std::string ret = 
		"BEGIN TRANSACTION; INSERT INTO 'POS Batch' VALUES (NULL); COMMIT;"
		"BEGIN TRANSACTION; UPDATE 'POS Entry' SET `Batch ID` = "
		"(SELECT MAX(`Batch ID`) FROM 'POS Batch') WHERE ";

	for(int i = 0; i < args.size(); i++)
	{
		ret +=	"`Entry ID` = " + args[i] + " OR ";
	}

	return ret.substr(0, ret.size() - 4) + "; COMMIT;";
}

std::string BatchImport(std::vector<std::string> args)
{
	return "SELECT `Job Title` FROM 'Jobs'";
}

namespace
{
	std::map<std::string, FnPtr> myMap =
	{
		{"Authenticate", Authenticate},
		{SET_ENTRY, SetEntry},
		{SET_ENTRY_CONTINUED, SetEntryContinued},
		{SET_BATCH_PROCESSED, SetBatchProcessed},
		{SET_ARAP, SetARAP},
		{GET_ENTRY, GetEntry},
		{GET_ENTRY_HTML, GetEntryHTML},
		{GET_ENTRY_HISTORY, GetEntryHistory},
		{SET_ORDERS_ENTRY, SetOrdersEntry},
		{GET_ORDERS_ENTRY, GetOrdersEntry},
		{GET_ORDERS_ROWS, GetOrderRows},
		{GET_ARAP, GetARAP},
		{UPDATE_ARAP, UpdateARAP},
		{SET_ACCOUNTS, SetAccounts},
        {GET_ACCOUNTS, GetAccounts},
        {CLOSE_ACCOUNTS, CloseAccount},
        {GET_ACCOUNTTYPES, GetAccountTypes},
        {GET_ACCOUNTHISTORY, GetAccountHistory},
        {VIEW_ACCOUNTS, ViewAccounts},
		{SET_LEDGERS, SetLedgers},
		{GET_LEDGERS, GetLedgers},
		{VIEW_LEDGERS, ViewLedgers},
		{SET_INVENTORY, SetInventory},
		{GET_INVENTORY, GetInventory},
		{EDIT_INVENTORY, EditInventory},
		{GET_EDIT_HISTORY, GetEditHistory},
		{GET_INVENTORY_SERVICES, GetInventoryServices},
		{GET_INVENTORY_HISTORY, GetInventoryHistory},
		{GET_INVENTORY_GRAPH, GetInventoryGraph},
		{SET_SERVICES, SetServices},
		{CLOSE_SERVICE, CloseService},
		{CLOSE_SERVICE_ACCOUNT, CloseServiceAccount},
		{GET_SERVICES, GetServices},
		{ADD_SERVICE, AddService},
		{REMOVE_SERVICE, RemoveService},
		{GET_PRODUCT_IN_SERVICES, GetProductsInServices},	
		{SET_CATEGORIES, SetCategories},
		{GET_CATEGORIES, GetCategories},
		{GET_SUPPLIERS, GetSuppliers},
		{SET_SUPPLIERS, SetSupplier},
		{REPORT_INCOME, ReportIncome},
		{REPORT_START_CASH, ReportStartCash},
		{REPORT_END_CASH, ReportEndCash},
		{SET_CUSTOMERS, SetCustomers},
		{GET_CUSTOMERS, GetCustomers},
		{GET_SIMP_CUSTOMERS, GetSimpCustomers},
		{SET_SALES_ENTRY, SetSalesEntry},
		{GET_SALES_ENTRY, GetSalesEntry},
		{REVERT_SALES_ENTRY, RevertSalesEntry},
		{GET_SALES_ENTRY_HISTORY, GetSalesEntryHistory},
		{GET_TRANSACTION_HISTORY, GetTransactionHistory},
		{GET_CUSTOMER_ORDER_HISTORY, GetCustomerOrderHistory},
		{GET_SUPPLIER_ORDER_HISTORY, GetSupplierOrderHistory},
		{GET_RECEIPT, GetReceipt},
		{SET_EMPLOYEE, SetEmployee},
		{GET_EMPLOYEES, GetEmployees},
		{SET_JOBS, SetJobs},
		{GET_JOBS, GetJobs},
		{BATCH_EXPORT, BatchExport},
		{BATCH_IMPORT, BatchImport}
	};
}

Sqlite::Sqlite()
{	
	DB_Path = PathRes::GetAbsolutePath("Jordan\\DBMS\\base - Copy.db");
	
	dailyBackup(DB_Path);

	int r = sqlite3_open(DB_Path.c_str(), &db);

	if (r)
	{
		std::cout << "Connection to Database Failed." << std::endl;
	}
}

Sqlite::~Sqlite()
{
	sqlite3_close(db);
}

static int callback(void* NotUsed, int argc, char** argv, char** azColName) 
{
	std::string temp;
	int i;

	for (i = 0; i < argc; i++)
	{
		temp = argv[i] ? argv[i] : "NULL";
		data.insert(data.end(), temp);
	}

	result = result + temp;
	
	return 0;
}

void Sqlite::ProcessQuery(std::string query, std::vector<std::string> args)
{
	data.clear();

	std::ofstream myfile;
	myfile.open(LOG_Path, std::ofstream::app);
	myfile << query.c_str() << std::endl;

	query = myMap[query](args);

	myfile << query << std::endl;

	std::string q = query;

	char* zErrMsg = 0;
	int r = sqlite3_exec(db, q.c_str(), callback, 0, &zErrMsg);

	//step.. finalize
	if (r)
	{
		std::cout << "SQL Error: " << zErrMsg << std::endl;
		myfile << "SQL Error: " << zErrMsg << std::endl;
	}

	myfile << std::endl;
	myfile.close();

	if(data.empty())
		data.insert(data.end(), "Success");
}

bool Sqlite::hasResult()
{
	return !data.empty();
}

std::list<std::string> Sqlite::getResult()
{
	std::list<std::string> tmp = data;
	return tmp;
}

std::list<std::string> Sqlite::getResultClear()
{
	std::list<std::string> tmp = data;
	data.clear();
	return tmp;
}

std::list<std::string> Sqlite::clearResults()
{
	std::list<std::string> tmp = data;
	data.clear();
	data.push_back("Success");
	return tmp;
}

void Sqlite::dailyBackup(std::string _path)
{
	time_t rawtime;
	time(&rawtime);
	struct tm* timeinfo;
	timeinfo = localtime(&rawtime);

	char day[3], month[3], year[5];

	itoa(timeinfo->tm_mday, day, 10);
	itoa(timeinfo->tm_mon + 1, month, 10);
	itoa(timeinfo->tm_year + 1900, year, 10);
	
	LOG_Path = _path + " ";
	LOG_Path.append(day);
	LOG_Path += "-";
	LOG_Path.append(month);
	LOG_Path += "-";
	LOG_Path.append(year);
	LOG_Path += ".log";

	std::ofstream myfile;
	myfile.open(_path + " BACKUP " + day + "-" + month + "-" + year, std::ofstream::in);

	if(!myfile.is_open())
	{
		myfile = std::ofstream(_path + " BACKUP " + day + "-" + month + "-" + year, std::ios::out | std::ios::binary);

		//Copy Data over
		std::ifstream src;
		src.open(_path, std::ios::in | std::ios::binary);

		myfile << src.rdbuf();

		src.close();
	}

	myfile.close();

	myfile.open(LOG_Path, std::ofstream::in);

	if (!myfile.is_open())
	{
		myfile = std::ofstream(LOG_Path);
	}

	myfile.close();
}