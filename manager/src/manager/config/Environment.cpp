//
//  Environment.cpp
//  manager
//
//  Created by apple on 25.05.2024.
//

#include "Environment.h"
#include "Constants.h"

#include <sys/file.h>
#include <unistd.h>
#include <errno.h>

namespace cm
{

Environment::Environment()
    : _runMode(RunMode::Failed)
    , _fileHandle(0)
{
    _fileHandle = open(manager_pid_file_path, O_CREAT | O_RDWR, 0666);
    int rc = flock(_fileHandle, LOCK_EX | LOCK_NB);
    if (rc)
    {
        if(EWOULDBLOCK == errno)
        {
            _runMode = RunMode::Secondary;
        }
        else
        {
            _runMode = RunMode::Failed;
        }
        close(_fileHandle);
        _fileHandle = 0;
    }
    else
    {
        _runMode = RunMode::Primary;
    }
}

Environment::~Environment()
{
    if (_fileHandle)
    {
        close(_fileHandle);
    }
}

}
