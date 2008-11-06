/*
 * utils.c
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <string.h>
#include <stdlib.h>

#include "constants.h"
#include "dhcp_state.h"

char * getTimestamp(){
	char* timestamp;
	time_t t;
	struct tm *local, *gtm;
	int h, m;

	timestamp = malloc(50);
	t=time(NULL);
	local=localtime(&t);
	gtm=gmtime(&t);
	h = (int)local->tm_gmtoff;
	m = (h/60)%60; //TODO no funciona?
	h = h/3600;
	if(h>=0)
		strftime(timestamp, 50, "%Y-%m-%d %H:%M:%S+", local);
	else
		strftime(timestamp, 50, "%Y-%m-%d %H:%M:%S-", local);

	sprintf(timestamp,"%s%.2d:%.2d",timestamp,h,m);
	return timestamp;
}

void printTrace(int xid, enum dhcp_message state, char* str){
	char *timestamp;
	timestamp = getTimestamp();

	if(xid == -1){

	}else if(xid == -2){

	}else if(xid == -3){

	}else{
		switch(state){
		case DHCPDISCOVER:
			fprintf(stdout,"#[%s] (%d) DHCPDISCOVER sent.\n",timestamp,xid);
			break;
		case DHCPOFFER:
			fprintf(stdout,"#[%s] (%d) DHCPOFFER received from %s.\n",timestamp,xid,str);
			break;
		case DHCPREQUEST:
			fprintf(stdout,"#[%s] (%d) DHCPREQUEST sent to %s.\n",timestamp,xid,str);
			break;
		case DHCPACK:
			fprintf(stdout,"#[%s] (%d) DHCPACK received: %s.\n",timestamp,xid,str);
			break;
		case DHCPNACK:
			fprintf(stdout,"#[%s] (%d) DHCPNACK received.\n",timestamp,xid);
			break;
		case DHCPRELEASE:
			fprintf(stdout,"#[%s] (%d) DHCPRELEASE sent %s.\n",timestamp,xid,str);
			break;
		case PID:
			fprintf(stdout,"#[%s] PID=%d.\n",timestamp,getpid());
			break;
		case IP:
			fprintf(stdout,"#[%s] IP %s.\n",timestamp,str);
			break;
		case SIGINT:
			fprintf(stdout,"#[%s] SIGINT received.\n",timestamp);
			break;
		case SIGUSR2:
			fprintf(stdout,"#[%s] SIGUSR2 received.\n",timestamp);
			break;
		}
	}
	free(timestamp);
}

void time_wait(int microsec){
	unsigned long time = microsec * 1000;
	//TODO quitar el comentario
	//usleep(time);
	time++;
}

void obtainHardwareAddress(){
	int fd;
	struct ifreq ifr;

	fd = socket(PF_INET, SOCK_PACKET, htons(ETH_P_ALL));
	strcpy(ifr.ifr_name, iface);
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	if(haddress == NULL)
		haddress = malloc(6);

	bzero(haddress, 6);
	memcpy(haddress,ifr.ifr_hwaddr.sa_data, 6);
	close(fd);
}
