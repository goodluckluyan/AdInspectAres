//@file:C_HttpParser.h
//@brief:
//@author: duheqing@oristartech.com
//date: 2012-7-10

#ifndef _H_HTTPPARSER_
#define _H_HTTPPARSER_

#include <string>
#include <stdlib.h>
#include <string.h>
#include<stdio.h>

bool IsRightVersion(const std::string &version);
void ToLower(char &elem);

class HttpRequestParser
{
public:
	HttpRequestParser();
	~HttpRequestParser();

	//set data and create a request, set all element and get http request.
	std::string &GetHttpRequest(bool action = true);

	int SetMethod(const std::string &method);
	int SetUri(const std::string &uri);
	int SetVersion(const std::string &version);
	int SetHost(const std::string &host);
	int SetUserAgent(const std::string &userAgent);
	int SetAccept(const std::string &accept);
	int SetAcceptCharset(const std::string &acceptCharset);
	int SetContentType(const std::string &contentType);
	int SetContent(const std::string &content);
	int SetSoapAction(const std::string &action);
	int SetCookie(const std::string &cookie);

	//clear all data
	void ClearHttp();

private:
	//request line
	std::string m_method;
	std::string m_uri;
	std::string m_version;
	
	//header message
	std::string m_host;
	std::string m_userAgent;
	std::string m_accept;
	std::string m_acceptCharset;
	std::string m_contentType;
	std::string m_soapAction;
	std::string m_Cookie;

	std::string m_content;
	std::string m_httpRequest;

private:
	bool IsRightMethod(const std::string &method);
};


class HttpResponseParser
{
public:
	HttpResponseParser();
	~HttpResponseParser();

	//get data from response, set response and parse it.
	int SetHttpResponse(const std::string &httpResponse);

	std::string &GetVersion(){ return m_version; }
	int GetStatus(){ return atoi(m_statusCode.c_str()); }
	std::string &GetReason(){ return m_reason; }

	std::string &GetServer(){ return m_server; }
	std::string &GetContentType(){ return m_contentType; }
	std::string &GetContent(){ return m_content; }
	std::string &GetCookie(){ return m_Cookie; }

	//clear all data
	void ClearHttp();

	enum StatusCode//http status code
	{
		RecvAndContinue = 100,
		Success = 200,
		Redirect = 300,
		ClientError = 400,
		ServerError = 500
	};

private:
	//status line
	std::string m_version;
	std::string m_statusCode;
	std::string m_reason;

	//header message
	std::string m_server;
	std::string m_contentLength;
	std::string m_contentType;

	std::string m_content;
	std::string m_httpResponse;
	std::string m_Cookie;

private:
	//get head element
	int SetHeaderMessage(const std::string &headElem);
};

#endif//_H_HTTPPARSER_
