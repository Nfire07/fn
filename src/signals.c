#include "../include/signals.h"

static volatile sig_atomic_t termination_signal = 0;

static void termination_handler(int signum) {
    termination_signal = signum;
}

int install_signal_handlers(void) {
    struct sigaction sa;

    sa.sa_handler = termination_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if(sigaction(SIGINT, &sa, NULL) == -1)
        return 0;

    if(sigaction(SIGTERM, &sa, NULL) == -1)
        return 0;

    if(sigaction(SIGHUP, &sa, NULL) == -1)
        return 0;

    if(sigaction(SIGQUIT, &sa, NULL) == -1)
        return 0;

    return 1;
}

int get_termination_signal(void) {
    return termination_signal;
}