#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <poll.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <ip_address> <port> [<payload_file>]\n", argv[0]);
        return 1;
    }

    char *ip_address = argv[1];
    int port = atoi(argv[2]);
    char *payload_file = (argc > 3) ? argv[3] : "payload.bin";
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return 1;
    }
    
    printf("Connecting to %s:%d...\n", ip_address, port);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }
    printf("Connected successfully!\n");
    
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    struct pollfd pfd;
    pfd.fd = sock;
    pfd.events = POLLIN;
    
    if (poll(&pfd, 1, 2000) > 0) {
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Server: %s", buffer);
        }
    }
    
    FILE *file = fopen(payload_file, "rb");
    if (file == NULL) {
        perror("Failed to open payload file");
        close(sock);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *payload = (char *)malloc(file_size + 1);
    if (payload == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        close(sock);
        return 1;
    }
    
    fread(payload, file_size, 1, file);
    payload[file_size] = '\0';
    fclose(file);
    
    printf("Sending payload (%ld bytes)...\n", file_size);
    
    if (send(sock, payload, file_size, 0) < 0) {
        perror("Send failed");
        free(payload);
        close(sock);
        return 1;
    }
    
    printf("Payload sent! Waiting for response...\n");

    int total_received = 0;
    int attempt;
    
    for (attempt = 0; attempt < 5; attempt++) {
        if (poll(&pfd, 1, 1000) > 0) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                printf("Server response: %s", buffer);
                total_received += bytes_received;
            } else if (bytes_received == 0) {
                printf("Connection closed by server.\n");
                break;
            } else {
                perror("Receive error");
                break;
            }
        } else {
            break;
        }
    }
    
    if (total_received == 0) {
        printf("No response received from server.\n");
    }
    
    free(payload);
    close(sock);
    printf("Connection closed.\n");
    
    return 0;
}