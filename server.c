#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void chatMode(int connDescriptor) {
    char buffer[BUFFER_SIZE];
    while(1) {
        int n=read(connDescriptor, buffer, BUFFER_SIZE);
        if (n<=0) break;
        buffer[n]='\0';
        printf("Client: %s", buffer);
        printf("You: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        write(connDescriptor, buffer, strlen(buffer));
    }
}

void receiveFile(int connDescriptor) {
    char buffer[BUFFER_SIZE];
    FILE *fp=fopen("received_file", "wb");
    if(!fp) {
        perror("File create failed");
        return;
    }
    long received=0;
    int n;
    while((n=read(connDescriptor, buffer, BUFFER_SIZE))>0) {
        fwrite(buffer, 1, n, fp);
        received+=n;
    }
    fclose(fp);
    printf("File received successfully (%ld bytes)\n", received);
}

int main() {
    int socketDescriptor, connDescriptor;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;
    socketDescriptor=socket(AF_INET, SOCK_STREAM, 0);
    if (socketDescriptor<0) {
        perror("Server socket creation error");
        exit(1);
    }
    int opt=1;
    setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(SERVER_PORT);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(socketDescriptor, (struct sockaddr*)&servaddr, sizeof(servaddr))<0) {
        perror("Bind error");
        close(socketDescriptor);
        exit(1);
    }
    listen(socketDescriptor, 5);
    printf("Server listening on port %d...\n", SERVER_PORT);
    clilen=sizeof(cliaddr);
    connDescriptor=accept(socketDescriptor, (struct sockaddr*)&cliaddr, &clilen);
    if (connDescriptor<0) {
        perror("Accept error");
        close(socketDescriptor);
        exit(1);
    }
    char mode[5]={0};
    read(connDescriptor, mode, sizeof(mode));

    if(strncmp(mode, "CHAT", 4)==0) {
        printf("Chat mode selected\n");
        chatMode(connDescriptor);
    } 
    else if(strncmp(mode, "FILE", 4)==0) {
        printf("File transfer mode selected\n");
        receiveFile(connDescriptor);
    }

    close(connDescriptor);
    close(socketDescriptor);
    return 0;
}