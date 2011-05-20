#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N_REDES	10
#define MAX_N_HOSTS	254
#define MAX_LEN		80

enum config_opt { string=0, boolean, integer };

struct st_host_entry {
	char host[MAX_LEN];
	char hw_eth[18];
	char ddns_host[MAX_LEN];
	char option_host[MAX_LEN];
	char fixed_add[16];
};

struct st_net {
	char file[MAX_LEN];
	char name[MAX_LEN];
};

struct st_host_entry hosts[254];

struct st_net redes[MAX_N_REDES];


void dhcp_edit_init(void)
{
	int i;
        for (i=0;i<MAX_N_REDES;i++) {
		redes[i].file[0] = '\0';
		redes[i].name[0] = '\0';
	}
}

void dhcp_edit_read_cfg(void)
{
        config_t cfg, *cf;
        int i, r;
        char token[MAX_LEN];
        const char *file = NULL;
        const char *name = NULL;

        cf = &cfg;
        config_init(cf);
 
        if (!config_read_file(cf, "configuracion.cfg"))
		goto error;

        /* Buscamos redes de hosts para leer desde configuracion.cfg */
        for (i=0;i<MAX_N_REDES;i++) {

        	sprintf(token,"dhcpdconf%i", i);
		r = config_lookup_string(cf, token, &file);
                if (r == 0)
                        break;

        	sprintf(token,"network%i", i);
		r = config_lookup_string(cf, token, &name);
                if (r == 0)
                        goto error;

		strcpy(redes[i].file, file);
		strcpy(redes[i].name, name);
        }

        config_destroy(cf);
        return;
                
error:
        perror("Error al leer la configuracion\n");
        config_destroy(cf);

        exit(1);
}


/* Leer  toda la red 2, en los lugares ip donde no hay nada poner libre a todo */
/* Cuando escribimos hacemos un backup del archivo via system() */
/* Aclarar que la mascara es 24 bits */

char *remove_blanks(char *s)
{
	while ((*s == ' ') || (*s == '\t'))
		s++;	
	return s;
}

int is_empty(char *s)
{
	if ((*s == '\0') || (*s == '\n'))
		return 1;

	return 0;
}

int is_comment(char *s)
{
	if (*s == '#')
		return 1;

	return 0;
}

int is_end_of_block(char *s)
{
	if (*s == '}')
		return 1;

	return 0;
}

void dhcp_edit_init_hosts(void)
{
	int i;
        for (i=0;i<MAX_N_HOSTS;i++) {
		hosts[i].host[0] = '\0';
		hosts[i].hw_eth[0] = '\0';
		hosts[i].ddns_host[0] = '\0';
		hosts[i].option_host[0] = '\0';
		hosts[i].fixed_add[0] = '\0';
	}
}

enum token { host=0, hardware_ethernet, option_hostname, fixedaddress, ddnshostname};

int next_token(char *s, char *value)
{
	char tmp[MAX_LEN];
	char *delimit =" {;\"";
	char * next;
	char *key[] = { "host", "hardware ethernet", "option host-name", "fixed-address", "ddns-hostname" };
	int i;

	strcpy(tmp, s);
	for (i=0;i<5;i++) {
		if (!strncmp(s, key[i], strlen(key[i]))) {
			s = s + strlen(key[i]);
			break;
		}
	}

	if (i == 5) {
		printf("Encontramos un problema en la linea : %s\n", tmp);
		exit(1);
	}

	next = strtok(s, delimit);
	strcpy(value, next);

	return i;
}

void dhcp_edit_parse(char *line, int i)
{
	char answer[MAX_LEN];
	enum token tok;

	tok = next_token(line, answer);
	switch (tok) {
	case host:
		if (!is_empty(hosts[i].host))
			goto parse_error;
		strcpy(hosts[i].host, answer);
		break;
	case hardware_ethernet:
		if (!is_empty(hosts[i].hw_eth))
			goto parse_error;
		strcpy(hosts[i].hw_eth, answer);
		break;
	case option_hostname:
		if (!is_empty(hosts[i].option_host))
			goto parse_error;
		strcpy(hosts[i].option_host, answer);
		break;
	case fixedaddress:
		if (!is_empty(hosts[i].fixed_add))
			goto parse_error;
		strcpy(hosts[i].fixed_add, answer);
		break;
	case ddnshostname:
		if (!is_empty(hosts[i].ddns_host))
			goto parse_error;
		strcpy(hosts[i].ddns_host, answer);
		break;
	};
	
	return;

parse_error:
	printf("Encontramos dos valores continuos de la misma key, \
							 valor : %s\n", answer);
	exit(1);
}

void dhcp_edit_read_dhcpd(int red)
{
	int reading = 1, i;		/* reading */
	char *s;
	char line[MAX_LEN];
	FILE *f;

	f = fopen(redes[red].file, "r");
	if (f == NULL)
		goto read_dhcpd_error;

	i = 0;
	while (reading) {
		s = fgets(line, MAX_LEN, f);
		if (s == NULL) {
			reading = 0;
		} else {

			s = remove_blanks(s);

			if (is_comment(s) || is_empty(s))
				continue;

			if (is_end_of_block(s))
				i++;
			else
				dhcp_edit_parse(s, i);
		}
	}

	fclose(f);
	return;

read_dhcpd_error:
	printf("Problema al leer %s\n", redes[red].file);
	exit(1);
}

int main(void)
{
	int i;
	
	dhcp_edit_init();
	dhcp_edit_init_hosts();
	dhcp_edit_read_cfg();
	dhcp_edit_read_dhcpd(0);

        for (i=0;i<MAX_N_HOSTS;i++) {
		printf ("entrada nro %i : %s %s %s %s %s\n", i, 
		hosts[i].host,
		hosts[i].hw_eth,
		hosts[i].ddns_host,
		hosts[i].option_host,
		hosts[i].fixed_add);
	}

	exit(0);
}
