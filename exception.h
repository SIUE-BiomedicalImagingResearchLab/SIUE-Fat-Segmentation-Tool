#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>

class Exception : std::exception
{
    private:
        const QString title_;
        const QString message_;

    public:
        Exception(const char *title__, const char *message__) : title_(title__), message_(message__) {}
        Exception(const QString title__, const QString message__) : title_(title__), message_(message__) {}

        const QString title() const
        {
            return this->title_;
        }

        const QString message() const
        {
            return this->message_;
        }

        ~Exception() throw() {}
};

#define EXCEPTION(title, message) throw Exception(title, message)
#define EXCEPTIONF(title, format, ...) { char *buf__ = new char[1024]; snprintf(buf__, 1024, format, __VA_ARGS__); buf__[1023] = '\0'; throw Exception(title, buf__); }

#endif // EXCEPTION_H
