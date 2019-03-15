#pragma once

#include "model.hpp"

#include <string>

namespace boosql {

class delete_model : public model
{
public:
    delete_model() {}
    delete_model(std::shared_ptr<adapter> adapter) : model(adapter) {}

    virtual ~delete_model() {}

    delete_model& from(const std::string& table_name) {
        _table_name = table_name;
        return *this;
    }

    delete_model& and_where(const std::string & condition)
    {
        model::and_where(condition);
        return *this;
    }

    delete_model& and_where(const col & condition)
    {
        model::and_where(condition);
        return *this;
    }

    delete_model& or_where(const std::string & condition)
    {
        model::or_where(condition);
        return *this;
    }

    delete_model& or_where(const col & condition)
    {
        model::or_where(condition);
        return *this;
    }

    delete_model& quote(std::function<void(delete_model& model)> callback)
    {
        model::quote<delete_model>(callback, *this);
        return *this;
    }

    delete_model& where(const std::string& condition) {
        model::where(condition);
        return *this;
    }

    delete_model& where(const col& condition) {
        model::where(condition);
        return *this;
    }
    
    const std::string & table_name()
    {
        return _table_name;
    }

    const std::string& str() override {
        _sql.clear();
        _sql.append("DELETE FROM ");
        _sql.append(_adapter->quote_field(_table_name));
        append_where();

        return _sql;
    }

    const std::string& str(std::vector<std::string> & params) override {
        _sql.clear();
        _sql.append("DELETE FROM ");
        _sql.append(_adapter->quote_field(_table_name));
        append_where(params);

        return _sql;
    }

    delete_model& reset() {
        _table_name.clear();
        model::reset();
        return *this;
    }
    friend inline std::ostream& operator<< (std::ostream& out, delete_model& mod) {
        out << mod.str();
        return out;
    }

protected:
    std::string _table_name;
};

}