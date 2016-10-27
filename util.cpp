#include "util.hpp"

namespace Util
{

bool FileExists(QString filename)
{
    QFileInfo fileInfo(filename);
    return (fileInfo.exists() && fileInfo.isFile());
}

}
