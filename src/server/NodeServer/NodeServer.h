#ifndef _NodeServer_H_
#define _NodeServer_H_

#include <iostream>
#include "servant/Application.h"

using namespace tars;

/**
 *
 **/
class NodeServer : public Application
{
public:
    /**
     *
     **/
    virtual ~NodeServer() {};

    /**
     *
     **/
    virtual void initialize();

    /**
     *
     **/
    virtual void destroyApp();
};

extern NodeServer g_app;

////////////////////////////////////////////
#endif
