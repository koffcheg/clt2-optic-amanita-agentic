#include "ut_signal.h"
#include <cstring>
#include <unistd.h>
#include <csignal>
#include <log4cxx/logger.h>

static log4cxx::LoggerPtr logger;
static volatile sig_atomic_t signal_self;

struct signal {
    int signo;
    const char *signame;
    unsigned int flags;

    void (*handler)(int signo);

    int *indicator;
};

static void signal_handler(int signo);

static struct signal signals[] = {
		{SIGINT,  "SIGINT",  0, 			signal_handler, nullptr,},
		{SIGTERM, "SIGTERM", 0, 			signal_handler, nullptr,},
		{SIGQUIT, "SIGQUIT", 0, 			signal_handler, nullptr,},
        {SIGSEGV, "SIGSEGV", SA_RESETHAND, 	signal_handler, nullptr,},
        {SIGABRT, "SIGABRT", SA_RESETHAND, 	signal_handler, nullptr,},
        {SIGPIPE, "SIGPIPE", 0, SIG_IGN,                   	nullptr,},
        {SIGCHLD, "SIGCHLD", 0, SIG_IGN,                   	nullptr,},
        {0, nullptr,         0, nullptr,                   	nullptr,},
};

static void(*stop_handler_)() = nullptr;

int init_signal(const char *logger_name, void(*stop_handler)()) {
	logger = log4cxx::Logger::getLogger(logger_name);
	stop_handler_ = stop_handler;

    struct signal *sig;
    for (sig = signals; sig->signo != 0; ++sig) {
        struct sigaction sa{};
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sig->handler;
        sa.sa_flags = sig->flags;
        sigemptyset(&sa.sa_mask);
        int ret = sigaction(sig->signo, &sa, nullptr);
        if (ret < 0) {
            return -1;
        }
    }

    return 0;
}

static void signal_handler(int signo) {
    struct signal *sig;
    for (sig = signals; sig->signo != 0; ++sig) {
        if (sig->signo == signo)
            break;
    }

    // protection from receiving a "broadcast" signal from self
    if (getpid() == getpgrp() && signal_self) {
        signal_self = 0;
        return;
    }

    switch (signo) {
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			stop_handler_();
			return;
		case SIGSEGV:
			LOG4CXX_FATAL(logger, getpid() << "signal: " << signo << " - " << sig->signame << " received, core dumping");
            raise(SIGSEGV);
            break;
        case SIGABRT:
			LOG4CXX_FATAL(logger, getpid() << "signal: " << signo << " - " << sig->signame << " received, core dumping");
            raise(SIGABRT);
            break;
        default:
            break;
    }

    if (getpid() == getpgrp()) {
        signal_self = 1;
        kill(0, signo);
		LOG4CXX_FATAL(logger, getpid() << "signal: " << signo << " - " << sig->signame << " sent to process group");
        return;
    }
}

