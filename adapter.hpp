#pragma once

#include <memory>
#include <string>

namespace boosql {

class adapter : public std::enable_shared_from_this<adapter>
{
public:
    virtual std::string quote_value(const std::string & value) = 0;
    virtual std::string quote_field(const std::string & field) = 0;
    virtual std::string placeholder() = 0;
};

class sqlite_adapter : public adapter
{
public:
    std::string quote_value(const std::string & value) override
    {
        return "'" + value + "'";
    }

    std::string quote_field(const std::string & field) override
    {
        return "\"" + field + "\"";
    }

    std::string placeholder() override
    {
        return "?";
    }
};

}