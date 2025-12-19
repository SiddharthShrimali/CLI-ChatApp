#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

void sendFile(int sockDescriptor) {
    char buffer[BUFFER_SIZE];
    char fileName[256];
    printf("Enter filename to send: ");
    scanf("%s", fileName);
    FILE *fp=fopen(fileName, "rb");
    if(!fp) {
        perror("File open failed");
        return;
    }
    struct stat st;
    stat(fileName, &st);
    long fileSize=st.st_size;
    char header[BUFFER_SIZE];
    long sent=0;
    while(!feof(fp)) {
        int n=fread(buffer, 1, BUFFER_SIZE, fp);
        write(sockDescriptor, buffer, n);
        sent+=n;
    }
    fclose(fp);
    printf("File sent successfully (%ld bytes)\n", sent);
}

void chatMode(int sockDescriptor) {
    char buffer[BUFFER_SIZE];
    while(1) {
        printf("You: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        write(sockDescriptor, buffer, strlen(buffer));
        int n=read(sockDescriptor, buffer, BUFFER_SIZE);
        if(n<=0) break;
        buffer[n]='\0';
        printf("Server: %s", buffer);
    }
}

int main() {
    int socketDescriptor;
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(socketDescriptor<0) {
        perror("Client socket creation error\n");
        exit(1);
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(SERVER_PORT);
    servaddr.sin_addr.s_addr=inet_addr(SERVER_IP);
    if(inet_addr(SERVER_IP)<=0) {
        perror("Invalid Address!\n");
        exit(1);
    }    
    int temp=connect(socketDescriptor, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(temp<0) {
        perror("Connection error with the server!\n");
        close(socketDescriptor);
        exit(1);
    }
    int choice; 
    printf("1. Chat\n2. Send File\nChoice: ");
    scanf("%d", &choice);
    getchar();
    if(choice==1) {
        write(socketDescriptor, "CHAT", 4);
        chatMode(socketDescriptor);
    } 
    else if(choice==2) {
        write(socketDescriptor, "FILE", 4);
        sendFile(socketDescriptor);
    }
    close(socketDescriptor);
    return 0;
}