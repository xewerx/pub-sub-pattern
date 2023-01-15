#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define SERWER_PORT 1234
#define MAX_CONNECTION 10

#define MAX_PAYLOAD_LENGTH 100
#define MAX_TOPIC_NAME_LENGTH 20
#define MAX_MESSAGE_LENGTH 100

#define MAX_SUBSCRIBERS_FOR_TOPIC 5
#define MAX_TOPICS_COUNT 25

struct cln {
    int cfd;
    struct sockaddr_in caddr;
    // tutaj mozna dodac kolejne pole ktore potrzebujemy w kliencie
};

struct topic {
    char name[MAX_TOPIC_NAME_LENGTH];
    int subscribersCount;
    struct cln* subscribers[MAX_SUBSCRIBERS_FOR_TOPIC];
};

struct topic TOPICS[MAX_TOPICS_COUNT];
int topics_count = 5;
char* init_topic_names[5] = { "Samochody", "Sport", "Zdrowie", "Polityka", "Finanse" };

void sendTopics(struct cln* client) {
    for(int i = 0; i < topics_count; i++) {
        int bytesSentTopics = 0;
        while(bytesSentTopics < sizeof(TOPICS[i].name))
            bytesSentTopics += write(client->cfd, TOPICS[i].name, sizeof(TOPICS[i].name));

        if(i < topics_count - 1) { // don't send separator for last topic
            int bytesSentSeparator = 0;
            while(bytesSentSeparator < 1)
                bytesSentSeparator += write(client->cfd, ";", 1); 
        }
    }

    write(client->cfd, "#", 1); // znak końca danych
}

void subscribeTopic(char* topicName, struct cln* subscriber) {
    printf("New subscription request for topic: %s \n", topicName);
    for(int i = 0; i < topics_count; i++) {
        if(strcmp(topicName, TOPICS[i].name) == 0) {
            TOPICS[i].subscribers[TOPICS[i].subscribersCount] = subscriber;
            TOPICS[i].subscribersCount++;
            return;
        }
    }
    printf("Requested topic doesn't exist!\n");
    return;
}

void publishMessage(char* topicName, char* message) {
    printf("Message: >>>%s<<< has been published for topic: %s \n", message, topicName);
    int topicsLength = sizeof(TOPICS) / sizeof(TOPICS[0]);
    for(int i = 0; i < topicsLength; i++) {
        if(strcmp(topicName, TOPICS[i].name) == 0) {
            for(int j = 0; j < TOPICS[i].subscribersCount; j++) {
                int bytesSentMessage = 0;
                while(bytesSentMessage < sizeof(message))
                    bytesSentMessage += write(TOPICS[i].subscribers[j]->cfd, message, strlen(message));
                write(TOPICS[i].subscribers[j]->cfd, "#", 1); // znak końca danych
            }
        }
    }
}

void addTopic(char* topicName) {
    strcpy(TOPICS[topics_count].name, topicName);
    TOPICS[topics_count].subscribersCount = 0;
    topics_count++;
    printf("New topic: >>>%s<<< has been added\n", topicName);
}

void removeTopic(char* topicName) {
    int indexOfElementToRemove = -1;
    for(int i = 0; i < topics_count; i++) {
        if(strcmp(topicName, TOPICS[i].name) == 0) {
            indexOfElementToRemove = i;
            for(int j = 0; j < TOPICS[i].subscribersCount; j++)
                close(TOPICS[i].subscribers[j]->cfd);
            break;
        }
    }

    if(indexOfElementToRemove == -1) {
        printf("Requested topic doesn't exist!\n");
        return;
    }

    for(int i = indexOfElementToRemove; i < MAX_TOPICS_COUNT; i++) {
        if(i+1 == MAX_TOPICS_COUNT) break;
        TOPICS[i] = TOPICS[i+1];
    }
    topics_count--;
    printf("Topic: >>>%s<<< has been removed\n", topicName);
}

void* clientHandlerThread(void* arg) {
    char buffer[MAX_PAYLOAD_LENGTH] = {};
    char topicName[MAX_TOPIC_NAME_LENGTH] = {};
    char message[MAX_MESSAGE_LENGTH] = {};

    struct cln* client = (struct cln*)arg;
    printf("New connection: %s\n", inet_ntoa((struct in_addr)client->caddr.sin_addr));

    int count = read(client->cfd, buffer, MAX_PAYLOAD_LENGTH);
    char command = buffer[0];

    switch(command) {
        case '1':
            sendTopics(client);
            close(client->cfd);
            free(client);
            break;
        case '2':
            strcat(topicName, &buffer[2]); // od 3 znaku do konca bufora
            subscribeTopic(topicName, client);
            break;
        case '3':
            printf("buffer>>");
            for(int i = 2; i < count; i++) {
                printf("%c", buffer[i]);
                if(buffer[i] == ';') {
                    strncat(topicName, &buffer[2], i - 2);
                    strcat(message, &buffer[i + 1]);
                }
            }
            printf("<<buffer\n");

            publishMessage(topicName, message);
            close(client->cfd);
            free(client);
            break;
        case '4':
            strcat(topicName, &buffer[2]); // od 3 znaku do konca bufora
            addTopic(topicName);
            close(client->cfd);
            free(client);
            break;
        case '5':
            strcat(topicName, &buffer[2]); // od 3 znaku do konca bufora
            removeTopic(topicName);
            close(client->cfd);
            free(client);
            break;
        default:
            printf("Unknown command\n");
    }

    // czyscimy bufory zeby nastepni klienci nie mieli pozostalosci danych
    memset(buffer, 0, sizeof buffer);
    memset(topicName, 0, sizeof topicName);
    memset(message, 0, sizeof message);

    return EXIT_SUCCESS;
}

int main() {
    for (int i = 0; i < topics_count; i++) {
        strcpy(TOPICS[i].name, init_topic_names[i]);
        // TOPICS[i].name = init_topic_names[i];
        TOPICS[i].subscribersCount = 0;
    }

    socklen_t slt;
    int sfd, cfd, on = 1;
    struct sockaddr_in saddr, caddr;
    pthread_t tid;

    sfd = socket(PF_INET, SOCK_STREAM, 0);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(SERWER_PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;

    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
    bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));
    listen(sfd, MAX_CONNECTION);

    printf("Server started on port: %d\n", SERWER_PORT);

    while(1) {
        struct cln* client = malloc(sizeof(struct cln));
        slt = sizeof(caddr);
        client->cfd = accept(sfd, (struct sockaddr*)&client->caddr, &slt);

        pthread_create(&tid, NULL, clientHandlerThread, client);
        pthread_detach(tid); // zeby nie bylo zombie
    }
    close(sfd);

    return EXIT_SUCCESS;
}