/*
 * constants.h
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <stdlib.h>

//Valores de la variable debug
#define DEBUG_ON	1
#define DEBUG_OFF	0

//Valores booleanos
#define false 0
#define true 1

//Valores de salida del programa
#define EXIT_ERROR	-1
#define EXIT_NORMAL	0
#define EXIT_NO_RESPONSE	1

//Estados del cliente
enum dhcp_states {INIT=1, SELECTING=2, REQUESTING=3, BOUND=4, RENEWING=5, REBINDING=6, REBOOTING=7, INIT_REBOOT=8 };
enum dhcp_message {DHCPDISCOVER=1, DHCPOFFER=2, DHCPREQUEST=3, DHCPACK=4, DHCPNACK=5, DHCPRELEASE=6, PID=7, IP=8, SIGINT=9, SIGUSR2=10};
//server port
#define SERVER_PORT 67
//client port
#define CLIENT_PORT 68

#endif /* CONSTANTS_H_ */
