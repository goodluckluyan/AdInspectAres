
#ifndef _H_PARSER_XML_
#define _H_PARSER_XML_
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMNode.hpp>	
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/HandlerBase.hpp>


class Char2XMLCh
{
public :
	//  Constructors and Destructor
	Char2XMLCh()
	{
		fUnicodeForm = NULL;
	}
	Char2XMLCh(const char* toTranscode)
	{
		fUnicodeForm = xercesc::XMLString::transcode(toTranscode);
	}
	~Char2XMLCh()
	{
		if (fUnicodeForm)
		{
			xercesc::XMLString::release(&fUnicodeForm);
			fUnicodeForm = NULL;
		}
	}

	void operator = (const char* chstr)
	{
		fUnicodeForm = xercesc::XMLString::transcode(chstr);
	}

	const XMLCh* unicodeForm() const
	{
		return fUnicodeForm;
	}

	XMLCh* get_xml_str()
	{
		return fUnicodeForm;
	}

private :
	XMLCh* fUnicodeForm;
};

#define C2X(str) Char2XMLCh(str).unicodeForm()

#endif
