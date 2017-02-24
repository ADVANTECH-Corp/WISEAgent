#include "network.h"
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

void network_init(void)
{
	//Add code
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
}

int network_close(socket_handle network_handle)
{
	int iRet = -1;
	//Add code
	struct linger ling_opt;

	ling_opt.l_linger = 1;
	ling_opt.l_onoff  = 1;
	setsockopt(network_handle, SOL_SOCKET, SO_LINGER, (char*)&ling_opt, sizeof(ling_opt) );
	iRet = closesocket(network_handle);
	return iRet;
}

int network_connect(char const * const host_name, unsigned int host_port, socket_handle *network_handle)
{
	int iRet = -1;
	//Add code
	int sock = INVALID_SOCKET;
	struct addrinfo hints;
	struct addrinfo *ainfo = NULL, *rp = NULL;
	int s;
	unsigned int val = 1;
	if(host_name == NULL) return iRet;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = PF_UNSPEC;
	hints.ai_flags = AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;

	s = getaddrinfo(host_name, NULL, &hints, &ainfo);
	if(s)
	{
		return iRet;
	}
	for(rp = ainfo; rp != NULL; rp = rp->ai_next)
	{
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sock == INVALID_SOCKET) continue;

		if(rp->ai_family == PF_INET)
		{
			((struct sockaddr_in *)rp->ai_addr)->sin_port = htons(host_port);
		}
		else if(rp->ai_family == PF_INET6)
		{
			((struct sockaddr_in6 *)rp->ai_addr)->sin6_port = htons(host_port);
		}else{
			continue;
		}
		if(connect(sock, rp->ai_addr, rp->ai_addrlen) != -1){
			break;
		}
		//shutdown(sock, SD_BOTH);
		network_close(sock);
		//closesocket(sock);
	}
	if(!rp){
		goto done;
	}

	if(ioctlsocket(sock, FIONBIO, &val)){
		network_close(sock);
		//closesocket(sock);
		goto done;
	}

	*network_handle = sock;
	iRet = 0;
done:
	freeaddrinfo(ainfo);
	return iRet;
}


network_status_t network_waitsock(socket_handle network_handle, int mode, int timeoutms)
{
	int waitret = 0;
	struct timeval timeout;
	int rc = -1;
	fd_set readfd, writefd;
	if(timeoutms >= 0)
	{
		timeout.tv_sec = timeoutms/1000;
		timeout.tv_usec = (timeoutms%1000) * 1000; 
	}
	else
	{
		timeout.tv_sec = 1;
		timeout.tv_usec = 0; 
	}

	FD_ZERO(&readfd);
	FD_ZERO(&writefd);

	if (mode & network_waitsock_read)
		FD_SET(network_handle, &readfd);

	if (mode & network_waitsock_write)
		FD_SET(network_handle, &writefd);

	rc = select(network_handle+1, &readfd, &writefd, NULL, &timeout);

	if (-1 == rc)
	{
		return network_waitsock_error;
	}

	if (FD_ISSET(network_handle, &readfd))
	{
		waitret |= network_waitsock_read;
	}
	if (FD_ISSET(network_handle, &writefd))
	{
		waitret |= network_waitsock_write;
	}
	if(waitret > 0) return waitret;
	return network_waitsock_timeout;
}

int network_send(socket_handle network_handle, char const * sendbuffer, unsigned int len)
{
	int iRet = 0;
	//Add code
	iRet = send(network_handle, (char *)sendbuffer, len, 0);

	return iRet;
}

int network_recv(socket_handle network_handle, char * recvbuffer, unsigned int len)
{
	int iRet = -1;
	//Add code
	iRet = recv(network_handle, (char *)recvbuffer, (int)len, 0);

	return iRet;
}

void network_cleanup(void)
{
	//Add code
	//WSACleanup();
}

int network_host_name_get(char * phostname, int size)
{
	int iRet = -1;
	char hostName[256] = {0};
	if(phostname == NULL) return iRet;
	network_init();
	iRet = gethostname(hostName, size);
	network_cleanup();
	if(!iRet)
	{
		strcpy(phostname, hostName);
	} 
	return iRet;
}

int network_ip_get(char * pipaddr, int size)
{
	int iRet = -1;
	char ipaddr[16] = {0};
	char hostName[256] = {0};
	struct hostent *phe = NULL;
	if(pipaddr == NULL) return iRet;
	network_init();
	iRet = gethostname(hostName, sizeof(hostName));
	if(!iRet)
	{
		struct in_addr addr;
		phe = gethostbyname(hostName);
		if (phe == 0) {
			iRet = -1;
			return iRet;
		}
		if(phe->h_addr_list[0] == 0) {
			iRet = -1;
			return iRet;
		}
		memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr));
		strcpy(pipaddr, inet_ntoa(addr));
		iRet = 0;
	}
	return iRet;
}

int network_mac_get(char * macstr)
{
	// Use IPHlpApi	
	int iRet = -1;
#ifdef MULTI_AGENT_RUN
	if(macstr == NULL) return iRet;
	memcpy(macstr, MultiAgentMac, strlen(MultiAgentMac) + 1);
	iRet = 0;
#else
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdInfo = NULL;
	ULONG            ulSizeAdapterInfo = 0;  
	DWORD            dwStatus;  

	MIB_IFROW MibRow = {0};  
	ulSizeAdapterInfo = sizeof(IP_ADAPTER_INFO); 
	pAdapterInfo = malloc(ulSizeAdapterInfo);   

	if (GetAdaptersInfo( pAdapterInfo, &ulSizeAdapterInfo) != ERROR_SUCCESS) 
	{
		free (pAdapterInfo);
		pAdapterInfo = malloc(ulSizeAdapterInfo);   
	}

	dwStatus = GetAdaptersInfo(pAdapterInfo, &ulSizeAdapterInfo);  

	if(dwStatus != ERROR_SUCCESS)  
	{  
		free(pAdapterInfo);  
		return  iRet;  
	}  

	pAdInfo = pAdapterInfo; 
	while(pAdInfo)  
	{  	
		memset(&MibRow, 0, sizeof(MIB_IFROW));
		MibRow.dwIndex = pAdInfo->Index;  
		MibRow.dwType = pAdInfo->Type;  

		if(GetIfEntry(&MibRow) == NO_ERROR)  
		{  
			//          if (MibRow.dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL && 
			//             strlen(pAdInfo->GatewayList.IpAddress.String) != 0)
			if (MibRow.dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL)
			{

				sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X",
					MibRow.bPhysAddr[0],
					MibRow.bPhysAddr[1],
					MibRow.bPhysAddr[2],
					MibRow.bPhysAddr[3],
					MibRow.bPhysAddr[4],
					MibRow.bPhysAddr[5]);

				iRet = 0;
				break;
			}
		}
		pAdInfo = pAdInfo->Next; 
	}
	if(pAdapterInfo)free(pAdapterInfo); 
#endif
	return iRet;
}

int network_mac_get_ex(char * macstr)
{
	// Use IPHlpApi	
	int iRet = -1;
#ifdef MULTI_AGENT_RUN

	if(macstr == NULL) return iRet;
	memcpy(macstr, MultiAgentMac, strlen(MultiAgentMac) + 1);
	iRet = 0;
#else
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	PIP_ADAPTER_INFO pAdInfo = NULL;
	ULONG            ulSizeAdapterInfo = 0;  
	DWORD            dwStatus;  

	MIB_IFROW MibRow = {0};  
	ulSizeAdapterInfo = sizeof(IP_ADAPTER_INFO); 
	pAdapterInfo = malloc(ulSizeAdapterInfo);   


	if (GetAdaptersInfo( pAdapterInfo, &ulSizeAdapterInfo) != ERROR_SUCCESS) 
	{
		free (pAdapterInfo);
		pAdapterInfo = malloc(ulSizeAdapterInfo);   
	}

	dwStatus = GetAdaptersInfo(pAdapterInfo, &ulSizeAdapterInfo);  

	if(dwStatus != ERROR_SUCCESS)  
	{  
		free(pAdapterInfo);  
		return  iRet;  
	}  

	pAdInfo = pAdapterInfo; 
	while(pAdInfo)  
	{  	
		memset(&MibRow, 0, sizeof(MIB_IFROW));
		MibRow.dwIndex = pAdInfo->Index;  
		MibRow.dwType = pAdInfo->Type;  

		if(GetIfEntry(&MibRow) == NO_ERROR)  
		{  
			//          if (MibRow.dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL && 
			//             strlen(pAdInfo->GatewayList.IpAddress.String) != 0)
			if (MibRow.dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL)
			{

				sprintf(macstr, "%02X%02X%02X%02X%02X%02X",
					MibRow.bPhysAddr[0],
					MibRow.bPhysAddr[1],
					MibRow.bPhysAddr[2],
					MibRow.bPhysAddr[3],
					MibRow.bPhysAddr[4],
					MibRow.bPhysAddr[5]);

				iRet = 0;
				break;
			}
		}
		pAdInfo = pAdInfo->Next; 
	}
	if(pAdapterInfo)free(pAdapterInfo); 
#endif
	return iRet;
}

int network_mac_list_get(char macsStr[][20], int n)
{
	int iRet = -1;
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	PIP_ADAPTER_INFO pAdInfo = NULL;
	ULONG            ulSizeAdapterInfo = 0;  
	DWORD            dwStatus;  

	MIB_IFROW MibRow = {0}; 
	int macIndex = 0;
	if(n <= 0) return iRet;
	ulSizeAdapterInfo = sizeof(IP_ADAPTER_INFO); 
	pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulSizeAdapterInfo);


	if (GetAdaptersInfo( pAdapterInfo, &ulSizeAdapterInfo) != ERROR_SUCCESS) 
	{
		free (pAdapterInfo);
		pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulSizeAdapterInfo);
	}

	dwStatus = GetAdaptersInfo(pAdapterInfo,   &ulSizeAdapterInfo);  

	if(dwStatus != ERROR_SUCCESS)  
	{  
		free(pAdapterInfo);  
		return  iRet;  
	}  

	pAdInfo = pAdapterInfo; 
	while(pAdInfo)  
	{  	
		if(pAdInfo->Type != MIB_IF_TYPE_ETHERNET && pAdInfo->Type != IF_TYPE_IEEE80211) 
		{
			pAdInfo = pAdInfo->Next; 
			continue;
		}

		memset(&MibRow, 0, sizeof(MIB_IFROW));
		MibRow.dwIndex = pAdInfo->Index;  
		MibRow.dwType = pAdInfo->Type;  

		if(GetIfEntry(&MibRow) == NO_ERROR)  
		{  
			//if (MibRow.dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL)
			{

				sprintf(macsStr[macIndex], "%02X:%02X:%02X:%02X:%02X:%02X",
					MibRow.bPhysAddr[0],
					MibRow.bPhysAddr[1],
					MibRow.bPhysAddr[2],
					MibRow.bPhysAddr[3],
					MibRow.bPhysAddr[4],
					MibRow.bPhysAddr[5]);
				macIndex++;
				if(macIndex >= n) break;
			}
		}
		pAdInfo = pAdInfo->Next; 
	}
	free(pAdapterInfo); 
	iRet = macIndex;
	return iRet;
}

int network_local_ip_get(int socket, char* clientip, int size)
{
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	//int res = getpeername(socket, (struct sockaddr *)&addr, &addr_size); //server IP
	int res = getsockname(socket, (struct sockaddr *)&addr, &addr_size);
	if(res == 0)
	{
		strcpy(clientip, inet_ntoa(addr.sin_addr));
	}
	return res;
}

bool network_magic_packet_send(char * mac, int size)
{
	bool bRet = false;
	if(size < 6) return bRet;
	{
		unsigned char packet[102];
		struct sockaddr_in addr;
		SOCKET sockfd;
		int i = 0, j = 0;
		bool optVal = true;
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		for(i=0;i<6;i++)  
		{
			packet[i] = 0xFF;  
		}
		for(i=1;i<17;i++)
		{
			for(j=0;j<6;j++)
			{
				packet[i*6+j] = mac[j];
			}
		}

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd > 0)
		{
			int iRet = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,(char *)&optVal, sizeof(optVal));
			if(iRet == 0)
			{
				memset((void*)&addr, 0, sizeof(addr));
				addr.sin_family = AF_INET;
				addr.sin_port = htons(9);
				addr.sin_addr.s_addr=INADDR_BROADCAST;
				iRet = sendto(sockfd, (const char *)packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr));
				if(iRet != SOCKET_ERROR) bRet = TRUE;
			}
			closesocket(sockfd);
		}
		WSACleanup();
	}
	return bRet;
}