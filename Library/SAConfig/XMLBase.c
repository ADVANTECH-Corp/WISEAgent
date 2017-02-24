#include "XMLBase.h"
#include <unistd.h>
#include <string.h>
#include "util_string.h"

char * xml_ansitoutf8(char* wText)
{
	char * utf8RetStr = NULL;
	int tmpLen = 0;
	if(!wText)
		return utf8RetStr;
	if(!IsUTF8(wText))
	{
		utf8RetStr = ANSIToUTF8(wText);
		tmpLen = !utf8RetStr ? 0 : strlen(utf8RetStr);
		if(tmpLen == 1)
		{
			if(utf8RetStr) free(utf8RetStr);
			utf8RetStr = UnicodeToUTF8((wchar_t *)wText);
		}
	}
	else
	{
		tmpLen = strlen(wText)+1;
		utf8RetStr = (char *)malloc(tmpLen);
		memcpy(utf8RetStr, wText, tmpLen);
	}
	return utf8RetStr;
}

xmlXPathObjectPtr static xml_GetNodeSet(xmlDocPtr doc, const xmlChar *pXpath) 
{
	xmlXPathContextPtr context = NULL;	    
	xmlXPathObjectPtr xpRet = NULL;		
    if(doc == NULL || pXpath == NULL) return xpRet;
	{
		context = xmlXPathNewContext(doc);		
		if (context != NULL) 
		{	
			xpRet = xmlXPathEvalExpression(pXpath, context); 
			xmlXPathFreeContext(context);				
			if (xpRet != NULL) 
			{
				if (xmlXPathNodeSetIsEmpty(xpRet->nodesetval))   
				{
					xmlXPathFreeObject(xpRet);
					xpRet = NULL;
				}
			}
		}
	}

	return xpRet;	
}

xml_doc_info * SACONFIG_API xml_Loadfile(char const * configFile, char const * const xml_root, char const * const xml_base)
{
	xml_doc_info * docinfo = NULL;
	xmlDocPtr doc = NULL;
	int count = 10;
	if(configFile == NULL) 
		return docinfo;

	if(access(configFile, F_OK))
		return docinfo;

	xmlInitParser();

	while(access(configFile, R_OK))
	{
		if(count <= 0)
			return docinfo;
		count--;
		usleep(500*1000);
	}

	doc = xmlReadFile(configFile, "UTF-8", 0);
	if(doc == NULL)
		return docinfo;


	docinfo = (xml_doc_info *)malloc(sizeof(xml_doc_info));
	memset(docinfo, 0, sizeof(xml_doc_info));
	docinfo->isInitParser = true;
	docinfo->doc = doc;
	strcpy(docinfo->xml_root, xml_root);
	strcpy(docinfo->xml_base, xml_base);
	return docinfo;
}

xml_doc_info * SACONFIG_API xml_CreateDoc(char const * const xml_root, char const * const xml_base)
{
	xml_doc_info * docinfo = NULL;
	xmlDocPtr doc = NULL;
	xmlNodePtr curNode = NULL;
	xmlInitParser();
	doc = xmlNewDoc(BAD_CAST("1.0"));
	curNode = xmlNewNode(NULL, BAD_CAST(xml_root));
	xmlDocSetRootElement(doc, curNode);
	curNode = xmlNewChild(curNode, NULL, BAD_CAST(xml_base), NULL);

	docinfo = (xml_doc_info *)malloc(sizeof(xml_doc_info));
	memset(docinfo, 0, sizeof(xml_doc_info));
	docinfo->isInitParser = true;
	docinfo->doc = doc;
	strcpy(docinfo->xml_root, xml_root);
	strcpy(docinfo->xml_base, xml_base);
	return docinfo;
}

bool SACONFIG_API xml_SaveFile(char const * configFile, xml_doc_info * doc)
{
	bool bRet = false;
	if(configFile == NULL) 
		return bRet;

	if(doc == NULL)
		return bRet;

	if(!access(configFile, F_OK))
	{
		int count = 10;
		while(access(configFile, W_OK))
		{
			if(count <= 0)
				return bRet;
			count--;
			usleep(500*1000);
		}
	}
	if(xmlSaveFile(configFile, doc->doc)>0)
		bRet = true;

	return bRet;
}

void SACONFIG_API xml_FreeDoc(xml_doc_info * doc)
{
	if(doc == NULL) 
		return;

	xmlFreeDoc(doc->doc);
	if(doc->isInitParser)
		xmlCleanupParser();
	doc->doc = NULL;
	free(doc);
}

bool SACONFIG_API xml_GetItemValue(xml_doc_info const * doc, char const * const itemName, char * itemValue, int valueLen)
{
	bool bRet = false;
	if(NULL == doc || NULL == itemName || NULL == itemValue) return bRet;
	{
		xmlChar * pXPath = NULL;
		xmlXPathObjectPtr xpRet = NULL;
		char xPathStr[128] = {0};

		sprintf(xPathStr, "/%s/%s/%s",doc->xml_root, doc->xml_base, itemName);
		pXPath = BAD_CAST(xPathStr);

		xpRet = xml_GetNodeSet(doc->doc, pXPath);
		if(xpRet) 
		{
			int i = 0;
			xmlNodeSetPtr nodeset = xpRet->nodesetval;
			for (i = 0; i < nodeset->nodeNr; i++) 
			{
				xmlNodePtr curNode = nodeset->nodeTab[i];    
				if(curNode != NULL) 
				{
					xmlChar *nodeValue = xmlNodeGetContent(curNode);
					if (nodeValue != NULL) 
					{
						if(xmlStrlen(nodeValue) <= valueLen)
						{
							strcpy(itemValue, (char*)nodeValue);
							bRet = true;
						}
						else
						{
							strncpy(itemValue, (char*)nodeValue, valueLen);
							bRet = true;
						}
						xmlFree(nodeValue);
						break;
					}
				}
			}
			xmlXPathFreeObject(xpRet);
		}
	}
	return bRet;
}

bool SACONFIG_API xml_SetItemValue(xml_doc_info const * doc, char const * const itemName, char const * const itemValue)
{
	bool bRet = false;
	
	if(NULL == doc || NULL == itemName || NULL == itemValue) return bRet;
	{
		xmlNodePtr curNode = NULL;
		char* utf8value = xml_ansitoutf8(itemValue);

		//if(strlen(itemValue))
		{
			xmlChar * pXPath = NULL;
			char xPathStr[128] = {0};
			xmlXPathObjectPtr xpRet = NULL;
			sprintf(xPathStr, "/%s/%s/%s",doc->xml_root, doc->xml_base, itemName);
			pXPath = BAD_CAST(xPathStr);
			xpRet = xml_GetNodeSet(doc->doc, pXPath);
			if(xpRet) 
			{
				int i = 0;
				xmlNodeSetPtr nodeset = xpRet->nodesetval;
				for (i = 0; i < nodeset->nodeNr; i++) 
				{
					curNode = nodeset->nodeTab[i];    
					if(curNode != NULL) 
					{
						xmlNodeSetContent(curNode, BAD_CAST(utf8value));
						bRet = true;
						break;
					}
				}
				xmlXPathFreeObject (xpRet);
			}
			else
			{
				memset(xPathStr, 0, sizeof(xPathStr));
				sprintf(xPathStr, "/%s/%s",doc->xml_root, doc->xml_base);
				pXPath = BAD_CAST(xPathStr);
				xpRet = xml_GetNodeSet(doc->doc, pXPath);
				if(xpRet)
				{
					int i = 0;
					xmlNodeSetPtr nodeset = xpRet->nodesetval;
					for (i = 0; i < nodeset->nodeNr; i++) 
					{
						curNode = nodeset->nodeTab[i];    
						if(curNode != NULL) 
						{
							xmlNewTextChild(curNode, NULL, BAD_CAST(itemName), BAD_CAST(utf8value));
							bRet = true;
							break;
						}
					}
					xmlXPathFreeObject (xpRet);
				}
				else
				{
					memset(xPathStr, 0, sizeof(xPathStr));
					sprintf(xPathStr, "/%s",doc->xml_root);
					pXPath = BAD_CAST(xPathStr);
					xpRet = xml_GetNodeSet(doc->doc, pXPath);
					if(xpRet)
					{
						int i = 0;
						xmlNodeSetPtr nodeset = xpRet->nodesetval;
						for (i = 0; i < nodeset->nodeNr; i++) 
						{
							curNode = nodeset->nodeTab[i];    
							if(curNode != NULL) 
							{
								xmlNodePtr tNode = xmlNewChild(curNode, NULL, BAD_CAST(doc->xml_base), NULL);
								xmlNewTextChild(tNode, NULL, BAD_CAST(itemName), BAD_CAST(utf8value));
								bRet = true;
								break;
							}
						}
						xmlXPathFreeObject (xpRet);
					}
				}
			}
		}

		free(utf8value);
	}
	return bRet;
}
