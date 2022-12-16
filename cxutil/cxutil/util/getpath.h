#ifndef CXUTIL_GETPATH_1606212869631_H
#define CXUTIL_GETPATH_1606212869631_H
#include <string>
#include <unordered_set>

namespace cxutil
{
    std::string getPathName(const std::string& filePath);

    /*
 * \brief Get the default search directories to search for definition files.
 * \return The default search directories to search for definition files.
 */
    std::unordered_set<std::string> defaultSearchDirectories();
}

#endif // CXUTIL_GETPATH_1606212869631_H