/*
 * transfer.c
 *
 *  Created on: 06-nov-2008
 *      Author: dconde
 */

#include "transfer.h"

//Metodos internos
int sendMSG(struct msg_dhcp_t *message);
int sendETH_Msg(struct mdhcp_t*, in_addr_t address);
int sendUDP_Msg(struct mdhcp_t*, in_addr_t address);

int sock_packet; // Socket de envio / recepción a nivel packet
struct sockaddr_ll addr_packet; // Dirección de envio

int init_sockets() {
	int ret = 0;
	// Creación del socket de recepción
	sock_packet = socket(PF_PACKET,SOCK_DGRAM, htons(ETH_P_IP));
	if (sock_packet < 0) {
		perror("socket");
		ret = -1;
	}
	if (ret >= 0) {
		// Definición de la dirección de recepción
		bzero(&addr_packet, sizeof(struct sockaddr_ll));
		addr_packet.sll_family = AF_PACKET;
		addr_packet.sll_addr[0] = 255;
		addr_packet.sll_addr[1] = 255;
		addr_packet.sll_addr[2] = 255;
		addr_packet.sll_addr[3] = 255;
		addr_packet.sll_addr[4] = 255;
		addr_packet.sll_addr[5] = 255;
		addr_packet.sll_addr[6] = 0;
		addr_packet.sll_addr[7] = 0;
		addr_packet.sll_halen = 6;
		addr_packet.sll_ifindex = obtain_ifindex();
		if (addr_packet.sll_ifindex < 0)
			ret = -1;
		addr_packet.sll_hatype = 0xFFFF;
		addr_packet.sll_protocol = htons(ETH_P_IP); //TODO raro que funcione con ese htons
		addr_packet.sll_pkttype = PACKET_BROADCAST; // TODO no estoy seguro del broadcast

		if (addr_packet.sll_ifindex == -1) {
			ret = -1;
		}

		if (ret >= 0) {
			ret = bind(sock_packet, (struct sockaddr *) &addr_packet,
					sizeof(struct sockaddr_ll));
			if (ret < 0)
				perror("bind");
		}
	}
	return ret;
}

void close_sockets(){
	close(sock_packet);
}

int sendDHCPDISCOVER() {
	int ret = EXIT_ERROR;
	unsigned int xid;
	double r;
	struct mdhcp_t* dhcpdiscover;
	char ** options;
	int opt_size;

	// Se genera un xid aleatorio
	r = (double) random() / (double) RAND_MAX;
	xid = UINT_MAX * r;

	// Se crea la estructura del mensaje con los datos adecuados
	dhcpdiscover = new_default_mdhcp();
	dhcpdiscover->op = DHCP_OP_BOOTREQUEST;
	dhcpdiscover->hlen = 6;
	dhcpdiscover->xid = xid;
	dhcpdiscover->secs = 0; //TODO preguntar al profesor
	memcpy(dhcpdiscover->chaddr, haddress, dhcpdiscover->hlen);

	options = malloc(4);
	opt_size = getDhcpDiscoverOptions(options);
	//memcpy(&dhcpdiscover->options, *options, opt_size);
	dhcpdiscover->options = *options; //TODO mirar options?
	dhcpdiscover->opt_length = opt_size;
	// Prueba
	//print_mdhcp(dhcpdiscover); //TODO quitar

	// Se controla que el lock esté abierto
	pthread_mutex_lock(lock);
	// Se envia el mensaje dhcp discover a broadcast
	if (sendETH_Msg(dhcpdiscover, INADDR_BROADCAST) >= 0) {
		//state = SELECTING; // TODO se necesita sincronización multihilo?
		ret = true;
	} else {
		fprintf(stderr,
				"ERROR: No se ha podido mandar el mensaje dhcpdiscover.\n");
	}

	// Se libera el lock
	pthread_mutex_unlock(lock);

	free(*options);
	free(options);
	free_mdhcp(dhcpdiscover);

	return ret;
}

void * sendDHCPREQUEST(void * arg) {
	int ret = EXIT_ERROR;
	unsigned int xid;
	double r;
	struct mdhcp_t* dhcpRequest;
	char ** options;
	int opt_size;
	struct offerIP selected_ip;

	// Se copian los parámetros y se libera el lock
	memcpy(&selected_ip, arg, sizeof(struct offerIP));

	pthread_mutex_unlock(lock_params);

	printf("enviando dhcpRequest\n");
	// Se genera un xid aleatorio
	r = (double) random() / (double) RAND_MAX;
	xid = UINT_MAX * r;

	// Se crea la estructura del mensaje con los datos adecuados
	dhcpRequest = new_default_mdhcp();

	dhcpRequest->op = DHCP_OP_BOOTREQUEST;
	dhcpRequest->hlen = 6;
	dhcpRequest->xid = xid;
	dhcpRequest->secs = 0;
	memcpy(dhcpRequest->chaddr, haddress, dhcpRequest->hlen);

	options = malloc(4);
	opt_size = getDhcpRequestOptions(options, &selected_ip);
	dhcpRequest->options = (*options);
	dhcpRequest->opt_length = opt_size;

	// Se envia el mensaje dhcp request a broadcast
	if (sendETH_Msg(dhcpRequest, INADDR_BROADCAST) >= 0) {
		ret = true;
	} else {
		fprintf(stderr,	"ERROR: No se ha podido mandar el mensaje dhcpRequest.\n");
	}

	free(*options);
	free(options);
	free_mdhcp(dhcpRequest);

	return (void*) ret;
}

int sendETH_Msg(struct mdhcp_t *dhcpStuct, in_addr_t address) {
	unsigned char** msg;
	size_t size;
	int ret, enviado;
	ret = 0;

	if (ret >= 0) {
		/// Definimos el mensaje, inclusion de cabeceras...
		msg = malloc(4);
		size = getETHMessage(msg, address, dhcpStuct);
		//printf("El tamaño del pakete es %d\n", size);

		// Se realiza el envio
		enviado = sendto(sock_packet, *msg, size, 0,
				(struct sockaddr *) &addr_packet, sizeof(struct sockaddr_ll));
		if (enviado == -1) {
			perror("sendto");
			ret = -1;
		}
		printf("Enviado %d\n", enviado);
	}

	return ret;
}

int sendUDP_Msg(struct mdhcp_t *message, in_addr_t address) {
	return true;
}

// Recive todos los mensajes de dhcpOffer
int get_selecting_messages(struct mdhcp_t messages[]) {
	fd_set recvset;
	struct timeval tv;
	int ret = 1;
	char * buf = malloc(1000); //TODO
	int num_dhcp = 0;

	// Se establecen los sets de descriptores
	FD_ZERO(&recvset);
	FD_SET(sock_packet, &recvset);
	// Se desbloquea el lock para que se envie el DhcpDiscover
	pthread_mutex_unlock(lock);
	//Recivimos multiples respuestas
	ret = 1;
	while (ret > 0 && num_dhcp < MAXDHCPOFFERS) {
		// Tiempo de espera del select - Hay que hacerlo en cada iteracción del buble
		tv.tv_sec = 10; // TODO Cuanto tiempo hay que esperar?
		tv.tv_usec = 0;

		ret = select(sock_packet + 1, &recvset, NULL, NULL, &tv);
		if(ret < 0) {
			perror("select");
		} else if(ret> 0) {
			int recv_size = recvfrom(sock_packet, buf, 1000, 0, NULL, NULL);
			printf("recibido %d\n",recv_size);

			get_dhcpH_from_ethM(&messages[num_dhcp], buf, recv_size);
			num_dhcp++;

			printf("ipOrigen %d\n",messages[0].siaddr);
			printf("id %d\n",messages[0].xid);
		}
	}
	ret = num_dhcp;
	free(buf);

	return ret;
}

int get_ACK_message() {
	fd_set recvset;
	struct timeval tv;
	int ret = 1;
	char * buf = malloc(1000); //TODO
	struct mdhcp_t dhcp_recv;
	int num_dhcp = 0;

	// Se establecen los sets de descriptores
	FD_ZERO(&recvset);
	FD_SET(sock_packet, &recvset);
	// Se desbloquea el lock para que se envie el DhcpDiscover
	pthread_mutex_unlock(lock);
	//Recivimos multiples respuestas
	ret = 1;
	while (ret > 0 && num_dhcp < MAXDHCPOFFERS) {
		// Tiempo de espera del select - Hay que hacerlo en cada iteracción del buble
		tv.tv_sec = 10; // TODO Cuanto tiempo hay que esperar?
		tv.tv_usec = 0;

		ret = select(sock_packet + 1, &recvset, NULL, NULL, &tv);
		if(ret < 0) {
			perror("select");
		} else if(ret> 0) {
			int recv_size = recvfrom(sock_packet, buf, 1000, 0, NULL, NULL);
			printf("recibido %d\n",recv_size);

			get_dhcpH_from_ethM(&dhcp_recv, buf, recv_size);
			num_dhcp++;

			printf("ipOrigen %d\n",dhcp_recv.siaddr);
			printf("id %d\n",dhcp_recv.xid);
			// TODO hay que ver que el mensaje sea ACK y no otra mierda
		}
	}
	ret = num_dhcp;
	free(buf);

	return ret;
}

