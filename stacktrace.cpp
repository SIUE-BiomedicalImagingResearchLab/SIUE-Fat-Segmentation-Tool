#include "stacktrace.h"

QString globalProgramName;

QString addr2line(QString programName, const void *addr)
{
    QString cmdStr;

    // Truncate the program name down to 256 characters, which is the maximum path size
    programName.truncate(256);

    // WEIRD BUG: If I don't give this it's own variable and instead imbed it into the .arg(XXX) part, it will
    // cause a segmentation fault? No idea why but whatever.
    quintptr address = (quintptr)addr;

    // Get command string to call, this command will map the address to the relevant line in the code
    // Second arg, the field width needs to be double the pointer size, base 16 for hex, and pad with 0s
    // OS X is different and uses atos instead of addr2line like Linux & Windows
#ifdef Q_OS_MACOS
    cmdStr = QString("atos -o %1 %2").arg(programName).arg(address, QT_POINTER_SIZE * 2, 16, QChar('0'));
#else // Q_OS_MACOS
    cmdStr = QString("addr2line -f -p -e %1 %2").arg(programName).arg(address, QT_POINTER_SIZE * 2, 16, QChar('0'));
#endif // Q_OS_MACOS

    return util::execCommand(cmdStr);
}

#ifdef Q_OS_WIN
QString windowsPrintStackTrace(CONTEXT *context, bool walkStack)
{
    // Get the current process
    HANDLE curProcess = GetCurrentProcess();
    // Structure will contain information regarding the stack
    STACKFRAME64 frame = {0};
    // Type of processor that is used, i386, AMD64, or itanium processor
    DWORD image;
    // stackStr contains the string with stack information
    QString stackStr;

    // Structure contains source information for an address such as file name and line number
    IMAGEHLP_LINE64 sourceInfo = {0};
    sourceInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    // Structure has the name of the symbol
    SYMBOL_INFO symbolInfo = {0};
    symbolInfo.SizeOfStruct = sizeof(SYMBOL_INFO);
    symbolInfo.MaxNameLen = 254;

    // Temporary variables not used
    DWORD dwDisplacement = 0;
    DWORD64 dwDisplacement64 = 0;

    // Initialize symbols for the current process
    SymInitialize(curProcess, 0, true);

    // Setup initial stack frame
#ifdef Q_PROCESSOR_X86_32
    image = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = context->Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context->Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context->Esp;
    frame.AddrStack.Mode = AddrModeFlat;
#elif defined(Q_PROCESSOR_X86_64)
    image = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset = context->Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context->Rsp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context->Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
#elif defined(Q_PROCESSOR_IA64)
    image = IMAGE_FILE_MACHINE_IA64;
    frame.AddrPC.Offset = context->StIIP;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context->IntSp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrBStore.Offset = context->RsBSP;
    frame.AddrBStore.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context->IntSp;
    frame.AddrStack.Mode = AddrModeFlat;
#endif

    // If walk the stack, then use StackWalk64 to go through each entry
    while (StackWalk64(image, curProcess, GetCurrentThread(),
                     &frame, context, 0, SymFunctionTableAccess, SymGetModuleBase, 0))
    {
        // WEIRD BUG: If I don't give this it's own variable and instead imbed it into the .arg(XXX) part, it will
        // cause a segmentation fault? No idea why but whatever.
        quintptr address = (quintptr)frame.AddrPC.Offset;

#ifdef Q_CC_MSVC
        // For MSVC, use SymFromAddr to get the symbol name, put <Unknown> if cannot be retrieved
        if (SymFromAddr(curProcess, frame.AddrPC.Offset, &dwDisplacement64, &symbolInfo))
            stackStr += QString("\t%1 at ").arg(symbolInfo.Name);
        else
            stackStr += "\t<Unknown> at ";

        // Next, get the source file and line number it is present at. Put unknowns if not able. Also, make sure
        // to put the actual address offset in case I need to check it out later.
        if (SymGetLineFromAddr64(curProcess, frame.AddrPC.Offset, &dwDisplacement, &sourceInfo))
            stackStr += QString("%1:%2 %3\n").arg(sourceInfo.FileName).arg(sourceInfo.LineNumber).arg(address, QT_POINTER_SIZE * 2, 16, QChar('0'));
        else
            stackStr += QString("<Unknown>:<Unknown> %1\n").arg(address, QT_POINTER_SIZE * 2, 16, QChar('0'));
#else // Q_CC_MSVC
        // For not MSVC, presumably MinGW, we use addr2line command which gets the symbol, file name, and line number in one.
        QString stackLine = addr2line(globalProgramName, (void *)frame.AddrPC.Offset);
        if (stackLine.isNull())
            stackStr += QString("<Unknown> at <Unknown>:<Unknown> %1\n").arg(address, QT_POINTER_SIZE * 2, 16, QChar('0'));
        else
            stackStr += QString("%1 %2\n").arg(stackLine.trimmed()).arg(address, QT_POINTER_SIZE * 2, 16, QChar('0'));
#endif // Q_CC_MSVC

        // If we do not want to walk the stack, then we need to quit after the first run through
        if (!walkStack)
            break;
    }

    // Clean up the symbols for the current process
    SymCleanup(GetCurrentProcess());

    return stackStr;
}

LONG WINAPI windowsExceptionHandler(EXCEPTION_POINTERS *exceptionInfo)
{
    QString exceptionStr;

    switch (exceptionInfo->ExceptionRecord->ExceptionCode)
    {
        case EXCEPTION_ACCESS_VIOLATION:            exceptionStr = "EXCEPTION_ACCESS_VIOLATION"; break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:       exceptionStr = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED"; break;
        case EXCEPTION_BREAKPOINT:                  exceptionStr = "EXCEPTION_BREAKPOINT"; break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:       exceptionStr = "EXCEPTION_DATATYPE_MISALIGNMENT"; break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:        exceptionStr = "EXCEPTION_FLT_DENORMAL_OPERAND"; break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:          exceptionStr = "EXCEPTION_DIVIDE_BY_ZERO"; break;
        case EXCEPTION_FLT_INEXACT_RESULT:          exceptionStr = "EXCEPTION_FLT_INEXACT_RESULT"; break;
        case EXCEPTION_FLT_INVALID_OPERATION:       exceptionStr = "EXCEPTION_FLT_INVALID_OPERATION"; break;
        case EXCEPTION_FLT_OVERFLOW:                exceptionStr = "EXCEPTION_FLT_OVERFLOW"; break;
        case EXCEPTION_FLT_STACK_CHECK:             exceptionStr = "EXCEPTION_FLT_STACK_CHECK"; break;
        case EXCEPTION_FLT_UNDERFLOW:               exceptionStr = "EXCEPTION_FLT_UNDERFLOW"; break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:         exceptionStr = "EXCEPTION_ILLEGAL_INSTRUCTION"; break;
        case EXCEPTION_IN_PAGE_ERROR:               exceptionStr = "EXCEPTION_IN_PAGE_ERROR"; break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:          exceptionStr = "EXCEPTION_INT_DIVIDE_BY_ZERO"; break;
        case EXCEPTION_INT_OVERFLOW:                exceptionStr = "EXCEPTION_INT_OVERFLOW"; break;
        case EXCEPTION_INVALID_DISPOSITION:         exceptionStr = "EXCEPTION_INVALID_DISPOSITION"; break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:    exceptionStr = "EXCEPTION_NONCONTINUABLE_EXCEPTION"; break;
        case EXCEPTION_PRIV_INSTRUCTION:            exceptionStr = "EXCEPTION_PRIV_INSTRUCTION"; break;
        case EXCEPTION_SINGLE_STEP:                 exceptionStr = "EXCEPTION_SINGLE_STEP"; break;
        case EXCEPTION_STACK_OVERFLOW:              exceptionStr = "EXCEPTION_STACK_OVERFLOW"; break;
        default:                                    exceptionStr = "Unrecognized Exception"; break;
    }

    // If this is a stack overflow, then we cannot walk the stack so show where error happened
    // Otherwise, walk the stack
    bool walkStack = (exceptionInfo->ExceptionRecord->ExceptionCode != EXCEPTION_STACK_OVERFLOW);
    QString stackStr = windowsPrintStackTrace(exceptionInfo->ContextRecord, walkStack);

    qCritical().nospace().noquote() << "Exception: " << exceptionStr << "\nStack: \n" << stackStr;

    return EXCEPTION_EXECUTE_HANDLER;
}

void setSignalHandler()
{
    SetUnhandledExceptionFilter(windowsExceptionHandler);
}

#else // Q_OS_WIN

// Maximum number of frames that we should retrieve from the stack at once
#define MAX_STACK_FRAMES 64

static void *stackTraces[MAX_STACK_FRAMES];

QString posixPrintStackTrace()
{
    // Obtain the number of traces in the stack
    int traceSize = backtrace(stackTraces, MAX_STACK_FRAMES);
    char **messages = backtrace_symbols(stackTraces, traceSize);
    // stackStr contains the string with stack information
    QString stackStr;

    // WEIRD BUG: If I don't give this it's own variable and instead imbed it into the .arg(XXX) part, it will
    // cause a segmentation fault? No idea why but whatever.
    quintptr address = (quintptr)stackTraces[i];

    // Skip first couple stack frames because they are this function and our handler, also skip the last frame as it's (always?) junk
    for (int i = 0; i < trace_size; ++i) // we'll use this for now so you can see what's going on
    {
        // Use addr2line command to get the symbol, file name, and line number in one
        QString stackLine = addr2line(globalProgramName, (void *)stackTraces[i]);
        if (stackLine.isNull())
            stackStr += QString("<Unknown> at <Unknown>:<Unknown> %1\n").arg(address, QT_POINTER_SIZE * 2, 16, QChar('0'));
        else
            stackStr += QString("%1 %2\n").arg(stackLine.trimmed()).arg(address, QT_POINTER_SIZE * 2, 16, QChar('0'));
    }

    if (messages)
        free(messages);
}

void posixSignalHandler(int sig, siginfo_t *siginfo, void *context)
{
    QString exceptionStr;

    switch (sig)
    {
        case SIGSEGV: exceptionStr = "SIGSEGV (Segmentation Fault)"; break;
        case SIGINT: exceptionStr = "SIGINT (Interactive Attention)"; break;
        case SIGFPE:
            switch (siginfo->si_code)
            {
                case FPE_INTDIV: exceptionStr = "SIGFPE (Integer Divide by Zero)"; break;
                case FPE_INTOVF: exceptionStr = "SIGFPE (Integer Overflow)"; break;
                case FPE_FLTDIV: exceptionStr = "SIGFPE (FP Divide by Zero"; break;
                case FPE_FLTOVF: exceptionStr = "SIGFPE (FP Overflow)"; break;
                case FPE_FLTUND: exceptionStr = "SIGFPE (FP Underflow)"; break;
                case FPE_FLTRES: exceptionStr = "SIGFPE (FP Inexact Result)"; break;
                case FPE_FLTINV: exceptionStr = "SIGFPE (FP Invalid Operation)"; break;
                case FPE_FLTSUB: exceptionStr = "SIGFPE (Subscript Out of Range)"; break;
                default: exceptionStr = "SIGFPE (Arithmetic Exception)"; break;
            }
            break;

        case SIGILL:
            switch (siginfo->si_code)
            {
                case ILL_ILLOPC: exceptionStr = "SIGILL (Illegal Opcode)"; break;
                case ILL_ILLOPN: exceptionStr = "SIGILL (Illegal Operand)"; break;
                case ILL_ILLADR: exceptionStr = "SIGILL (Illegal Addressing Mode)"; break;
                case ILL_ILLTRP: exceptionStr = "SIGILL (Illegal Trap)"; break;
                case ILL_PRVOPC: exceptionStr = "SIGILL (Privileged Opcode)"; break;
                case ILL_PRVREG: exceptionStr = "SIGILL (Privileged Register)"; break;
                case ILL_COPROC: exceptionStr = "SIGILL (Coprocessor Error)"; break;
                case ILL_BADSTK: exceptionStr = "SIGILL (Internal Stack Error)"; break;
                default: exceptionStr = "SIGILL (Illegal Instruction)"; break;
            }
            break;

        case SIGTERM: exceptionStr = "SIGTERM (Termination Request)"; break;
        case SIGABRT: exceptionStr = "SIGABRT (Abort)"; break;
        default: exceptionStr = "Unrecognized Exception"; break;
    }

    qCritical() << "Exception: " << qUtf8Printable(exceptionStr);
    qCritical() << "Stack: ";

    posixPrintStackTrace();
    _Exit(1);
}

static uint8_t alternateStack[SIGSTKSZ];

void setSignalHandler()
{
    // Setup alternate stack
    stack_t ss = {};
    ss.ss_sp = (void *)alternateStack;
    ss.ss_size = SIGSTKSZ;
    ss.ss_flags = 0;

    if (!sigaltstack(&ss, NULL))
        qCritical() << "Error calling sigaltstack";

    // Register our signal handlers
    struct sigaction sigAction = {};
    sigAction.sa_sigaction = posixSignalHandler;
    sigemptyset(&sigAction.sa_mask);

#ifdef Q_OS_MACOS
    // For some reason backtrace() doesn't work on osx when we use an alternate stack
    sigAction.sa_flags = SA_SIGINFO;
#else // Q_OS_MACOS
    sigAction.sa_flags = SA_SIGINFO | SA_ONSTACK;
#endif // Q_OS_MACOS

    if (!sigaction(SIGSEGV, &sigAction, NULL))
        qCritical() << "Error calling sigaction for SIGSEGV";

    if (!sigaction(SIGFPE,  &sigAction, NULL))
        qCritical() << "Error calling sigaction for SIGFPE";

    if (!sigaction(SIGINT,  &sigAction, NULL))
        qCritical() << "Error calling sigaction for SIGINT";

    if (!sigaction(SIGILL,  &sigAction, NULL))
        qCritical() << "Error calling sigaction for SIGILL";

    if (!sigaction(SIGTERM, &sigAction, NULL))
        qCritical() << "Error calling sigaction for SIGTERM";

    if (!sigaction(SIGABRT, &sigAction, NULL))
        qCritical() << "Error calling sigaction for SIGABRT";
}
#endif // Q_OS_WIN

QString printStackTrace()
{
#ifdef Q_OS_WIN
    CONTEXT context;
    RtlCaptureContext(&context);

    return windowsPrintStackTrace(&context, true);
#else // Q_OS_WIN
    return posixPrintStackTrace();
#endif
}
