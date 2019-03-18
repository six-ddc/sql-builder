#pragma once

#include "adapter.hpp"
#include "col.hpp"

#include <memory>
#include <vector>
#include <functional>


namespace boosql
{

class model 
{
public:
    model() {
        auto adapter = std::make_shared<sqlite_adapter>();
        _adapter = adapter->shared_from_this();
    }

    model(const model & m)
    {
        _adapter = m._adapter->shared_from_this();
        _where_condition = m._where_condition;
    }

    model(std::shared_ptr<adapter> adapter) : _adapter(adapter->shared_from_this()) {}

    virtual std::string where_str()
    {
        std::string ret;
        for(auto i = _where_condition.begin(); i != _where_condition.end(); ++i) {
            ret.append((*i).str(_adapter.get(), table_name()));
        }

        return ret;
    }

    virtual std::string where_str(std::vector<std::string> & params)
    {
        std::string ret;
        for(auto i = _where_condition.begin(); i != _where_condition.end(); ++i) {
            ret.append((*i).str(_adapter.get(), table_name(), params));
        }

        return ret;
    }

    virtual ~model() {}

    virtual const std::string & table_name() = 0;
    virtual const std::string& str() = 0;
    virtual const std::string& str(std::vector<std::string> &) = 0;

protected:

    void append_where()
    {
        std::string w = where_str();
        if (w.length() > 0) {
            _sql.append( " WHERE " );
            _sql.append(w);
        }
    }

    void append_where(std::vector<std::string> & params)
    {
        std::string w = where_str(params);
        if (w.length() > 0) {
            _sql.append( " WHERE " );
            _sql.append(w);
        }
    }

    void and_where(const std::string & condition)
    {
        _where_condition.push_back(col().o_and());
        where(condition);
    }
    void and_where(const col & condition)
    {
        _where_condition.push_back(col().o_and());
        where(condition);
    }
    void or_where(const std::string & condition)
    {
        _where_condition.push_back(col().o_or());
        where(condition);
    }
    void or_where(const col & condition)
    {
        _where_condition.push_back(col().o_or());
        where(condition);
    }
    template <class T>
    void quote(std::function<void(T& model)> callback, T& model)
    {
        _where_condition.push_back(col().o_and());
        _where_condition.push_back(col().quote_begin());
        callback(model);
        _where_condition.push_back(col().quote_end());
    }
    void where(const std::string& condition) {
        _where_condition.push_back(col()(condition));
    }
    void where(const col& condition) {
        _where_condition.push_back(condition);
    }
    void reset()
    {
        _where_condition.clear();
    }

protected:
    std::string _sql;
    std::shared_ptr<adapter> _adapter;

private:
    std::vector<col> _where_condition;
};

}
