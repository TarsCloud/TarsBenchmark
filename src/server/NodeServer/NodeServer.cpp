#include "NodeServer.h"
#include "NodeImp.h"

using namespace std;

NodeServer g_app;

/////////////////////////////////////////////////////////////////
void
NodeServer::initialize()
{
    //initialize application here:
    //...

    addServant<NodeImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".NodeObj");
}
/////////////////////////////////////////////////////////////////
void
NodeServer::destroyApp()
{
    //destroy application here:
    //...
}
/////////////////////////////////////////////////////////////////
int
main(int argc, char* argv[])
{
    try
    {
        g_app.main(argc, argv);
        g_app.waitForShutdown();
    }
    catch (std::exception& e)
    {
        cerr << "std::exception:" << e.what() << std::endl;
    }
    catch (...)
    {
        cerr << "unknown exception." << std::endl;
    }
    return -1;
}
/////////////////////////////////////////////////////////////////
