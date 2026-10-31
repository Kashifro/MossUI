#pragma once

#include <string>

namespace MossUI {

// Persistent key-value storage behind Setting::Var & co. The mod adapts its own settings file to this once:
//   MossUI::SetStore(&myStore);
class Store {
public:
    virtual ~Store() = default;
    virtual bool GetBool(const std::string& key, bool def) = 0;
    virtual void SetBool(const std::string& key, bool value) = 0;
    virtual int GetInt(const std::string& key, int def) = 0;
    virtual void SetInt(const std::string& key, int value) = 0;
    virtual std::string GetString(const std::string& key, const std::string& def) = 0;
    virtual void SetString(const std::string& key, const std::string& value) = 0;
};

void SetStore(Store* store);
Store* CurrentStore();   // null until SetStore: Var rows then just write the variable

}
