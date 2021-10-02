#include "ServerManager.h"

int main(int argc, char* argv[])
{
	ServerManager* sm = new ServerManager;
	return sm->Run();
}