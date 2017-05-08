#include "MyLogger.h"
#include <log4cxx/logger.h>    
#include <log4cxx/propertyconfigurator.h> 

using namespace log4cxx; 

MyLogger::MyLogger()
{
}
MyLogger::~MyLogger()
{
}

void MyLogger::Init(std::string prop_file,std::string module_name)
{
    PropertyConfigurator::configure(prop_file);
    m_modulename = module_name;
}


void MyLogger::PrintLog(LEVEL_M level,const char *msg,...)
{
    char buff[LOG_BUFF_SIZE]={'\0'};
    msgFormatA(buff,LOG_BUFF_SIZE,msg);

    switch(level)
    {
    case TRACE:
        LOG4CXX_TRACE(log4cxx::Logger::getLogger(m_modulename),buff);
        break;
    case DEBUG:
        LOG4CXX_DEBUG(log4cxx::Logger::getLogger(m_modulename),buff);
        break;
    case INFO:
        LOG4CXX_INFO(log4cxx::Logger::getLogger(m_modulename),buff);
        break;
    case WARN:
        LOG4CXX_WARN(log4cxx::Logger::getLogger(m_modulename),buff);
        break;
    case ERROR:
        LOG4CXX_ERROR(log4cxx::Logger::getLogger(m_modulename),buff);
        break;
    case FATAL:
        LOG4CXX_FATAL(log4cxx::Logger::getLogger(m_modulename),buff);
        break;
    case OFF:
        break;
    default:
        break;
    }

}
