/*
 * transfer.c
 *
 *  Created on: 06-nov-2008
 *      Author: dconde
 */

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "constants.h"
#include "dhcp_state.h"
#include "f_messages.h"

//Metodos internos
int sendMSG(struct msg_dhcp_t *message);
int sendETH_Msg(struct mdhcp_t*, in_addr_t address);
int sendUDP_Msg(struct mdhcp_t*, in_addr_t address );

int sendDHCPDISCOVER(){
	printf("Vamos a petar\n");
	int ret = EXIT_ERROR;
	unsigned int xid;
	double r;
	struct mdhcp_t* dhcpdiscover;

	// Se genera un xid aleatorio
	r = (double)random()/(double)RAND_MAX;
	xid = UINT_MAX *r;

	// Se crea la estructura del mensaje con los datos adecuados
	dhcpdiscover = new_default_mdhcp();
	dhcpdiscover->op = DHCP_OP_BOOTREQUEST;
	dhcpdiscover->htype = 0;//TODO mirar el rfc.
	dhcpdiscover->hlen = 6;
	dhcpdiscover->xid = xid;
	dhcpdiscover->secs = 0; //TODO preguntar al profesor
	memcpy(dhcpdiscover->chaddr, haddress, dhcpdiscover->hlen);
	//dhcpdiscover->sname= ""; //TODO mirar options?
	//dhcpdiscover->file= ""; //TODO mirar options?
	dhcpdiscover->options= NULL; //TODO mirar options?
// Prueba
	print_mdhcp(dhcpdiscover); //TODO quitar

	// Se envia el mensaje dhcp discover a broadcast
	if(sendETH_Msg(dhcpdiscover, INADDR_BROADCAST) == true){
		//state = SELECTING; // TODO se necesita sincronización multihilo?
		ret = true;
		printf("guay\n");
	}else{
		fprintf(stderr,"ERROR: No se ha podido mandar el mensaje dhcpdiscover.\n");
	}

	free_mdhcp(dhcpdiscover);
	return ret;
}


int sendMSG(struct msg_dhcp_t *message){
	int ret = false;

	/*switch(state){
	case INIT:
	case SELECTING:
		ret = sendRAW_Msg(message);
		break;
	default:
		ret = sendUDP_Msg(message);
		break;
	}*/

	return ret;
}

int sendETH_Msg(struct mdhcp_t *dhcpStuct, in_addr_t address ){
	struct ip_header_t* ipHeader;
	struct udp_header_t* udpHeader;
	struct sockaddr_in	addr; // Direccion de envio
	unsigned char* msg;
	size_t size;
	int ret, sock, enviado;
	ret = 0;

	// Creamos el socket
	sock = socket (PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if(sock == -1){
		perror("socket");
		ret = -1;
	}

	// Definimos parametros de configuracion para el envio
	///Se inicia la direccion de destino
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = address;
	addr.sin_port = htons(SERVER_PORT);

	/// Definimos el mensaje, inclusion de cabeceras...
	ipHeader = new_default_ipHeader();
	udpHeader = new_default_udpHeader();
	size = getRawMessage(msg, ipHeader, udpHeader, dhcpStuct);
	printf("El tamaño del pakete es %d\n", size);


	// Se realiza el envio
	enviado = sendto(sock, msg, size, 0, (struct sockaddr *)&addr, sizeof (struct sockaddr_in));
	if(enviado == -1){
		perror("sendto");
		ret = -1;
	}
	printf("Enviado %d\n", enviado);

	return ret;
}


int sendUDP_Msg(struct mdhcp_t *message, in_addr_t address ){
	return true;
}

