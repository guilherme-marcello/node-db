#ifndef _ADDRESS_H
#define _ADDRESS_H /* Módulo address */

/**
 * Retrieves the IP address of the running machine.
 *
 * @return A dynamically allocated string containing the IPv4 address,
 *         or NULL if an error occurs or no suitable address is found.
 *
 * @remarks The caller is responsible for freeing the memory allocated for the IP address.
 */
char* get_ip_address();


#endif
