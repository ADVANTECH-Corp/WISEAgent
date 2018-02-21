#include "network.h"
#include <sys/ioctl.h>
#include <netdb.h>
#include <fcntl.h>
#include <net/if.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>

void app_network_init(void)
{
   //Add code
}

int network_connect(char const * const host_name, unsigned int host_port, socket_handle *network_handle)
{
   int iRet = -1;
   //Add code
   int sock = INVALID_SOCKET;
   struct addrinfo hints;
   struct addrinfo *ainfo, *rp;
   int s;
   int opt;
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
      if(connect(sock, rp->ai_addr, rp->ai_addrlen) != -1)
      {

         break;
      }
      close(sock);
   }
   if(!rp){
      return iRet;
   }
   freeaddrinfo(ainfo);

   opt = fcntl(sock, F_GETFL, 0);
   if(opt == -1 || fcntl(sock, F_SETFL, opt | O_NONBLOCK) == -1)
   {
      close(sock);
      return iRet;
   }

   *network_handle = sock;
   iRet = 0;
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
   FD_SET(network_handle, &readfd);
   FD_ZERO(&writefd);

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

int app_network_close(socket_handle network_handle)
{
   int iRet = -1;
   //Add code
   struct linger ling_opt;

   ling_opt.l_linger = 1;
   ling_opt.l_onoff  = 1;

   setsockopt(network_handle, SOL_SOCKET, SO_LINGER, (char*)&ling_opt, sizeof(ling_opt) );

   iRet = close(network_handle);

   return iRet;
}

void network_cleanup(void)
{
   //Add code
}

int network_host_name_get(char * phostname, int size)
{
   int iRet = -1;
   char hostName[256];
   if(phostname == NULL) return iRet;
   app_network_init();
   iRet = gethostname(hostName, size);
   network_cleanup();
   if(!iRet)
   {
      strcpy(phostname, hostName);
   } 
   return iRet;
}

int network_ip_get(char * ipaddr, int size)
{
   int iRet = -1;
   int err = 0;
   int fd;
   struct ifreq ifr;
   struct ifconf ifc;
   char buf[1024];
   if(!ipaddr) return iRet;
   fd = socket( AF_INET, SOCK_DGRAM, 0 );
   if( fd == -1) return iRet;

   ifc.ifc_len = sizeof(buf);
   ifc.ifc_buf = buf;
   if (ioctl(fd, SIOCGIFCONF, &ifc) == -1)
   {
	   close(fd);
	   return iRet;
   }
   else
   {
       struct ifreq* it = ifc.ifc_req;
       const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

       for (; it != end; ++it) {
	   ifr.ifr_addr.sa_family = AF_INET;
           strcpy(ifr.ifr_name, it->ifr_name);
           if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0) {
                if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
                        err = 1;
                        break;
                    }
                }
            }
            else { printf("ioctl failed: %d\n", err); }
        }

        if (err)
        {
            sprintf(ipaddr,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
        }
   }
   
   iRet = 0;
   close(fd);
   return iRet;
}

int network_mac_get(char * macstr)
{
   int iRet = -1;
   int err = 0;
   int sock_mac;
   struct ifreq ifr_mac;
   struct ifconf ifc;
   char buf[1024];
   if(!macstr) return iRet;
   sock_mac = socket( AF_INET, SOCK_STREAM, 0 );
   if( sock_mac == -1) return iRet;

   ifc.ifc_len = sizeof(buf);
   ifc.ifc_buf = buf;
   if (ioctl(sock_mac, SIOCGIFCONF, &ifc) == -1) 
   {
	   close(sock_mac);
	   return iRet;
   }	
   else
   {
       struct ifreq* it = ifc.ifc_req;
       const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

       for (; it != end; ++it) {
           strcpy(ifr_mac.ifr_name, it->ifr_name);
           if (ioctl(sock_mac, SIOCGIFFLAGS, &ifr_mac) == 0) {
                if (! (ifr_mac.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                    if (ioctl(sock_mac, SIOCGIFHWADDR, &ifr_mac) == 0) {
                        err = 1;
                        break;
                    }
                }
            }
            else { printf("ioctl failed: %d\n", err); }
        }

        if (err)
        {
            sprintf(macstr,"%02X:%02X:%02X:%02X:%02X:%02X",
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);
        }
   }
   
   iRet = 0;
   close(sock_mac);
   return iRet;
}
int network_mac_get_ex2(char * macstr);
int network_mac_get_ex(char * macstr)
{
   int iRet = -1;
   int err = 0;
   int sock_mac;
   struct ifreq ifr_mac;
   struct ifconf ifc;
   char buf[1024];
   
   iRet = network_mac_get_ex2(macstr);
   if(iRet == 0)
	return iRet;
   
   if(!macstr) return iRet;
   sock_mac = socket( AF_INET, SOCK_STREAM, 0 );
   if( sock_mac == -1) return iRet;
   
   ifc.ifc_len = sizeof(buf);
   ifc.ifc_buf = buf;
   if (ioctl(sock_mac, SIOCGIFCONF, &ifc) == -1) 
   {
	   close(sock_mac);
	   return iRet;
   }
   else
   {
       struct ifreq* it = ifc.ifc_req;
       const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

       for (; it != end; ++it) {
           strcpy(ifr_mac.ifr_name, it->ifr_name);
           if (ioctl(sock_mac, SIOCGIFFLAGS, &ifr_mac) == 0) {
                if (! (ifr_mac.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                    if (ioctl(sock_mac, SIOCGIFHWADDR, &ifr_mac) == 0) {
                        err = 1;
                        break;
                    }
                }
            }
            else { printf("ioctl failed: %d\n", err); }
        }

        if (err)
        {
            sprintf(macstr,"%02X%02X%02X%02X%02X%02X",
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
               (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);
               iRet = 0;
        }
   }
   close(sock_mac);
   return iRet;
}
int getMAC(char *name, char * macstr, bool filter);
int network_mac_get_ex2(char * macstr)
{
   int iRet = -1;
   DIR *dir;
   struct dirent *ptr;
   dir = opendir("/sys/class/net/");
   while((ptr = readdir(dir)) != NULL)
   {
      if(strlen(ptr->d_name)<=2)
        continue;
      if(getMAC(ptr->d_name, macstr, true)!=0)
        continue;
      printf("%s MAC: %s\n", ptr->d_name, macstr);
      if(strcmp(macstr, "000000000000") == 0)
        continue;
      iRet = 0;
      break;
   }
   closedir(dir);
   return iRet;
}

int getMAC(char *name, char * macstr, bool filter) {
	FILE *f;
	char buf[128];
	char *line = NULL;
	ssize_t count;
	size_t len = 0;
	int iRet = -1;
	unsigned int addr[6] = {0};
	snprintf(buf, sizeof(buf), "/sys/class/net/%s/address", name);
	f = fopen(buf, "r");
	if(!f) {
		printf("Error opening:%s\n", buf);
		return iRet;
	}
	count = getline(&line, &len, f);
    	fclose(f);
	if (count == -1) {
		printf("Error opening:%s\n", buf);
		return iRet;
	}
	sscanf(line, "%02X:%02X:%02X:%02X:%02X:%02X\n", &addr[0],&addr[1],&addr[2],&addr[3],&addr[4],&addr[5]);
    	if(filter)
    	{
		sprintf(macstr,"%02X%02X%02X%02X%02X%02X",
			addr[0],
			addr[1],
			addr[2],
			addr[3],
			addr[4],
			addr[5]);
    	}
    	else
    	{
		sprintf(macstr,"%02X:%02X:%02X:%02X:%02X:%02X",
			addr[0],
			addr[1],
			addr[2],
			addr[3],
			addr[4],
			addr[5]);
    	}

	
	iRet = 0;
	return iRet;
}
int network_mac_list_get_ex2(char macsStr[][20], int n);
int network_mac_list_get(char macsStr[][20], int n)
{
   int sock_mac;
   struct ifreq ifr_mac;
   struct ifconf ifc;
   char buf[1024];
   int cnt = 0;
   
   if(!macsStr) return cnt;

   cnt = network_mac_list_get_ex2(macsStr, n);
   if(cnt > 0)
	return cnt;

   sock_mac = socket( AF_INET, SOCK_STREAM, 0 );
   if( sock_mac == -1) return cnt;
   
   ifc.ifc_len = sizeof(buf);
   ifc.ifc_buf = buf;
   if (ioctl(sock_mac, SIOCGIFCONF, &ifc) == -1)
   {
	   close(sock_mac);
	   return cnt;
   }	   
   else
   {
       struct ifreq* it = ifc.ifc_req;
       const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
       for (; it != end; ++it) {
           strcpy(ifr_mac.ifr_name, it->ifr_name);
           if (ioctl(sock_mac, SIOCGIFFLAGS, &ifr_mac) == 0) {
                if (! (ifr_mac.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                    if (ioctl(sock_mac, SIOCGIFHWADDR, &ifr_mac) == 0) {
                        sprintf(macsStr[cnt],"%02X:%02X:%02X:%02X:%02X:%02X",
							(unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
							(unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
							(unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
							(unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
							(unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
							(unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);
						cnt++;
						if(cnt >= n) break;
                    }
                }
            }
            else { printf("ioctl failed\n"); }
        }
   }
   close(sock_mac);
   return cnt;
}

int network_mac_list_get_ex2(char macsStr[][20], int n)
{
   int cnt = 0;
   DIR *dir;
   struct dirent *ptr;
   dir = opendir("/sys/class/net/");
   while((ptr = readdir(dir)) != NULL)
   {
      char macStr[18] = {0};
      if(strlen(ptr->d_name)<=2)
	    continue;
      if(getMAC(ptr->d_name, macStr, false) !=0)
        continue;
      printf("%s MAC: %s\n", ptr->d_name, macStr);
      if(strcmp(macStr, "00:00:00:00:00:00") == 0)
        continue;
      strcpy(macsStr[cnt], macStr);
      cnt++;
      if(cnt >= n) break;
   }
   closedir(dir);
   return cnt;
}

int network_local_ip_get(int socket, char* clientip, int size)
{
	struct sockaddr_storage addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	//int res = getpeername(socket, (struct sockaddr *)&addr, &addr_size); //server IP
	int res = getsockname(socket, (struct sockaddr *)&addr, &addr_size);
	if(res == 0)
	{
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		inet_ntop(addr.ss_family, &s->sin_addr, clientip, size);
	}
	return res;
}
bool network_magic_packet_send(char * mac, int size)
{
	bool bRet = false;
	if(size < 6) return bRet;
	{
		unsigned char packet[102] = {0};
		unsigned char *message_ptr = packet;
		struct sockaddr_in addr;
		int sockfd;
		int i = 0;
		int optVal = 1;

		memset(message_ptr, 0xFF, 6);
		message_ptr += 6;
		for (i = 0; i < 16; ++i) {
		    memcpy(message_ptr, mac, 6);
		    message_ptr += 6;
		}
		sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(sockfd > 0)
		{
			int iRet = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,(char *)&optVal, sizeof(optVal));
			if(iRet == 0)
			{
				printf("Sendto\n");
				memset((void*)&addr, 0, sizeof(addr));
				addr.sin_family = AF_INET;
				addr.sin_port = htons(9);
				addr.sin_addr.s_addr= INADDR_BROADCAST;
				iRet = sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr));
				if(iRet != -1)
					bRet = true;
				else
					printf("Sendto fail\n");
			}
			else
				printf("setsockopt fail:%s\n", strerror(errno));
			close(sockfd);
		}
	}
	return bRet;
}
