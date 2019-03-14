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

    string where_str()
    {
        string ret;
        auto size = _where_condition.size();
        if(size > 0) {
            ret.append(" WHERE ");
            for(size_t i = 0; i < size; ++i) {
                ret.append(_where_condition[i].str(_adapter.get(), table_name()));
            }
        }

        return ret;
    }

    virtual ~model() {}

    virtual const string & table_name() = 0;
    virtual const std::string& str() = 0;

protected:
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