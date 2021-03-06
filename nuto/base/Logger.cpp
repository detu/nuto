#include "nuto/base/Logger.h"
#include "nuto/base/Exception.h"

NuTo::Logger::Logger(std::string prefix, bool isQuiet)
    : mLogFile()
    , mLogFileName()
    , mIsQuiet(isQuiet)
    , mPrefix(prefix)
{
}

void NuTo::Logger::OpenFile(std::string filename)
{
    mLogFileName = filename;
    mLogFile.close();
    mLogFile.clear();
    mLogFile.open(filename.c_str());
    if (!mLogFile.is_open())
        throw Exception(__PRETTY_FUNCTION__, "File " + filename + " could not be opened.");
}

void NuTo::Logger::CloseFile()
{
    mLogFile.close();
}

void NuTo::Logger::SetQuiet(bool isQuiet)
{
    mIsQuiet = isQuiet;
}

std::string NuTo::Logger::OptionalPrefix()
{
    if (mPrintPrefix)
    {
        mPrintPrefix = false;
        return mPrefix;
    }
    else
    {
        return "";
    }
}

namespace NuTo
{

NuTo::Logger& operator<<(NuTo::Logger& rLogger, const char& t)
{
    rLogger.Out(t, t == '\n');
    return rLogger;
}

NuTo::Logger& operator<<(NuTo::Logger& rLogger, const std::string& t)
{
    rLogger.Out(t, t[t.size() - 1] == '\n');
    return rLogger;
}

NuTo::Logger& operator<<(NuTo::Logger& rLogger, const char* t)
{
    std::string s(t);
    rLogger.Out(t, s[s.size() - 1] == '\n');
    return rLogger;
}
}

NuTo::Logger NuTo::Log::Debug = NuTo::Logger("Debug| ", true);
NuTo::Logger NuTo::Log::Info = NuTo::Logger("Info|  ", false);
NuTo::Logger NuTo::Log::Error = NuTo::Logger("Error| ", false);
