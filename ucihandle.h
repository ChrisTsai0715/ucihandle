#ifndef CUCIHANDLE_H
#define CUCIHANDLE_H
#include "uci.h"
#include <stab.h>
#include <string>
#include <map>
#include "sync/cslock.h"

class UCIHandle
{
public:
    UCIHandle(const std::string &path);
    UCIHandle();
    ~UCIHandle();

    //eg:network.lan.ifname
    bool GetValue(const char* configName, char* value);
    bool GetValue(const char* configName, int*  value);

    bool GetAllOption(const char* configName, std::map<std::string, std::string>& optionMap);

    bool SetValue(const char* configName, const char* value);
    bool SetValue(const char* configName, int value);

private:
    CMutexLock m_mutex;
    std::string m_uciConfigPath;
};

#endif // CUCIHANDLE_H
