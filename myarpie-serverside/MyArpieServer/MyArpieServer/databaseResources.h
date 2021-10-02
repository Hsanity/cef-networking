#pragma once
#include "sqlite/sqlite3.h"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <string>
#include <list>
#include <chrono>
#include <ctime>
#include <time.h>

#define SET_ENTRY "SetEntry"
#define SET_ENTRY_CONTINUED "SetEntryContinued"
#define SET_BATCH_PROCESSED "SetBatchProcessed"
#define SET_ARAP "SetARAP"
#define GET_ENTRY "GetEntry"
#define GET_ENTRY_HTML "GetEntryHTML"
#define GET_ENTRY_HISTORY "GetEntryHistory"
#define SET_ORDERS_ENTRY "SetOrdersEntry"
#define GET_ORDERS_ENTRY "GetOrdersEntry"
#define GET_ORDERS_ROWS "GetOrderRows"
#define GET_ARAP "GetARAP"
#define UPDATE_ARAP "UpdateARAP"
#define SET_ACCOUNTS "SetAccounts"
#define GET_ACCOUNTS "GetAccounts"
#define CLOSE_ACCOUNTS "CloseAccount"
#define GET_ACCOUNTTYPES "GetAccountTypes"
#define GET_ACCOUNTHISTORY "GetAccountHistory"
#define VIEW_ACCOUNTS "ViewAccounts"
#define SET_LEDGERS "SetLedgers"
#define GET_LEDGERS "GetLedgers"
#define VIEW_LEDGERS "ViewLedgers"
#define SET_INVENTORY "SetInventory"
#define GET_INVENTORY "GetInventory"
#define EDIT_INVENTORY "EditInventory"
#define GET_EDIT_HISTORY "GetEditHistory"
#define GET_INVENTORY_SERVICES "GetInventoryServices"
#define GET_INVENTORY_HISTORY "GetInventoryHistory"
#define GET_INVENTORY_GRAPH "GetInventoryGraph"
#define SET_SERVICES "SetServices"
#define CLOSE_SERVICE "CloseService"
#define CLOSE_SERVICE_ACCOUNT "CloseServiceAccount"
#define GET_SERVICES "GetServices"
#define GET_PRODUCT_IN_SERVICES "GetProductsInServices"
#define ADD_SERVICE "AddService"
#define REMOVE_SERVICE "RemoveService"
#define SET_CATEGORIES "SetCategories"
#define GET_CATEGORIES "GetCategories"
#define GET_SUPPLIERS "GetSuppliers"
#define SET_SUPPLIERS "SetSupplier"
#define SET_CUSTOMERS "SetCustomers"
#define GET_CUSTOMERS "GetCustomers"
#define GET_SIMP_CUSTOMERS "GetSimpCustomers"
#define SET_SALES_ENTRY "SetSalesEntry"
#define GET_SALES_ENTRY "GetSalesEntry"
#define REVERT_SALES_ENTRY "RevertSalesEntry"
#define GET_SALES_ENTRY_HISTORY "GetSalesEntryHistory"
#define GET_TRANSACTION_HISTORY "GetTransactionHistory"
#define GET_CUSTOMER_ORDER_HISTORY "GetCustomerOrderHistory"
#define GET_SUPPLIER_ORDER_HISTORY "GetSupplierOrderHistory"
#define GET_RECEIPT "GetReceipt"
#define SET_EMPLOYEE "SetEmployee"
#define GET_EMPLOYEES "GetEmployees"
#define SET_JOBS "SetJobs"
#define GET_JOBS "GetJobs"
#define BATCH_EXPORT "BatchExport"
#define BATCH_IMPORT "BatchImport"
#define REPORT_INCOME "ReportIncome"
#define REPORT_START_CASH "ReportStartCash"
#define REPORT_END_CASH "ReportEndCash"

enum class Resource_Type
{
    Type_Not_Found,
    ACC_R,
    ACC_W,
};

inline Resource_Type GetResourceType(const char* resource)
{
    if(strcmp(resource, SET_ENTRY) == 0) return Resource_Type::ACC_W;
    if(strcmp(resource, SET_ENTRY_CONTINUED) == 0) return Resource_Type::ACC_W;
    if(strcmp(resource, SET_BATCH_PROCESSED) == 0) return Resource_Type::ACC_W;
    if(strcmp(resource, SET_ARAP) == 0) return Resource_Type::ACC_W;
    if(strcmp(resource, GET_ENTRY) == 0) return Resource_Type::ACC_R;
    if(strcmp(resource, GET_ENTRY_HTML) == 0) return Resource_Type::ACC_R;
    if(strcmp(resource, GET_ENTRY_HISTORY) == 0) return Resource_Type::ACC_R;

    if(strcmp(resource, SET_ORDERS_ENTRY) == 0) return Resource_Type::ACC_W;
    if(strcmp(resource, GET_ORDERS_ENTRY) == 0) return Resource_Type::ACC_R;
    if(strcmp(resource, GET_ORDERS_ROWS) == 0) return Resource_Type::ACC_R;
	if(strcmp(resource, GET_ARAP) == 0) return Resource_Type::ACC_R;
	if(strcmp(resource, UPDATE_ARAP) == 0) return Resource_Type::ACC_W;

    if(strcmp(resource, SET_ACCOUNTS) == 0) return Resource_Type::ACC_W;
    if(strcmp(resource, GET_ACCOUNTS) == 0) return Resource_Type::ACC_R;
    if(strcmp(resource, CLOSE_ACCOUNTS) == 0) return Resource_Type::ACC_W;
    if(strcmp(resource, GET_ACCOUNTTYPES) == 0) return Resource_Type::ACC_R;
    if(strcmp(resource, GET_ACCOUNTHISTORY) == 0) return Resource_Type::ACC_R;
    if(strcmp(resource, VIEW_ACCOUNTS) == 0) return Resource_Type::ACC_R;

    if (strcmp(resource, SET_LEDGERS) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, GET_LEDGERS) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, VIEW_LEDGERS) == 0) return Resource_Type::ACC_R;

    if (strcmp(resource, SET_INVENTORY) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, GET_INVENTORY) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, EDIT_INVENTORY) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, GET_EDIT_HISTORY) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, GET_INVENTORY_SERVICES) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, GET_INVENTORY_HISTORY) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, GET_INVENTORY_GRAPH) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, ADD_SERVICE) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, REMOVE_SERVICE) == 0) return Resource_Type::ACC_W;

    if (strcmp(resource, SET_SERVICES) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, CLOSE_SERVICE) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, CLOSE_SERVICE_ACCOUNT) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, GET_SERVICES) == 0) return Resource_Type::ACC_R;
	if (strcmp(resource, GET_PRODUCT_IN_SERVICES) == 0) return Resource_Type::ACC_R;

    if (strcmp(resource, SET_CATEGORIES) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, GET_CATEGORIES) == 0) return Resource_Type::ACC_R;

    if (strcmp(resource, GET_SUPPLIERS) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, SET_SUPPLIERS) == 0) return Resource_Type::ACC_W;

    if (strcmp(resource, REPORT_INCOME) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, REPORT_START_CASH) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, REPORT_END_CASH) == 0) return Resource_Type::ACC_R;

    if (strcmp(resource, SET_CUSTOMERS) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, GET_CUSTOMERS) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, GET_SIMP_CUSTOMERS) == 0) return Resource_Type::ACC_R;

    if (strcmp(resource, SET_SALES_ENTRY) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, GET_SALES_ENTRY) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, REVERT_SALES_ENTRY) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, GET_SALES_ENTRY_HISTORY) == 0) return Resource_Type::ACC_R;

    if (strcmp(resource, GET_TRANSACTION_HISTORY) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, GET_CUSTOMER_ORDER_HISTORY) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, GET_SUPPLIER_ORDER_HISTORY) == 0) return Resource_Type::ACC_R;
    if (strcmp(resource, GET_RECEIPT) == 0) return Resource_Type::ACC_R;

    if (strcmp(resource, SET_EMPLOYEE) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, GET_EMPLOYEES) == 0) return Resource_Type::ACC_R;

    if (strcmp(resource, SET_JOBS) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, GET_JOBS) == 0) return Resource_Type::ACC_R;

    if (strcmp(resource, BATCH_EXPORT) == 0) return Resource_Type::ACC_W;
    if (strcmp(resource, BATCH_IMPORT) == 0) return Resource_Type::ACC_W;

	//deal with this throw it back 
    return Resource_Type::Type_Not_Found;
}

class PathRes
{
public:
	static std::string GetAbsolutePath (const char* RelativePath)
	{
        TCHAR buffer[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, buffer);
        CefString directory = buffer;

        std::string tmp = std::string(projectName);
        CefString ProjectName = CefString(tmp);

        int index = 0;

        for (int i = 0; i < directory.size(); i++)
        {
            if (directory.ToString().at(i) == ProjectName.ToString().at(0))
            {
                for (int j = 0; j < ProjectName.size(); j++)
                {
                    int ij = i + j;

                    if (directory.ToString().at(ij) == ProjectName.ToString().at(j))
                    {
                        if (j == ProjectName.size() - 1)
                        {
                            index = i + j;
                        }
                    }
                    else
                        break;
                }
            }

            if (index != 0)
            {
                index += 1;
                break;
            }
        }

        CefString dir = directory.ToString().substr(0, index) + "\\" + RelativePath;

        return dir.ToString();
	}
};