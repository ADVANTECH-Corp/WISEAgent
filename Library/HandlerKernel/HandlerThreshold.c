#include "HandlerThreshold.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#define DEF_MAX_THR_EVENT_STR                   "#tk#maxThreshold#tk#"
#define DEF_MIN_THR_EVENT_STR                   "#tk#minThreshold#tk#"
#define DEF_AVG_EVENT_STR                       "#tk#average#tk#"
#define DEF_MAX_EVENT_STR                       "#tk#max#tk#"
#define DEF_MIN_EVENT_STR                       "#tk#min#tk#"
#define DEF_AND_EVENT_STR                       "#tk#and#tk#"
#define DEF_NOR_EVENT_STR                       "#tk#normal#tk#"
#define DEF_NOT_SUPT_EVENT_STR                  "#tk#not surport#tk#"

thr_item_list HANDLERTHRESHOLD_API HandlerThreshold_CreateThrList()
{
	thr_item_node_t * head = NULL;
	head = (thr_item_node_t *)malloc(sizeof(thr_item_node_t));
	if(head)
	{
		head->next = NULL;
		memset(head->thrItemInfo.pathname, 0, sizeof(head->thrItemInfo.pathname));
		head->thrItemInfo.isEnable = false;
		head->thrItemInfo.maxThr = DEF_INVALID_VALUE;
		head->thrItemInfo.minThr = DEF_INVALID_VALUE;
		head->thrItemInfo.rangeType = range_unknow;
		head->thrItemInfo.lastingTimeS = DEF_INVALID_TIME;
		head->thrItemInfo.intervalTimeS = DEF_INVALID_TIME;
		head->thrItemInfo.checkRetValue = DEF_INVALID_VALUE;
		head->thrItemInfo.checkSrcValList.head = NULL;
		head->thrItemInfo.checkSrcValList.nodeCnt = 0;
		head->thrItemInfo.checkType = ck_type_unknow;
		head->thrItemInfo.repThrTime = DEF_INVALID_VALUE;
		head->thrItemInfo.isNormal = true;
		head->thrItemInfo.isInvalid = true;
		head->thrItemInfo.on_triggered = NULL;
	}
	return head;
}

void HANDLERTHRESHOLD_API HandlerThreshold_DestroyThrList(thr_item_list thrList)
{
	if(NULL == thrList) return;
	HandlerThreshold_DeleteAllThrNode(thrList);
	free(thrList); 
	thrList = NULL;
}

bool HANDLERTHRESHOLD_API HandlerThreshold_InsertThrList(thr_item_list thrList, thr_item_info_t * pThrItem)
{
	bool bRet = false;
	thr_item_node_t * newNode = NULL, * findNode = NULL, *head = NULL;
	if(pThrItem == NULL || thrList == NULL) return bRet;
	head = thrList;
	findNode = HandlerThreshold_FindThrNode(head, pThrItem->pathname);
	if(findNode == NULL)
	{
		newNode = (thr_item_node_t *)malloc(sizeof(thr_item_node_t));
		memset(newNode, 0, sizeof(thr_item_node_t));

		strcpy(newNode->thrItemInfo.pathname, pThrItem->pathname);
		newNode->thrItemInfo.isEnable = pThrItem->isEnable;
		newNode->thrItemInfo.maxThr = pThrItem->maxThr;
		newNode->thrItemInfo.minThr = pThrItem->minThr;
		newNode->thrItemInfo.rangeType = pThrItem->rangeType;
		newNode->thrItemInfo.lastingTimeS = pThrItem->lastingTimeS;
		newNode->thrItemInfo.intervalTimeS = pThrItem->intervalTimeS;
		newNode->thrItemInfo.checkType = pThrItem->checkType;
		newNode->thrItemInfo.checkRetValue = DEF_INVALID_VALUE;
		newNode->thrItemInfo.checkSrcValList.head = NULL;
		newNode->thrItemInfo.checkSrcValList.nodeCnt = 0;
		newNode->thrItemInfo.repThrTime = 0;
		newNode->thrItemInfo.isNormal = pThrItem->isNormal;
		newNode->thrItemInfo.isInvalid = pThrItem->isInvalid;
		newNode->thrItemInfo.on_triggered = pThrItem->on_triggered;
		newNode->next = head->next;
		head->next = newNode;
		bRet = true;
	}
	else
	{
		bRet = false;
	}
	return bRet;
}

thr_item_node_t * HANDLERTHRESHOLD_API HandlerThreshold_FindThrNode(thr_item_list thrList, char* pathname)
{
	thr_item_node_t * findNode = NULL, *head = NULL;
	if(thrList == NULL) return findNode;
	head = thrList;
	findNode = head->next;
	while(findNode)
	{
		if(strcmp(findNode->thrItemInfo.pathname, pathname) == 0) break;
		else
		{
			findNode = findNode->next;
		}
	}

	return findNode;
}

bool HANDLERTHRESHOLD_API HandlerThreshold_DeleteThrNode(thr_item_list thrList, char* pathname)
{
	bool bRet = false;
	thr_item_node_t * delNode = NULL, *head = NULL;
	thr_item_node_t * p = NULL;
	if(thrList == NULL) return bRet;
	head = thrList;
	p = head;
	delNode = head->next;
	while(delNode)
	{
		if(strcmp(delNode->thrItemInfo.pathname, pathname) == 0)
		{
			p->next = delNode->next;
			if(delNode->thrItemInfo.checkSrcValList.head)
			{
				check_value_node_t * frontValueNode = delNode->thrItemInfo.checkSrcValList.head;
				check_value_node_t * delValueNode = frontValueNode->next;
				while(delValueNode)
				{
					frontValueNode->next = delValueNode->next;
					free(delValueNode);
					delValueNode = frontValueNode->next;
				}
				free(delNode->thrItemInfo.checkSrcValList.head);
				delNode->thrItemInfo.checkSrcValList.head = NULL;
			}
			free(delNode);
			delNode = NULL;
			bRet = true;
			break;
		}
		else
		{
			p = delNode;
			delNode = delNode->next;
		}
	}
	return bRet;
}

bool HANDLERTHRESHOLD_API HandlerThreshold_DeleteAllThrNode(thr_item_list thrList)
{
	bool bRet = false;
	thr_item_node_t * delNode = NULL, *head = NULL;
	if(thrList == NULL) return bRet;
	head = thrList;

	delNode = head->next;
	while(delNode)
	{
		head->next = delNode->next;
		if(delNode->thrItemInfo.checkSrcValList.head)
		{
			check_value_node_t * frontValueNode = delNode->thrItemInfo.checkSrcValList.head;
			check_value_node_t * delValueNode = frontValueNode->next;
			while(delValueNode)
			{
				frontValueNode->next = delValueNode->next;
				free(delValueNode);
				delValueNode = frontValueNode->next;
			}
			free(delNode->thrItemInfo.checkSrcValList.head);
			delNode->thrItemInfo.checkSrcValList.head = NULL;
		}
		free(delNode);
		delNode = head->next;
	}

	bRet = true;
	return bRet;
}

bool HANDLERTHRESHOLD_API HandlerThreshold_UpdateThrInfoList(thr_item_list curThrList, thr_item_list newThrList, char ** checkRetMsg, unsigned int bufLen)
{
	bool bRet = false;
	if(NULL == newThrList || NULL == curThrList || checkRetMsg == NULL || bufLen == 0) return bRet;
	{
		thr_item_node_t * newThrItemNode = NULL, * findThrItemNode = NULL;
		thr_item_node_t * curThrItemNode = curThrList->next;
		while(curThrItemNode) //first all thr node set invalid
		{
			curThrItemNode->thrItemInfo.isInvalid = 1;
			curThrItemNode = curThrItemNode->next;
		}
		newThrItemNode = newThrList->next;
		while(newThrItemNode)  //merge old&new thr list
		{
			findThrItemNode = HandlerThreshold_FindThrNode(curThrList, newThrItemNode->thrItemInfo.pathname);
			if(findThrItemNode) //exist then update thr argc
			{
				findThrItemNode->thrItemInfo.isInvalid = 0;
				findThrItemNode->thrItemInfo.intervalTimeS = newThrItemNode->thrItemInfo.intervalTimeS;
				findThrItemNode->thrItemInfo.lastingTimeS = newThrItemNode->thrItemInfo.lastingTimeS;
				findThrItemNode->thrItemInfo.isEnable = newThrItemNode->thrItemInfo.isEnable;
				findThrItemNode->thrItemInfo.maxThr = newThrItemNode->thrItemInfo.maxThr;
				findThrItemNode->thrItemInfo.minThr = newThrItemNode->thrItemInfo.minThr;
				findThrItemNode->thrItemInfo.rangeType = newThrItemNode->thrItemInfo.rangeType;
				findThrItemNode->thrItemInfo.on_triggered = newThrItemNode->thrItemInfo.on_triggered; 
			}
			else  //not exist then insert to old list
			{
				HandlerThreshold_InsertThrList(curThrList, &newThrItemNode->thrItemInfo);
			}
			newThrItemNode = newThrItemNode->next;
		}
		{
			unsigned int defbufLen = bufLen;
			thr_item_node_t * preNode = curThrList,* normalRepNode = NULL, *delNode = NULL;
			curThrItemNode = preNode->next;
			while(curThrItemNode) //check need delete&normal report node
			{
				normalRepNode = NULL;
				delNode = NULL;
				if(curThrItemNode->thrItemInfo.isInvalid == 1)
				{
					preNode->next = curThrItemNode->next;
					delNode = curThrItemNode;
					if(curThrItemNode->thrItemInfo.isNormal == false)
					{
						normalRepNode = curThrItemNode;
					}
				}
				else
				{
					preNode = curThrItemNode;
				}
				if(normalRepNode == NULL && curThrItemNode->thrItemInfo.isEnable == false && curThrItemNode->thrItemInfo.isNormal == false)
				{
					normalRepNode = curThrItemNode;
				}
				if(normalRepNode)
				{
					char *tmpMsg = NULL;
					int len = strlen(curThrItemNode->thrItemInfo.pathname)+strlen(DEF_NOR_EVENT_STR)+32;
					tmpMsg = (char*)calloc(1, len);
					sprintf(tmpMsg, "%s %s", curThrItemNode->thrItemInfo.pathname, DEF_NOR_EVENT_STR);
					if(tmpMsg && strlen(tmpMsg))
					{
						if(defbufLen<strlen(tmpMsg)+strlen(*checkRetMsg)+1)
						{
							int newLen = strlen(tmpMsg) + strlen(*checkRetMsg) + 1024;
							*checkRetMsg = (char *)realloc(*checkRetMsg, newLen);
							defbufLen = newLen;
						}	
						if(strlen(*checkRetMsg))
						{
							strcat(*checkRetMsg, tmpMsg);
							//sprintf(*checkRetMsg, "%s;%s", *checkRetMsg, tmpMsg);
						}
						else
						{
							sprintf(*checkRetMsg, "%s", tmpMsg);
						}
					}
					if(tmpMsg)free(tmpMsg);
					tmpMsg = NULL;
					normalRepNode->thrItemInfo.isNormal = true;
				}
				if(delNode)
				{
					if(delNode->thrItemInfo.checkSrcValList.head)
					{
						check_value_node_t * frontValueNode = delNode->thrItemInfo.checkSrcValList.head;
						check_value_node_t * delValueNode = frontValueNode->next;
						while(delValueNode)
						{
							frontValueNode->next = delValueNode->next;
							free(delValueNode);
							delValueNode = frontValueNode->next;
						}
						free(delNode->thrItemInfo.checkSrcValList.head);
						delNode->thrItemInfo.checkSrcValList.head = NULL;
					}
					free(delNode);
					delNode = NULL;
				}
				curThrItemNode = preNode->next;
			}
		}
	}
	bRet = true;
	return bRet;
}

void HANDLERTHRESHOLD_API HandlerThreshold_WhenDelThrCheckNormal(thr_item_list thrItemList, char** checkMsg, unsigned int bufLen)
{
	if(NULL == thrItemList || NULL == checkMsg || bufLen == 0) return;
	{
		thr_item_list curThrItemList = thrItemList;
		thr_item_node_t * curThrItemNode = curThrItemList->next;
		char *tmpMsg = NULL;
		unsigned int defbufLen = bufLen;
		while(curThrItemNode)
		{
			if(curThrItemNode->thrItemInfo.isEnable && !curThrItemNode->thrItemInfo.isNormal)
			{
				curThrItemNode->thrItemInfo.isNormal = true;
				//sprintf(tmpMsg, "%d normal", curThrItemNode->thrItemInfo.id);
				{
					int len = strlen(curThrItemNode->thrItemInfo.pathname)+strlen(DEF_NOR_EVENT_STR)+32;
					tmpMsg = (char*)calloc(1, len);
				}
				sprintf(tmpMsg, "%s %s", curThrItemNode->thrItemInfo.pathname, DEF_NOR_EVENT_STR);
			}
			if(tmpMsg && strlen(tmpMsg))
			{
				if(defbufLen<strlen(tmpMsg)+strlen(*checkMsg)+1)
				{
					int newLen = strlen(tmpMsg) + strlen(*checkMsg) + 1024;
					*checkMsg = (char *)realloc(*checkMsg, newLen);
					defbufLen = newLen;
				}	
				if(strlen(*checkMsg))
				{
					sprintf(*checkMsg, "%s;%s", *checkMsg, tmpMsg);
				}
				else
				{
					sprintf(*checkMsg, "%s", tmpMsg);
				}
			}
         if(tmpMsg)free(tmpMsg);
			tmpMsg = NULL;
			curThrItemNode = curThrItemNode->next;
		}
	}
}

void HANDLERTHRESHOLD_API HandlerThreshold_IsThrItemListNormal(thr_item_list curThrItemList, bool * isNormal)
{
	if(NULL == isNormal || curThrItemList == NULL) return;
	{
		thr_item_node_t * curThrItemNode = NULL;
		curThrItemNode = curThrItemList->next;
		while(curThrItemNode)
		{
			if(curThrItemNode->thrItemInfo.isEnable && !curThrItemNode->thrItemInfo.isNormal)
			{
				*isNormal = false;
				break;
			}
			curThrItemNode = curThrItemNode->next;
		}
	}
}

bool HANDLERTHRESHOLD_API HandlerThreshold_IsThrListEmpty(thr_item_list thrList)
{
	bool bRet = true;
	thr_item_node_t * curNode = NULL, *head = NULL;
	if(thrList == NULL) return bRet;
	head = thrList;
	curNode = head->next;
	if(curNode != NULL) bRet = false;
	return bRet;
}

bool HandlerThreshold_CheckSrcVal(thr_item_info_t * pThrItemInfo, double checkValue)
{
	bool bRet = false;
	if(pThrItemInfo == NULL) return bRet;
	{
		long long nowTime = time(NULL);
		pThrItemInfo->checkRetValue = DEF_INVALID_VALUE;
		if(pThrItemInfo->checkSrcValList.head == NULL)
		{
			pThrItemInfo->checkSrcValList.head = (check_value_node_t *)malloc(sizeof(check_value_node_t));
			pThrItemInfo->checkSrcValList.nodeCnt = 0;
			pThrItemInfo->checkSrcValList.head->checkValTime = DEF_INVALID_TIME;
			pThrItemInfo->checkSrcValList.head->ckV = DEF_INVALID_VALUE;
			pThrItemInfo->checkSrcValList.head->next = NULL;
		}

		if(pThrItemInfo->checkSrcValList.nodeCnt > 0)
		{
			long long minCkvTime = 0;
			check_value_node_t * curNode = pThrItemInfo->checkSrcValList.head->next;
			minCkvTime = curNode->checkValTime;
			while(curNode)
			{
				if(curNode->checkValTime < minCkvTime)  minCkvTime = curNode->checkValTime;
				curNode = curNode->next; 
			}

			if(nowTime - minCkvTime >= pThrItemInfo->lastingTimeS)
			{
				switch(pThrItemInfo->checkType)
				{
				case ck_type_avg:
					{
						check_value_node_t * curNode = pThrItemInfo->checkSrcValList.head->next;
						double avgTmpF = 0;
						while(curNode)
						{
							if((int)curNode->ckV != DEF_INVALID_VALUE) 
							{
								avgTmpF += curNode->ckV;
							}
							curNode = curNode->next; 
						}
						if(pThrItemInfo->checkSrcValList.nodeCnt > 0)
						{
							avgTmpF = avgTmpF/pThrItemInfo->checkSrcValList.nodeCnt;
							pThrItemInfo->checkRetValue = avgTmpF;
							bRet = true;
						}
						break;
					}
				case ck_type_max:
					{
						check_value_node_t * curNode = pThrItemInfo->checkSrcValList.head->next;
						double maxTmpF = -999;
						while(curNode)
						{
							if(curNode->ckV > maxTmpF) maxTmpF = curNode->ckV;
							curNode = curNode->next; 
						}
						if(maxTmpF > -999)
						{
							pThrItemInfo->checkRetValue = maxTmpF;
							bRet = true;
						}
						break;
					}
				case ck_type_min:
					{
						check_value_node_t * curNode = pThrItemInfo->checkSrcValList.head->next;
						double minTmpF = 99999;
						while(curNode)
						{
							if(curNode->ckV < minTmpF) minTmpF = curNode->ckV;
							curNode = curNode->next; 
						}
						if(minTmpF < 99999)
						{
							pThrItemInfo->checkRetValue = minTmpF;
							bRet = true;
						}
						break;
					}
				default: break;
				}

				{
					check_value_node_t * frontNode = pThrItemInfo->checkSrcValList.head;
					check_value_node_t * curNode = frontNode->next;
					check_value_node_t * delNode = NULL;
					while(curNode)
					{
						if(nowTime - curNode->checkValTime >= pThrItemInfo->lastingTimeS)
						{
							delNode = curNode;
							frontNode->next  = curNode->next;
							curNode = frontNode->next;
							free(delNode);
							pThrItemInfo->checkSrcValList.nodeCnt--;
							delNode = NULL;
						}
						else
						{
							frontNode = curNode;
							curNode = frontNode->next;
						}
					}
				}
			}
		}
		{
			check_value_node_t * head = pThrItemInfo->checkSrcValList.head;
			check_value_node_t * newNode = (check_value_node_t *)malloc(sizeof(check_value_node_t));
			newNode->checkValTime = nowTime;
			newNode->ckV = checkValue;
			newNode->next = head->next;
			head->next = newNode;
			pThrItemInfo->checkSrcValList.nodeCnt++;
		}
	}
	return bRet;
}

bool HandlerThreshold_CheckThrItem(thr_item_info_t * pThrItemInfo, MSG_ATTRIBUTE_T* attr, char * checkRetMsg)
{
	bool bRet = false;
	bool isTrigger = false;
	bool triggerMax = false;
	bool triggerMin = false;
	char tmpRetMsg[1024] = {0};
	char checkTypeStr[64] = {0};
	char descEventStr[256] = {0};
	if(pThrItemInfo == NULL || checkRetMsg== NULL || attr == NULL) return bRet;
	{
		switch(pThrItemInfo->checkType)
		{
		case ck_type_avg:
			{
				sprintf(checkTypeStr, DEF_AVG_EVENT_STR);
				break;
			}
		case ck_type_max:
			{
				sprintf(checkTypeStr, DEF_MAX_EVENT_STR);
				break;
			}
		case ck_type_min:
			{
				sprintf(checkTypeStr, DEF_MIN_EVENT_STR);
				break;
			}
		default: break;
		}
	}
	{
		if(HandlerThreshold_CheckSrcVal(pThrItemInfo, attr->v) && (int)pThrItemInfo->checkRetValue != DEF_INVALID_VALUE)
		{  
			if(pThrItemInfo->rangeType & range_max)
			{
				if(pThrItemInfo->maxThr != DEF_INVALID_VALUE && (pThrItemInfo->checkRetValue > pThrItemInfo->maxThr))
				{
					//sprintf(tmpRetMsg, "%d(%s:%.0f)>maxThreshold(%.0f)", pThrItemInfo->id, checkTypeStr, pThrItemInfo->checkRetValue.vf, pThrItemInfo->maxThr);
					sprintf(tmpRetMsg, "%s(%s:%f)>%s(%f)", pThrItemInfo->pathname, checkTypeStr, pThrItemInfo->checkRetValue, DEF_MAX_THR_EVENT_STR, pThrItemInfo->maxThr);
					if(strlen(descEventStr)) sprintf(tmpRetMsg, "%s:%s", tmpRetMsg, descEventStr);
					triggerMax = true;
				}
			}
			if(pThrItemInfo->rangeType & range_min)
			{
				if(pThrItemInfo->minThr != DEF_INVALID_VALUE && (pThrItemInfo->checkRetValue  < pThrItemInfo->minThr))
				{
					//if(strlen(tmpRetMsg)) sprintf(tmpRetMsg, "%s and %d(%s:%.0f)<minThreshold(%.0f)", tmpRetMsg, pThrItemInfo->id, checkTypeStr, pThrItemInfo->checkRetValue.vf, pThrItemInfo->minThr);
					//else sprintf(tmpRetMsg, "%d(%s:%.0f)<minThreshold(%.0f)", pThrItemInfo->id, checkTypeStr, pThrItemInfo->checkRetValue.vf, pThrItemInfo->minThr);
					if(strlen(tmpRetMsg)) sprintf(tmpRetMsg, "%s %s %s(%s:%f)<%s(%f)", tmpRetMsg, DEF_AND_EVENT_STR, pThrItemInfo->pathname, checkTypeStr, pThrItemInfo->checkRetValue, DEF_MIN_THR_EVENT_STR, pThrItemInfo->minThr);
					else sprintf(tmpRetMsg, "%s(%s:%f)<%s(%f)", pThrItemInfo->pathname, checkTypeStr, pThrItemInfo->checkRetValue, DEF_MIN_THR_EVENT_STR, pThrItemInfo->minThr);
					if(strlen(descEventStr)) sprintf(tmpRetMsg, "%s:%s", tmpRetMsg, descEventStr);
					triggerMin = true;
				}
			}
		}
	}

	switch(pThrItemInfo->rangeType)
	{
	case range_max:
		{
			isTrigger = triggerMax;
			break;
		}
	case range_min:
		{
			isTrigger = triggerMin;
			break;
		}
	case range_maxmin:
		{
			isTrigger = triggerMin || triggerMax;
			break;
		}
	}

	if(isTrigger)
	{
		long long nowTime = time(NULL);
		if(pThrItemInfo->intervalTimeS == DEF_INVALID_TIME || pThrItemInfo->intervalTimeS == 0 || nowTime - pThrItemInfo->repThrTime >= pThrItemInfo->intervalTimeS)
		{
			pThrItemInfo->repThrTime = nowTime;
			pThrItemInfo->isNormal = false;
			bRet = true;
			if(pThrItemInfo->on_triggered)
				pThrItemInfo->on_triggered(pThrItemInfo, attr);
		}
	}
	else
	{
		if(!pThrItemInfo->isNormal && (int)pThrItemInfo->checkRetValue != DEF_INVALID_VALUE)
		{
			memset(tmpRetMsg, 0, sizeof(tmpRetMsg));
			//sprintf(tmpRetMsg, "%d(%s:%.0f) normal", pThrItemInfo->id, checkTypeStr, pThrItemInfo->checkRetValue.vf);
			sprintf(tmpRetMsg, "%s(%s:%f) %s", pThrItemInfo->pathname, checkTypeStr, pThrItemInfo->checkRetValue, DEF_NOR_EVENT_STR);
			if(strlen(descEventStr)) sprintf(tmpRetMsg, "%s:%s", tmpRetMsg, descEventStr);
			pThrItemInfo->isNormal = true;
			bRet = true;

			if(pThrItemInfo->on_triggered)
				pThrItemInfo->on_triggered(pThrItemInfo, attr);
		}
	}

	if(!bRet) sprintf(checkRetMsg,"");
	else sprintf(checkRetMsg, "%s", tmpRetMsg);

	return bRet;
}

bool HANDLERTHRESHOLD_API HandlerThreshold_CheckThr(thr_item_list curThrItemList, MSG_CLASSIFY_T* pCapability, char ** checkRetMsg, unsigned int bufLen, bool * isNormal)
{
	bool bRet = false;
	if(curThrItemList == NULL || pCapability == NULL || checkRetMsg == NULL || isNormal == NULL || bufLen == 0) return bRet;
	{
		thr_item_node_t * curThrItemNode = NULL;
		char tmpMsg[1024] = {0};
		unsigned int defLength = bufLen;
		curThrItemNode = curThrItemList->next;
		while(curThrItemNode)
		{
			if(curThrItemNode->thrItemInfo.isEnable)
			{
				MSG_ATTRIBUTE_T* attr = IoT_FindSensorNodeWithPath(pCapability, curThrItemNode->thrItemInfo.pathname);
				if(attr == NULL || attr->type != attr_type_numeric)
				{
					if(defLength<strlen(*checkRetMsg)+strlen(curThrItemNode->thrItemInfo.pathname)+strlen(DEF_NOT_SUPT_EVENT_STR)+16)
					{
						int newLen = strlen(*checkRetMsg)+strlen(curThrItemNode->thrItemInfo.pathname)+strlen(DEF_NOT_SUPT_EVENT_STR)+2*1024;
						*checkRetMsg = (char*)realloc(*checkRetMsg, newLen);
						defLength = newLen;
					}
					if(strlen(*checkRetMsg))sprintf(*checkRetMsg, "%s;%s %s", *checkRetMsg, curThrItemNode->thrItemInfo.pathname, DEF_NOT_SUPT_EVENT_STR);
					else sprintf(*checkRetMsg, "%s %s", curThrItemNode->thrItemInfo.pathname, DEF_NOT_SUPT_EVENT_STR);
					curThrItemNode->thrItemInfo.isEnable = false;
					curThrItemNode = curThrItemNode->next;
					continue;
				}

				memset(tmpMsg, 0, sizeof(tmpMsg));
				HandlerThreshold_CheckThrItem(&curThrItemNode->thrItemInfo, attr, tmpMsg);
				if(strlen(tmpMsg))
				{
					if(bufLen<strlen(*checkRetMsg)+strlen(tmpMsg)+16)
					{
						int newLen = strlen(*checkRetMsg)+strlen(tmpMsg)+2*1024;
						*checkRetMsg = (char*)realloc(*checkRetMsg, newLen);
					}
					if(strlen(*checkRetMsg))sprintf(*checkRetMsg, "%s;%s", *checkRetMsg, tmpMsg);
					else sprintf(*checkRetMsg, "%s", tmpMsg);
				}
				if(*isNormal && !curThrItemNode->thrItemInfo.isNormal)
				{
					*isNormal = curThrItemNode->thrItemInfo.isNormal; 
				}
			}
			curThrItemNode = curThrItemNode->next;
		}
	}
	return bRet = true;
}
