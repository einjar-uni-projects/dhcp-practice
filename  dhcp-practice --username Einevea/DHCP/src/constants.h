/*
 * constants.h
 *
 *  Created on: 21-oct-2008
 *      Author: Daniel Conde y Adrián López
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <stdlib.h>

//Valores de la variable debug
#define DEBUG_ON	1
#define DEBUG_OFF	0

//Valores booleanos
#define FALSE 0
#define TRUE 1

//Valores de salida del programa
#define EXIT_ERROR	-1
#define EXIT_NORMAL	0
#define EXIT_NO_RESPONSE	1

//Estados del cliente
enum dhcp_message {DHCPDISCOVER=1, DHCPOFFER=2, DHCPREQUEST=3, DHCPACK=4, DHCPNAK=5, DHCPRELEASE=6, PID=7, IP=8, DHCPSIGINT=9, DHCPSIGUSR2=10};
//server port
#define SERVER_PORT 67
//client port
#define CLIENT_PORT 68

// Número máximo de ofertas que se escuchan.
#define MAXDHCPOFFERS 10

#endif /* CONSTANTS_H_ */
