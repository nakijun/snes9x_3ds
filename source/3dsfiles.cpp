#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <tuple>
#include <vector>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <3ds.h>
#include <dirent.h>

#include "port.h"
#include "3dsfiles.h"

inline std::string operator "" s(const char* s, unsigned int length) {
    return std::string(s, length);
}

static char currentDir[_MAX_PATH] = "";

//----------------------------------------------------------------------
// Initialize the library
//----------------------------------------------------------------------
void file3dsInitialize(void)
{
    getcwd(currentDir, _MAX_PATH);
}


//----------------------------------------------------------------------
// Gets the current directory.
//----------------------------------------------------------------------
char *file3dsGetCurrentDir(void)
{
    return currentDir;
}


//----------------------------------------------------------------------
// Go up or down a level.
//----------------------------------------------------------------------
void file3dsGoUpOrDownDirectory(const DirectoryEntry& entry) {
    if (entry.Type == FileEntryType::ParentDirectory) {
        file3dsGoToParentDirectory();
    } else if (entry.Type == FileEntryType::ChildDirectory) {
        file3dsGoToChildDirectory(entry.Filename.c_str());
    }
}

//----------------------------------------------------------------------
// Go up to the parent directory.
//----------------------------------------------------------------------
void file3dsGoToParentDirectory(void)
{
    int len = strlen(currentDir);

    if (len > 1)
    {
        for (int i = len - 2; i>=0; i--)
        {
            if (currentDir[i] == '/')
            {
                currentDir[i + 1] = 0;
                break;
            }
        }
    }
}


//----------------------------------------------------------------------
// Go up to the child directory.
//----------------------------------------------------------------------
void file3dsGoToChildDirectory(const char* childDir)
{
    strncat(currentDir, childDir, _MAX_PATH);
    strncat(currentDir, "/", _MAX_PATH);
}


//----------------------------------------------------------------------
// Gets the extension of a given file.
//----------------------------------------------------------------------
char *file3dsGetExtension(char *filePath)
{
    int len = strlen(filePath);
    char *extension = &filePath[len];

    for (int i = len - 1; i >= 0; i--)
    {
        if (filePath[i] == '.')
        {
            extension = &filePath[i + 1];
            break;
        }
    }
    return extension;
}


//----------------------------------------------------------------------
// Case-insensitive check for substring.
//----------------------------------------------------------------------
char* stristr( char* str1, const char* str2 )
{
    char* p1 = str1 ;
    const char* p2 = str2 ;
    char* r = *p2 == 0 ? str1 : 0 ;

    while( *p1 != 0 && *p2 != 0 )
    {
        if( tolower( *p1 ) == tolower( *p2 ) )
        {
            if( r == 0 )
            {
                r = p1 ;
            }

            p2++ ;
        }
        else
        {
            p2 = str2 ;
            if( tolower( *p1 ) == tolower( *p2 ) )
            {
                r = p1 ;
                p2++ ;
            }
            else
            {
                r = 0 ;
            }
        }

        p1++ ;
    }

    return *p2 == 0 ? r : 0 ;
}

//----------------------------------------------------------------------
// Load all ROM file names (up to 512 ROMs)
//
// Specify a comma separated list of extensions.
//
//----------------------------------------------------------------------
void file3dsGetFiles(std::vector<DirectoryEntry>& files, char *extensions, int maxFiles)
{
    files.clear();

    struct dirent* dir;
    DIR* d = opendir(currentDir);

    if (strlen(currentDir) > 1)
    {
        // Insert the parent directory.
        files.emplace_back(".. (Up to Parent Directory)"s, FileEntryType::ParentDirectory);
    }

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (dir->d_name[0] == '.')
                continue;
            if (dir->d_type == DT_DIR)
            {
                files.emplace_back(std::string(dir->d_name), FileEntryType::ChildDirectory);
            }
            if (dir->d_type == DT_REG)
            {
                char *ext = file3dsGetExtension(dir->d_name);

                if (ext[0] == '\0' || !stristr(extensions, ext))
                    continue;

                files.emplace_back(std::string(dir->d_name), FileEntryType::File);
            }
        }
        closedir(d);
    }

    std::sort( files.begin(), files.end(), [](const DirectoryEntry& a, const DirectoryEntry& b) {
        return std::tie(a.Type, a.Filename) < std::tie(b.Type, b.Filename);
    } );
}