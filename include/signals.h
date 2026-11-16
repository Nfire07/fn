#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>
#include <stddef.h>

int install_signal_handlers(void);
int get_termination_signal(void);

#endif