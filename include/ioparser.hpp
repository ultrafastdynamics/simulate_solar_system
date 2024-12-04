#pragma once

#include <vector>
#include <string>
#include <utility>

namespace IOParser
{
    /**
     * @brief Gets all files in a directory.
     *
     * @param directory The directory to parse.
     *
     * @return A vector of filenames.
     */
    std::vector<std::string> getFilesInDirectory(std::string directory);

    //! Returns the content of a file split into its lines
    std::vector<std::string> fileAsLines(std::string filename);

    /**
     * Splits a given string into a vector of substrings based on a specified delimiter.
     * 
     * This function takes a string and splits it into separate tokens whenever the 
     * delimiter character is encountered. The resulting tokens are stored in a vector 
     * which is returned by the function.
     * 
     * @param str The string to be split.
     * @param delimiter The character used to separate the string into tokens.
     * @return A vector containing the resulting tokens.
     */
    std::vector<std::string> split(std::string str, char delimiter);

}