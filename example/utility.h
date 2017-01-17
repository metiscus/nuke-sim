#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <map>
#include <string>
#include <sstream>
#include <vector>

namespace utility
{

    /*! \brief A list of strings */
    typedef std::vector<std::string> StringList;

    /*! \brief A string to string dictionary */
    typedef std::map<std::string, std::string> StringDict;

    /*! \brief Removes leading and trailing whitespace from a string
        \param[in] string 
        \return string without leading or trailing spaces
    */
    inline std::string chomp_string(const std::string& string)
    {
        if(string.empty())
        {
            return "";
        }

        if(string == " ")
        {
            return "";
        }
        
        std::string::size_type beg = string.find_first_not_of(" ");
        if(beg == std::string::npos)
        {
            return "";
        }

        std::string::size_type end = string.find_last_not_of(" ");  
        

        return string.substr(beg, end-beg+1);
    }


    /*!  \brief Reads the contents of a file into a string.
        \param[in] filename The name of the file to open and read.
        \return The contents of the file as a string.
    */
    inline std::string file_to_string( const std::string& filename )
    {
        std::ifstream inFile (filename.c_str());
        std::string ret;
        if(inFile.is_open())
        {
            while(!inFile.eof())
            {
                std::string line;
                std::getline(inFile, line);
                ret = ret + line + "\n";
            }
        }

        return ret;
    }

    /*! \brief Splits a string using the separator character.
        \param[in] longstring The main string to split.
        \param[in] sep The chacacter to split.
        \return The list longstring split using , with leading and trailing spaces removed
    */
    inline StringList split_string (const std::string& lstring, const char sep = ',')
    {
        StringList ret;
        std::istringstream is(lstring);
        std::string token;
        while(std::getline(is, token, sep))
        {
            ret.push_back(chomp_string(token));
        }

        return ret;
    }
}

#endif // UTILITY_H_INCLUDED
