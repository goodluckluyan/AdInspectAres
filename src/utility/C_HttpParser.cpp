//@file:C_HttpParser.cpp
//@brief:
//@author: duheqing@oristartech.com
//date: 2012-7-10

#include "C_HttpParser.h"

#include "iostream"
#include <algorithm>

using namespace std;

bool IsRightVersion(const string &version)
{
	if((version.size() > strlen("http/1"))
		&& (version[0] == 'H' || version[0] == 'h') && (version[1] == 'T' || version[1] == 't')
		&& (version[2] == 'T' || version[2] == 't') && (version[3] == 'P' || version[3] == 'p'))
		return true;
	else
		return false;
}

void ToLower(char &elem)
{
	if(elem>='A' && elem<='Z')
                elem += 'a' - 'A';
}

HttpRequestParser::HttpRequestParser(){}
HttpRequestParser::~HttpRequestParser(){}

string& HttpRequestParser::GetHttpRequest(bool action)
{
	if(m_method.empty() || m_uri.empty() || m_version.empty())
		return m_httpRequest;

	string enter = "\r\n";

	m_httpRequest = m_method + " " + m_uri + " " + m_version + enter;
	if(!m_host.empty())
		m_httpRequest += "Host:" + m_host + enter;
	if(!m_userAgent.empty())
		m_httpRequest += "User-Agent:" + m_userAgent + enter;
	if(!m_Cookie.empty())
		m_httpRequest += "Cookie:" + m_Cookie + enter;
	if(!m_accept.empty())
		m_httpRequest += "Accept:" + m_accept + enter;
	if(!m_acceptCharset.empty())
		m_httpRequest += "Accept-Charset:" + m_acceptCharset + enter;
	if(!m_content.empty())
	{
		char tmp[32];
		sprintf(tmp, "%u", m_content.size());
		m_httpRequest += "Content-Length:";
		m_httpRequest += tmp + enter;
	}
	if(!m_content.empty())
		m_httpRequest += "Content-Type:" + m_contentType + enter;
	if(action)
		m_httpRequest += "SoapAction:" + m_soapAction + enter;
	m_httpRequest += enter;
	if(!m_content.empty())
		m_httpRequest += m_content;

	return m_httpRequest;
}

int HttpRequestParser::SetMethod(const string &method)
{
	if(!IsRightMethod(method))
		return -1;

	m_method = method;
	return 0;
}

int HttpRequestParser::SetUri(const string &uri)
{
	if(uri.empty())
		return -1;

	if((uri.size() > strlen("http"))
		&& (uri[0] == 'h' || uri[0] == 'H')
		&& (uri[1] == 't' || uri[1] == 'T')
		&& (uri[2] == 't' || uri[2] == 'T')
		&& (uri[3] == 'p' || uri[3] == 'P'))//duheqing 2013-9-27
		;//do nothing
	else if(uri[0] != '/')
		m_uri = '/';
	else
		;//do nothing
	m_uri += uri;
	return 0;
}

int HttpRequestParser::SetVersion(const string &version)
{
	if(!IsRightVersion(version))
		return -1;

	m_version = version;
	return 0;
}

int HttpRequestParser::SetHost(const string &host)
{
	if(host.empty())
		return -1;

	m_host = host;
	return 0;
}

int HttpRequestParser::SetUserAgent(const string &userAgent)
{
	if(userAgent.empty())
		return -1;

	m_userAgent = userAgent;
	return 0;
}

int HttpRequestParser::SetAccept(const string &accept)
{
	if(accept.empty())
		return -1;

	m_accept = accept;
	return 0;
}

int HttpRequestParser::SetAcceptCharset(const string &acceptCharset)
{
	if(acceptCharset.empty())
		return -1;

	m_acceptCharset = acceptCharset;
	return 0;
}

int HttpRequestParser::SetContentType(const string &contentType)
{
	if(contentType.empty())
		return -1;

	m_contentType = contentType;
	return 0;
}

int HttpRequestParser::SetContent(const string &content)
{
	if(content.empty())
		return -1;

	m_content = content;
	return 0;
}
int HttpRequestParser::SetSoapAction(const string &action)
{
	if(action.empty())
		return -1;

	m_soapAction = action;
	return 0;
}

int HttpRequestParser::SetCookie(const std::string &cookie)
{
	if(cookie.empty())
		return -1;

	m_Cookie = cookie;
	return 0;
}

void HttpRequestParser::ClearHttp()
{
	m_method.clear();
	m_uri.clear();
	m_version.clear();
	m_host.clear();
	m_userAgent.clear();
	m_accept.clear();
	m_acceptCharset.clear();
	m_contentType.clear();
	m_soapAction.clear();
	m_content.clear();
	m_httpRequest.clear();
}

bool HttpRequestParser::IsRightMethod(const string &method)
{
	if(method.empty())
		return false;

	string info = method;
	for_each(info.begin(), info.end(), ToLower);

	if(info == "get" || info == "post" || info == "head" || info == "put"
		|| info == "delete" || info == "trace" || info == "connect" || info == "options")
		return true;
	return false;
}



HttpResponseParser::HttpResponseParser(){}
HttpResponseParser::~HttpResponseParser(){}

int HttpResponseParser::SetHttpResponse(const string &httpResponse)
{	//parse http response
	m_httpResponse = httpResponse;

	//duheqing for cookie 2013-8-2
// 	unsigned int cookiepos = m_httpResponse.find("JSESSIONID=");
// 	if(cookiepos != string::npos)
// 		m_Cookie = m_httpResponse.substr(cookiepos, strlen("JSESSIONID=") + 32);

	unsigned int contentHead = httpResponse.find("\r\n\r\n");
	if(contentHead == string::npos)
	{
		return -1;
	}

	unsigned int bpos = 0;
	unsigned int epos = httpResponse.find(" ");
	if(epos > contentHead)
	{
		return -1;
	}

	m_version.assign(httpResponse, bpos, epos - bpos);
	
	bpos = epos + 1;
	epos = httpResponse.find(" ", bpos);
	if(epos > contentHead)
	{
		return -1;
	}

	m_statusCode.assign(httpResponse, bpos, epos - bpos);
	if(atoi(m_statusCode.c_str()) < 100 || atoi(m_statusCode.c_str()) > 599)
	{
		return -1;
	}

	bpos = epos + 1;
	epos = httpResponse.find("\r\n", bpos);
	if(epos > contentHead)
		return -1;
	m_reason.assign(httpResponse, bpos, epos - bpos);
	bpos = epos + 2;

	while((epos = httpResponse.find("\r\n", bpos)) <=  contentHead)
	{
		int ret = SetHeaderMessage(httpResponse.substr(bpos, epos - bpos));
		if(ret != 0)
			;//do nothing
		bpos = epos + 2;
	}

	contentHead += 4;
	m_content.assign(httpResponse, contentHead, httpResponse.size());
	return 0;
}

int HttpResponseParser::SetHeaderMessage(const string &headElem)
{
	string info = headElem;
	unsigned int pos = info.find(':');
	if(pos == string::npos)
		return -1;
	if(info[++pos] == ' ')
		++pos;

	std::for_each(info.begin(), find(info.begin(), info.end(), ':'), ToLower);

	if(info.compare(0, strlen("server"), "server") == 0)
		m_server.assign(info, pos, info.size());
	else if(info.compare(0, strlen("content-length"), "content-length") == 0)
		m_contentLength.assign(info, pos, info.size());
	else if(info.compare(0, strlen("content-type"), "content-type") == 0)
		m_contentType.assign(info, pos, info.size());
	else
		return -1;
	return 0;
}

void HttpResponseParser::ClearHttp()
{
	m_version.clear();
	m_statusCode.clear();
	m_reason.clear();
	m_server.clear();
	m_contentLength.clear();
	m_contentType.clear();
	m_content.clear();
	m_httpResponse.clear();
}
