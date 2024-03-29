/*
 * transfer.h
 *
 *  Created on: 06-nov-2008
 *       Author: Daniel Conde y Adrián López
 */

#ifndef TRANSFER_H_
#define TRANSFER_H_

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "constants.h"
#include "dhcp_state.h"
#include "utils.h"
#include "f_messages.h"

int sendDHCPDISCOVER();
int sendDHCPREQUEST();
int sendDHCPRELEASE();
int get_selecting_messages(struct mdhcp_t messages[]);
int get_ACK_message();
int init_sockets();
void close_sockets();


#endif /* TRANSFER_H_ */
