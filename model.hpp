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
    const std::string & table_name()
    {
        return _table_name;
    }

    model()
    {
        _adapter = new sqlite_adapter();
        _auto_delete_adapter = true;
    }

    model(const model & m)
    {
        _adapter = m._adapter->clone();
        _auto_delete_adapter = true;

        _table_name = m._table_name;
        _where_condition = m._where_condition;
    }

    model(adapter * adapter) : _adapter(adapter) {}

    model(adapter * adapter, const std::string & table_name) 
    : _adapter(adapter), _table_name(table_name)
    {
    }

    model(const std::string & table_name)
    {
        _adapter = new sqlite_adapter();
        _auto_delete_adapter = true;
        _table_name = table_name;
    }

    virtual std::string where_str()
    {
        std::string ret;
        for(auto i = _where_condition.begin(); i != _where_condition.end(); ++i) {
            ret.append((*i).str(_adapter, table_name()));
        }

        return ret;
    }

    virtual std::string where_str(std::vector<std::string> & params)
    {
        std::string ret;
        for(auto i = _where_condition.begin(); i != _where_condition.end(); ++i) {
            ret.append((*i).str(_adapter, table_name(), params));
        }

        return ret;
    }

    virtual ~model()
    {
        if (_auto_delete_adapter) {
            delete _adapter;
        }
    }

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
        _where_condition.push_back(col()(condition));
    }
    void and_where(const col & condition)
    {
        _where_condition.push_back(col().o_and());
        _where_condition.push_back(condition);
    }
    void or_where(const std::string & condition)
    {
        _where_condition.push_back(col().o_or());
        _where_condition.push_back(col()(condition));
    }
    void or_where(const col & condition)
    {
        _where_condition.push_back(col().o_or());
        _where_condition.push_back(condition);
    }
    template <class T>
    void quote(std::function<void(T& model)> callback, T& model)
    {
        if (_where_condition.size() > 0) {
            _where_condition.push_back(col().o_and());
        }
        _where_condition.push_back(col().quote_begin());
        callback(model);
        _where_condition.push_back(col().quote_end());
    }
    void where(const std::string& condition) {
        if (_where_condition.size() > 0) {
            _where_condition.push_back(col().o_and());
        }
        _where_condition.push_back(col()(condition));
    }
    void where(const col& condition) {
        int s = _where_condition.size();
        if (s > 0) {
            col last = _where_condition.back();
            if (!last.empty() && last.last().val != "(") {
                _where_condition.push_back(col().o_and());
            }
        }
        _where_condition.push_back(condition);
    }
    void reset()
    {
        _table_name.clear();
        _where_condition.clear();
    }

protected:
    std::string _sql;
    adapter * _adapter;
    bool _auto_delete_adapter = false;
    std::string _table_name;

private:
    std::vector<col> _where_condition;
};

}
