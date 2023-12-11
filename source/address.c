#include "address.h"

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <ifaddrs.h>

// Function to get the IP address of the running machine
char* get_ip_address() {
    struct ifaddrs *ifaddr, *ifa;
    char *ip_address = NULL;

    // retrieve information about network interfaces
    if (assert_error(
        getifaddrs(&ifaddr) == -1,
        "get_ip_address",
        "Failed to retrieve information about network interfaces\n"
    )) return NULL;

    // iterate through the list of network interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        // skip interfaces with no address, interfaces that are not up, or loopback interfaces
        if (ifa->ifa_addr == NULL || (ifa->ifa_flags & IFF_UP) == 0 || (ifa->ifa_flags & IFF_LOOPBACK) != 0) {
            continue;
        }

        // check for IPv4 addresses
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            
            // duplicate and return the first non-loopback IPv4 address found
            ip_address = strdup(inet_ntoa(sa->sin_addr));
            break;
        }
    }

    // free memory allocated by getifaddrs
    freeifaddrs(ifaddr);
    return ip_address;
}