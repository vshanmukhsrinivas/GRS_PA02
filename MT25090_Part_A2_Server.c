

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>

#define BACKLOG 128
#define NUM_FIELDS 8

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    stop = 1;
}

/* Structured message */
typedef struct {
    char *fields[NUM_FIELDS];
    size_t field_size;
} message_t;

/* Thread argument */
typedef struct {
    int client_fd;
    size_t message_size;
} thread_arg_t;

/* Create structured message */
message_t *create_message(size_t message_size) {

    message_t *msg = malloc(sizeof(message_t));
    msg->field_size = message_size / NUM_FIELDS;

    for (int i = 0; i < NUM_FIELDS; i++) {
        msg->fields[i] = malloc(msg->field_size);
        memset(msg->fields[i], 'A' + i, msg->field_size);
    }

    return msg;
}

/* Free structured message */
void free_message(message_t *msg) {
    for (int i = 0; i < NUM_FIELDS; i++)
        free(msg->fields[i]);
    free(msg);
}

/* Client handler thread */
void *client_handler(void *arg) {

    thread_arg_t *targ = (thread_arg_t *)arg;
    int fd = targ->client_fd;
    size_t message_size = targ->message_size;

    char *request_buffer = malloc(message_size);

    message_t *response = create_message(message_size);

    while (!stop) {

        /* Step 1: Receive request */
        ssize_t r = recv(fd, request_buffer, message_size, 0);
        if (r <= 0)
            break;

        /* Step 2: Prepare iovec */
        struct iovec iov[NUM_FIELDS];

        for (int i = 0; i < NUM_FIELDS; i++) {
            iov[i].iov_base = response->fields[i];
            iov[i].iov_len  = response->field_size;
        }

        /* Step 3: Prepare msghdr */
        struct msghdr msg_hdr;
        memset(&msg_hdr, 0, sizeof(msg_hdr));

        msg_hdr.msg_iov = iov;
        msg_hdr.msg_iovlen = NUM_FIELDS;

        /* Step 4: Send using single system call */
        ssize_t s = sendmsg(fd, &msg_hdr, 0);

        if (s <= 0)
            break;
    }

    free(request_buffer);
    free_message(response);
    close(fd);
    free(targ);

    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s <port> <message_size>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    size_t message_size = atol(argv[2]);

    signal(SIGINT, handle_sigint);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd,
         (struct sockaddr *)&server_addr,
         sizeof(server_addr));

    listen(server_fd, BACKLOG);

    printf("A2 Server (One-Copy) running on port %d\n", port);

    while (!stop) {

        int client_fd = accept(server_fd, NULL, NULL);

        thread_arg_t *targ = malloc(sizeof(thread_arg_t));
        targ->client_fd = client_fd;
        targ->message_size = message_size;

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, targ);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
