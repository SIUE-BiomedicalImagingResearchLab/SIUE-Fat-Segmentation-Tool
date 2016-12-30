#ifndef UTIL_H
#define UTIL_H

#include <QFileInfo>

namespace util
{

/* Determines if the filename exists and is a file, returns true, otherwise returns false */
bool fileExists(QString filename);

}

#endif // UTIL_H
