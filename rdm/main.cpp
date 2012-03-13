#include <QCoreApplication>
#include <QThread>
#include <QThreadPool>
#include "Rdm.h"
#include <getopt.h>
#include <QDateTime>
#include <stdio.h>
#include <stdlib.h>
#include <Log.h>

void usage(FILE *f)
{
    fprintf(f,
            "rdm ...options...\n"
            "  --help|-h               Display this page\n"
            "  --include-path|-I [arg] Add additional include path to clang\n"
            "  --include|-i [arg]      Add additional include directive to clang\n"
            "  --log-file|-l [arg]     Log to this file\n"
            "  --verbose|-v            Change verbosity, multiple -v's are allowed\n"
            "  --thread-count|-j [arg] Spawn this many threads for thread pool\n");
}

int main(int argc, char** argv)
{
    struct option opts[] = {
        { "help", no_argument, 0, 'h' },
        { "include-path", required_argument, 0, 'I' },
        { "include", required_argument, 0, 'i' },
        { "log-file", required_argument, 0, 'l' },
        { "verbose", no_argument, 0, 'v' },
        { "thread-count", required_argument, 0, 'j' },
        { 0, 0, 0, 0 }
    };

    int jobs = QThread::idealThreadCount();
    unsigned options = 0;
    QList<QByteArray> defaultArguments;
    const char *logFile = 0;
    int logLevel = 0;

    forever {
        const int c = getopt_long(argc, argv, "hI:i:l:j:nv", opts, 0);
        if (c == -1)
            break;
        switch (c) {
        case 'h':
            usage(stdout);
            return 0;
        case 'j':
            jobs = atoi(optarg);
            if (jobs <= 0) {
                fprintf(stderr, "Can't parse argument to -j %s", optarg);
                return 1;
            }
            break;
        case 'I':
            defaultArguments.append("-I" + QByteArray(optarg));
            break;
        case 'i':
            defaultArguments.append("-include");
            defaultArguments.append(optarg);
            break;
        case 'l':
            logFile = optarg;
            break;
        case 'v':
            ++logLevel;
            break;
        case '?':
            usage(stderr);
            return 1;
        }
    }
    QThreadPool::globalInstance()->setMaxThreadCount(jobs);
    QCoreApplication app(argc, argv);
    if (!initLogging(logLevel, logFile)) {
        fprintf(stderr, "Can't initialize logging with %d %s\n", logLevel, logFile ? logFile : "");
        return false;
    }

    log("Running with %d jobs\n", jobs);

    Rdm rdm;
    if (!rdm.init(options, defaultArguments))
        return 1;

    const int ret = app.exec();
    return ret;
}
