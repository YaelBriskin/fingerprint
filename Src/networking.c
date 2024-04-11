// #include "../Inc/networking.h"

// void configureNetwork()
// {
//     // Настройка IP-адреса, маски подсети и шлюза
//     char command[100];
//     sprintf(command, "ifconfig %s %s netmask %s up", INTERFACE_NAME, IP_ADDRESS, NETMASK);
//     if (system(command) == -1) {
//         perror("Error setting up IP address");
//         exit(EXIT_FAILURE);
//     }

//     sprintf(command, "route add default gw %s", GATEWAY);
//     if (system(command) == -1) {
//         perror("Error setting up gateway");
//         exit(EXIT_FAILURE);
//     }
//     printf("The network interface has been successfully configured.\n");
//     return 0;
// }