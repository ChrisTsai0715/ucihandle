#include "ucihandle.h"
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <stdlib.h>

UCIHandle::UCIHandle(const std::string &path)
{
    m_uciConfigPath = path;
}

UCIHandle::UCIHandle()
{

}

UCIHandle::~UCIHandle()
{
}

bool UCIHandle::GetValue(const char *configName, char *value)
{
    CAutoLockEx<CMutexLock> m_lock(m_mutex);
    struct uci_context *ctx = uci_alloc_context();
    if (!m_uciConfigPath.empty())
        uci_set_confdir(ctx, m_uciConfigPath.c_str());

    char strConfigName[100] = {0};
    strcpy(strConfigName, configName);
    struct uci_ptr p;
    if(UCI_OK != uci_lookup_ptr(ctx, &p, strConfigName, true))
    {
        uci_perror(ctx, "[uci] get value err");
        return false;
    }

    if (0 == p.o)
    {
        uci_free_context(ctx);
        return false;
    }

    strcpy(value, p.o->v.string);
    uci_free_context(ctx);

    return true;
}

bool UCIHandle::GetValue(const char *configName, int *value)
{
    char strValue[30] = {0};
    if (!GetValue(configName, strValue))
        return false;

    if (0 == strValue[0])
        return false;

    *value = atoi(strValue);
    return true;
}

bool UCIHandle::SetValue(const char *configName, const char *value)
{    
    CAutoLockEx<CMutexLock> m_lock(m_mutex);
    struct uci_context *ctx = uci_alloc_context();
    if (!m_uciConfigPath.empty())
        uci_set_savedir(ctx, m_uciConfigPath.c_str());

    std::string strConfig(configName);
    std::string strSplit[3];
    unsigned int numSplits = 0;
    size_t start, pos;
    start = 0;
    do
    {
        pos = strConfig.find_first_of(".", start);
        if (pos == start)
        {
            start = pos + 1;
        }
        else if (pos == std::string::npos)
        {
            strSplit[numSplits] = strConfig.substr(start);
            break;
        }
        else
        {
            strSplit[numSplits] = strConfig.substr(start, pos - start);
            start = pos + 1;
        }
        start = strConfig.find_first_not_of(".", start);
        ++numSplits;
    } while (pos != std::string::npos);

    struct uci_ptr ptr = {
        UCI_TYPE_ITEM,
        uci_ptr::UCI_LOOKUP_COMPLETE,
        0,
        0,
        0,
        0,
        strSplit[0].c_str(),
        strSplit[1].c_str(),
        strSplit[2].c_str(),
        value,
    };

    if (UCI_OK != uci_set(ctx,&ptr))
    {
        uci_perror(ctx, "[uci] set value err");
        uci_unload(ctx,ptr.p); //卸载包
        return false;
    }
    uci_commit(ctx, &ptr.p, false); //提交保存更改
    uci_unload(ctx,ptr.p); //卸载包
    uci_free_context(ctx);

    return true;
}

bool UCIHandle::SetValue(const char *configName, int value)
{
    char strValue[50] = {0};
    sprintf(strValue, "%d", value);
    return SetValue(configName, strValue);
}

bool UCIHandle::GetAllOption(const char *configName, std::map<std::string, std::string> &optionMap)
{
    CAutoLockEx<CMutexLock> m_lock(m_mutex);
    struct uci_context *ctx = uci_alloc_context();

    if (!m_uciConfigPath.empty())
        uci_set_confdir(ctx, m_uciConfigPath.c_str());

    char strConfigName[100] = {0};
    strcpy(strConfigName, configName);

    struct uci_ptr p;
    if(UCI_OK != uci_lookup_ptr(ctx, &p, strConfigName, true))
    {
        uci_perror(ctx, "[uci] get value err");
        goto cleanup;
    }

    uci_element *e;
    uci_foreach_element(&p.s->options , e)
    {
        uci_option *option = uci_to_option(e);
        optionMap[option->e.name] = option->v.string;
    }

cleanup:
    uci_free_context(ctx);
    return true;
}
