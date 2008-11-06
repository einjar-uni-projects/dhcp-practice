/*
 * dhcp_state.h
 *
 *  Created on: 06-nov-2008
 *      Author: dconde
 */

#ifndef DHCP_STATE_H_
#define DHCP_STATE_H_
#include "constants.h"

char *iface, *hostname, *address, *haddress;
enum dhcp_states state;
int exit_value, timeout, lease, debug, haddress_size;

#endif /* DHCP_STATE_H_ */
