#pragma once

#include "adapter.hpp"
#include "col.hpp"

#include <memory>
#include <vector>
#include <functional>

namespace boosql
{

using namespace std;

class model 
{
public:
    model() {
        auto adapter = make_shared<sqlite_adapter>();
        _adapter = adapter->shared_from_this();
    }

    model(shared_ptr<adapter> adapter) : _adapter(adapter->shared_from_this()) {}

    virtual string where_str()
    {
        string ret;
        for(auto i = _where_condition.begin(); i != _where_condition.end(); ++i) {
            ret.append((*i).str(_adapter.get(), table_name()));
        }

        return ret;
    }

    virtual string where_str(vector<string> & params)
    {
        string ret;
        for(auto i = _where_condition.begin(); i != _where_condition.end(); ++i) {
            ret.append((*i).str(_adapter.get(), table_name(), params));
        }

        return ret;
    }

    virtual ~model() {}

    virtual const string & table_name() = 0;
    virtual const std::string& str() = 0;
    virtual const std::string& str(vector<string> &) = 0;

protected:

    void append_where()
    {
        string w = where_str();
        if (w.length() > 0) {
            _sql.append( " WHERE " );
            _sql.append(w);
        }
    }

    void append_where(vector<string> & params)
    {
        string w = where_str(params);
        if (w.length() > 0) {
            _sql.append( " WHERE " );
            _sql.append(w);
        }
    }

    void and_where(const string & condition)
    {
        _where_condition.push_back(col("").and());
        where(condition);
    }
    void and_where(const col & condition)
    {
        _where_condition.push_back(col("").and());
        where(condition);
    }
    void or_where(const string & condition)
    {
        _where_condition.push_back(col("").or());
        where(condition);
    }
    void or_where(const col & condition)
    {
        _where_condition.push_back(col("").or());
        where(condition);
    }
    template <class T>
    void quote(std::function<void(T& model)> callback, T& model)
    {
        _where_condition.push_back(col("").and());
        _where_condition.push_back(col("").quote_begin());
        callback(model);
        _where_condition.push_back(col("").quote_end());
    }
    void where(const string& condition) {
        _where_condition.push_back(col("")(condition));
    }
    void where(const col& condition) {
        _where_condition.push_back(condition);
    }
    void reset()
    {
        _where_condition.clear();
    }

private:
    model(const model& m) = delete;
    model& operator =(const model& data) = delete;

private:
    vector<col> _where_condition;

protected:
    std::string _sql;
    shared_ptr<adapter> _adapter;
};

}