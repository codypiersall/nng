#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>

#define CHECK(x) (assert((x) == 0))
#define ADDR "tcp://127.0.0.1:14224"

#define NUM_ITER 1000

struct seen_events {
    int pipe_id;
    int event;
};

struct cb_data {
    int iter;
    pthread_mutex_t mut;
    struct seen_events *events;
};


void cb(nng_pipe pipe, nng_pipe_ev event, void *data) {
    // add delays to simulate doing real work.
    // These are the delays that demonstrated the race on my computer.
    if (event == NNG_PIPE_EV_ADD_PRE) {
        usleep(40);
    }
    else if (event == NNG_PIPE_EV_ADD_POST) {
        usleep(50);
    }
    else if (event == NNG_PIPE_EV_REM_POST) {
        usleep(20);
    }
    struct cb_data *d = (struct cb_data *) data;
    pthread_mutex_lock(&d->mut);
    d->iter += 1;
    d->events[d->iter].pipe_id = nng_pipe_id(pipe);
    d->events[d->iter].event = event;
    pthread_mutex_unlock(&d->mut);
}


void init_cb_data(struct cb_data *data, int num_iter) {
    data->iter = 0;
    pthread_mutex_init(&data->mut, NULL);
    data->events = malloc(sizeof (*data->events) * num_iter * 3);
}

int main(int argc, char *argv[]) {
    nng_socket listener, dialer;
    int num_iter = NUM_ITER;
    char *addr = ADDR;
    if (argc >= 3) {
        num_iter = strtol(argv[2], NULL, 0);
    }
    if (argc >= 2) {
        addr = argv[1];
    }
    struct cb_data listener_cb_data, dialer_cb_data;
    init_cb_data(&listener_cb_data, num_iter);
    init_cb_data(&dialer_cb_data, num_iter);

    CHECK(nng_pair0_open(&listener));
    CHECK(nng_listen(listener, addr, NULL, 0));

    for (int j = 0; j < num_iter; j++) {
        CHECK(nng_pair_open(&dialer));
        CHECK(nng_pipe_notify(dialer, NNG_PIPE_EV_ADD_PRE, cb, &dialer_cb_data));
        CHECK(nng_pipe_notify(dialer, NNG_PIPE_EV_ADD_POST, cb, &dialer_cb_data));
        CHECK(nng_pipe_notify(dialer, NNG_PIPE_EV_REM_POST, cb, &dialer_cb_data));
        CHECK(nng_dial(dialer, addr, NULL, NNG_FLAG_NONBLOCK));
        usleep(100);
        CHECK(nng_close(dialer));
    }

    for (int j = 0; j < num_iter * 3; j++) {
        struct seen_events *e = &dialer_cb_data.events[j];
        fprintf(stdout, "%d %d\n", e->pipe_id, e->event);
    }
    usleep(100000);
}
