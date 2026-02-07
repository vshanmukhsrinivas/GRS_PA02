

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>

#define NUM_FIELDS 8

typedef struct {
    char *server_ip;
    int port;
    size_t message_size;
    int duration;
    long long *total_bytes;
    long long *total_requests;
    pthread_mutex_t *lock;
} thread_arg_t;

/* Worker thread */
void *client_worker(void *arg) {

    thread_arg_t *targ = (thread_arg_t *)arg;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(targ->port);

    if (inet_pton(AF_INET, targ->server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return NULL;
    }

    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(sock);
        return NULL;
    }

    char *request_buffer = malloc(targ->message_size);
    memset(request_buffer, 'R', targ->message_size);

    char *response_buffer = malloc(targ->message_size);

    struct timeval start, now;
    gettimeofday(&start, NULL);

    while (1) {

        gettimeofday(&now, NULL);

        double elapsed =
            (now.tv_sec - start.tv_sec) +
            (now.tv_usec - start.tv_usec) / 1000000.0;

        if (elapsed >= targ->duration)
            break;

        /* Send request */
        ssize_t s = send(sock,
                         request_buffer,
                         targ->message_size,
                         0);

        if (s <= 0) {
            perror("Send failed");
            break;
        }

        /* Receive full response */
        ssize_t total_received = 0;

        while (total_received < targ->message_size) {

            ssize_t r = recv(sock,
                             response_buffer + total_received,
                             targ->message_size - total_received,
                             0);

            if (r <= 0) {
                perror("Recv failed");
                break;
            }

            total_received += r;
        }

        /* Update global counters */
        pthread_mutex_lock(targ->lock);
        *(targ->total_bytes) += (targ->message_size * 2);
        *(targ->total_requests) += 1;
        pthread_mutex_unlock(targ->lock);
    }

    free(request_buffer);
    free(response_buffer);
    close(sock);

    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 6) {
        printf("Usage: %s <ip> <port> <msg_size> <threads> <duration>\n",
               argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    size_t message_size = atol(argv[3]);
    int threads = atoi(argv[4]);
    int duration = atoi(argv[5]);

    pthread_t tid[threads];

    long long total_bytes = 0;
    long long total_requests = 0;
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);

    struct timeval global_start, global_end;
    gettimeofday(&global_start, NULL);

    for (int i = 0; i < threads; i++) {

        thread_arg_t *targ = malloc(sizeof(thread_arg_t));
        targ->server_ip = server_ip;
        targ->port = port;
        targ->message_size = message_size;
        targ->duration = duration;
        targ->total_bytes = &total_bytes;
        targ->total_requests = &total_requests;
        targ->lock = &lock;

        pthread_create(&tid[i], NULL, client_worker, targ);
    }

    for (int i = 0; i < threads; i++)
        pthread_join(tid[i], NULL);

    gettimeofday(&global_end, NULL);

    double total_time =
        (global_end.tv_sec - global_start.tv_sec) +
        (global_end.tv_usec - global_start.tv_usec) / 1000000.0;

    printf("TOTAL_BYTES=%lld\n", total_bytes);
    printf("TOTAL_REQUESTS=%lld\n", total_requests);
    printf("TOTAL_TIME=%f\n", total_time);

    pthread_mutex_destroy(&lock);

    return 0;
}
