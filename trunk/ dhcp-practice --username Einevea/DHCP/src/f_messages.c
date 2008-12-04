/*
 * f_messages.c
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#include "f_messages.h"
#include "constants.h"
#include "dhcp_state.h"

//METODOS PRIVADOS
char* StrToHexStr(char *str, int leng);

int get_UDPhdr(unsigned char **, struct msg_dhcp_t*);
int get_IPhdr(unsigned char **, in_addr_t, unsigned char *, int);
unsigned short in_cksum(unsigned short *ptr, int nbytes);

//CODIGO
int mdhcp_to_message_size(struct mdhcp_t *str_dhcp) {
	int ret;
	ret = str_dhcp->opt_length + DHCP_BSIZE; //En octetos
	return ret;
}

/*
 * Se reserva memoria
 */
struct msg_dhcp_t* from_mdhcp_to_message(struct mdhcp_t *str_dhcp) {
	int size, p, iaux;
	short saux;
	unsigned char *msg;
	struct msg_dhcp_t * ret;

	ret = malloc(sizeof(struct msg_dhcp_t));

	size = mdhcp_to_message_size(str_dhcp); //En octetos

	msg = malloc(size);
	p = 0;
	memcpy(msg, &str_dhcp->op, 1);
	p += 1;
	memcpy(msg + p, &str_dhcp->htype, 1);
	p += 1;
	memcpy(msg + p, &str_dhcp->hlen, 1);
	p += 1;
	memcpy(msg + p, &str_dhcp->hops, 1);
	p += 1;
	iaux = htonl(str_dhcp->xid);
	memcpy(msg + p, &iaux, 4);
	p += 4;
	saux = htons(str_dhcp->secs);
	memcpy(msg + p, &saux, 2);
	p += 2;
	saux = htons(str_dhcp->flags);
	memcpy(msg + p, &saux, 2);
	p += 2;
	iaux = htonl(str_dhcp->ciaddr);
	memcpy(msg + p, &iaux, 4);
	p += 4;
	iaux = htonl(str_dhcp->yiaddr);
	memcpy(msg + p, &iaux, 4);
	p += 4;
	iaux = htonl(str_dhcp->siaddr);
	memcpy(msg + p, &iaux, 4);
	p += 4;
	iaux = htonl(str_dhcp->giaddr);
	memcpy(msg + p, &iaux, 4);
	p += 4;
	memcpy(msg + p, &str_dhcp->chaddr, 16);
	p += 16;
	memcpy(msg + p, &str_dhcp->sname, 64);
	p += 64;
	memcpy(msg + p, &str_dhcp->file, 128);
	p += 128;
	memcpy(msg + p, str_dhcp->options, str_dhcp->opt_length);

	ret->length = size;
	ret->msg = msg;

	return ret;
}

struct mdhcp_t* new_default_mdhcp() {
	struct mdhcp_t *ret;
	int s;
	ret = malloc(sizeof(struct mdhcp_t));
	ret->op = 0;
	ret->htype = 1;
	ret->hlen = 0;
	ret->hops = 0;
	ret->xid = 0;
	ret->secs = 0;
	ret->flags = 0;
	ret->ciaddr = 0;
	ret->yiaddr = 0;
	ret->siaddr = 0;
	ret->giaddr = 0;
	s = strlen(ret->chaddr);
	bzero(ret->chaddr, s);
	s = strlen(ret->sname);
	bzero(ret->sname, s);
	s = strlen(ret->file);
	bzero(ret->file, s);
	ret->opt_length = 0;
	ret->options = NULL;

	return ret;
}

struct mdhcp_t* from_message_to_mdhcp(struct msg_dhcp_t *message) {
	int p, osize, iaux;
	short saux;
	struct mdhcp_t *ret;
	unsigned char *msg;

	ret = malloc(sizeof(struct mdhcp_t));
	msg = message->msg;
	osize = message->length - DHCP_BSIZE;

	p = 0;
	memcpy(&ret->op, msg, 1);
	p += 1;
	memcpy(&ret->htype, msg + p, 1);
	p += 1;
	memcpy(&ret->hlen, msg + p, 1);
	p += 1;
	memcpy(&ret->hops, msg + p, 1);
	p += 1;
	memcpy(&iaux, msg + p, 4);
	ret->xid = ntohl(iaux);
	p += 4;
	memcpy(&saux, msg + p, 2);
	ret->secs = ntohs(saux);
	p += 2;
	memcpy(&saux, msg + p, 2);
	ret->flags = ntohs(saux);
	p += 2;
	memcpy(&iaux, msg + p, 4);
	ret->ciaddr = ntohl(iaux);
	p += 4;
	memcpy(&iaux, msg + p, 4);
	ret->yiaddr = ntohl(iaux);
	p += 4;
	memcpy(&iaux, msg + p, 4);
	ret->siaddr = ntohl(iaux);
	p += 4;
	memcpy(&iaux, msg + p, 4);
	ret->giaddr = ntohl(iaux);
	p += 4;
	memcpy(&ret->chaddr, msg + p, 16);
	p += 16;
	memcpy(&ret->sname, msg + p, 64);
	p += 64;
	memcpy(&ret->file, msg + p, 128);
	p += 128;

	ret->opt_length = osize;
	if (osize > 0) {
		ret->options = malloc(osize);
		memcpy(&ret->options, msg + p, osize);
	}

	return ret;
}

void free_mdhcp(struct mdhcp_t *str_dhcp) {
	if (str_dhcp->opt_length > 0)
		free(str_dhcp->options);
	free(str_dhcp);
}
void free_message(struct msg_dhcp_t *message) {
	if (message->length > 0)
		//free(message->msg);
	free(message);
}
void print_mdhcp(struct mdhcp_t *str_dhcp) {
	char *cad, *chaddr, *sname, *file, *options, *xchaddr;
	unsigned int op, htype, hlen, hops, xid, secs, flags, ciaddr, yiaddr,
			siaddr, giaddr, opt_length;
	int size;
	size = sizeof(struct mdhcp_t) + str_dhcp->opt_length;
	cad = malloc(size);

	op = (unsigned int) str_dhcp-> op;
	htype = (unsigned int) str_dhcp-> htype;
	hlen = (unsigned int) str_dhcp-> hlen;
	hops = (unsigned int) str_dhcp-> hops;
	xid = (unsigned int) str_dhcp-> xid;
	secs = (unsigned int) str_dhcp-> secs;
	flags = (unsigned int) str_dhcp-> flags;
	ciaddr = (unsigned int) str_dhcp-> ciaddr;
	yiaddr = (unsigned int) str_dhcp-> yiaddr;
	siaddr = (unsigned int) str_dhcp-> siaddr;
	giaddr = (unsigned int) str_dhcp-> giaddr;
	opt_length = (unsigned int) str_dhcp-> opt_length;
	chaddr = str_dhcp->chaddr;
	sname = str_dhcp->sname;
	file = str_dhcp->file;
	options = str_dhcp->options;
	xchaddr = StrToHexStr(chaddr, haddress_size);
	sprintf(cad, "%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%s-%s-%s-%u-%s", op, htype,
			hlen, hops, xid, secs, flags, ciaddr, yiaddr, siaddr, giaddr,
			xchaddr, sname, file, opt_length, options);
	free(xchaddr);
	printf("---\n%s\n---\n", cad);
}

char* StrToHexStr(char *str, int leng) {
	int i;
	char *newstr = malloc((leng * 2) + 1);
	char *cpold = str;
	char *cpnew = newstr;

	for (i = 0; i < leng; i++) {
		sprintf(cpnew, "%02x", (unsigned char) (*cpold++));
		cpnew += 2;
	}
	*(cpnew) = '\0';
	return newstr;
}

void print_message(struct msg_dhcp_t *message) {
	int i, l;
	unsigned char *msg = message->msg;
	l = message->length;
	printf("---\n");
	for (i = 0; i < l; i++) {
		printf("%c", msg[i]);
	}
	printf("---\n");
}

/*
 * Obtiene el mensaje en formato char* a partir de las cabeceras
 * Establece los valores de tamaño de las cabeceras
 */
int getETHMessage(unsigned char** msg, in_addr_t hostname, struct mdhcp_t* mdhcp) {
	int total_size;
	struct msg_dhcp_t* dhcp_msg;
	unsigned char** udp_msg;

	udp_msg = malloc(4);

	total_size = 0;
	dhcp_msg = from_mdhcp_to_message(mdhcp);
	total_size = get_UDPhdr(udp_msg, dhcp_msg);
	total_size = get_IPhdr(msg, hostname,*udp_msg, total_size);
	printf("<%s>\n",*msg);
	free(*udp_msg);
	free(udp_msg);
	return total_size;
}

int get_UDPhdr(unsigned char ** msg, struct msg_dhcp_t* dhcp_msg){
	int size, p;
	struct udphdr *udph;

	size = 8 + dhcp_msg->length;
	*msg = malloc(size);
	udph = malloc(sizeof(struct udphdr));
	udph->source = htons(CLIENT_PORT);
	udph->dest = htons(SERVER_PORT);
	udph->len = htons(size);
	udph->check = 0;

	p = 0;
	memcpy(*msg, &udph->source, 2);
	p += 2;
	memcpy(*msg + p, &udph->dest, 2);
	printf("<<%s>>\n", *msg);
	p += 2;
	memcpy(*msg + p, &udph->len, 2);
	p += 2;
	memcpy(*msg + p, &udph->check, 2);
	p += 2;
	memcpy(*msg + p, dhcp_msg->msg, dhcp_msg->length);

	free(udph);

	return size;
}


int get_IPhdr(unsigned char ** msg, in_addr_t hostname, unsigned char * udp_msg, int udp_size){
	int size, p;
	u_int8_t caux;
	struct iphdr *iph;

	size = 20 + udp_size;
	*msg = malloc(size);
	iph = malloc(sizeof(struct iphdr));
	iph->version = 4;
	iph->ihl = 5;
	iph->tos = 0;
	iph->tot_len = htons(size);
	iph->id = getuid();
	printf("--%d\n", iph->id);
	iph->ttl = 255;
	iph->frag_off = 0;
	iph->protocol = IPPROTO_UDP;
	iph->saddr = 0;
	iph->daddr = hostname;
	iph->check = htons(in_cksum((unsigned short *)iph, sizeof(struct iphdr)));


	p = 0;
	caux = iph->version;
	caux = caux << 4;
	caux += iph->ihl;
	memcpy(*msg, &caux, 1);
	p += 1;
	memcpy(*msg + p, &iph->tos, 1);
	p += 1;
	memcpy(*msg + p, &iph->tot_len, 2);
	p += 2;
	memcpy(*msg + p, &iph->id, 2);
	p += 2;
	memcpy(*msg + p, &iph->frag_off, 2);
	p += 2;
	memcpy(*msg + p, &iph->ttl, 1);
	p += 1;
	memcpy(*msg + p, &iph->protocol, 1);
	p += 1;
	memcpy(*msg + p, &iph->check, 2);
	p += 2;
	memcpy(*msg + p, &iph->saddr, 4);
	p += 4;
	memcpy(*msg + p, &iph->daddr, 4);
	p += 4;

	memcpy(*msg + p, udp_msg, udp_size);

	free(iph);
	printf("<%s>\n",*msg);
	return size;
}

unsigned short in_cksum(unsigned short *ptr, int nbytes){
	register long sum;
	u_short oddbyte;
	register u_short answer;

	sum = 0;
	while(nbytes > 1){
		sum += *ptr++;
		nbytes -= 2;
	}

	if(nbytes == 1){
		oddbyte = 0;
		*((u_char *) &oddbyte) = *(u_char *)ptr;
		sum += oddbyte;
	}

	sum  = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;

	return answer;
}

int getDhcpOptions(char** opt, int type){
	//DHCPDISCOVER
	int p, size;
	u_int8_t magic_c [4];
	u_int8_t msg_type [3];
	u_int8_t end_opt;

	magic_c[0] = 99;
	magic_c[1] = 130;
	magic_c[2] = 83;
	magic_c[3] = 99;

	msg_type[0] = 0x35;
	msg_type[1] = 0x1;
	msg_type[2] = 0x1;


	end_opt = 0xFF;

	size = 64;
	*opt = malloc(size);

	bzero(*opt, size);
	p = 0;
	memcpy(*opt, magic_c, 4);
	p += 4;
	memcpy(*opt + p, msg_type, 3);
	p += 3;
	memcpy(*opt + p, &end_opt, 1);
	p += 1;

	return size;
}

