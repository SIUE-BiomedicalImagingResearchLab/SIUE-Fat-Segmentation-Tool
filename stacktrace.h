#ifndef STACKTRACE_H
#define STACKTRACE_H

#include <QString>
#include <QDebug>

#include "util.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <imagehlp.h>
#else // Q_OS_WIN
#include <err.h>
#include <execinfo.h>
#include <signal.h>
#include <cerrno>
#endif // Q_OS_WIN

extern QString globalProgramName;

QString addr2line(QString programName, const void *addr);

void setSignalHandler();
QString printStackTrace();

#endif // STACKTRACE_H
