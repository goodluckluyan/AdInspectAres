#ifndef _C_H_SOAPSERVER_
#define _C_H_SOAPSERVER_

#include "soapH.h"
#include <fstream>

void *MonitorSoapServerMain(void *port);
int MonitorForBrowser(struct soap *soapHandle);
void ExitSoapService();


void *process_request(void *csoap);
#endif//_C_H_SOAPSERVER_
