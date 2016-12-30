#include "util.h"

namespace util
{

bool fileExists(QString filename)
{
    QFileInfo fileInfo(filename);
    return (fileInfo.exists() && fileInfo.isFile());
}

}
