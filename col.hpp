#pragma once

#include <string>
#include <vector>
#include "adapter.hpp"

namespace boosql
{

using namespace std;

class col
{
    typedef enum {field, value, other} witch;

    struct item {
    public:
        witch type;
        string val;
    };

public:
    col() {}


    col(const string & c)
    {
        name(c);
    }

    col(const string & c, const string & tn)
    {
        name(c);
        table_name(tn);
    }

    virtual ~col() {}

    col & name(const string & c)
    {
        if (c != "") {
            _items.push_back(item{ field, c });
        }
        return *this;
    }

    col & table_name(const string & tn)
    {
        _table_name = tn;
        return *this;
    }

    col& as(const string& s)
    {
        _items.push_back(item{other, "AS " + s});
        return *this;
    }

    col& is_null() {
        _items.push_back(item{other, "IS NULL"});
        return *this;
    }

    col& is_not_null() {
        _items.push_back(item{other, "IS NOT NULL"});
        return *this;
    }

    template <typename T>
    col& in(const vector<T>& args) {
        return in_or_not(args, "IN");
    }

    template <typename T>
    col& not_in(const vector<T>& args) {
        return in_or_not(args, "NOT IN");
    }

    template<typename T>
    col & in_or_not(const vector<T>& args, string in)
    {
        size_t size = args.size();
        if(size == 1) {
            _items.push_back(item<string>{other, "="});
            ostringstream str;
            str << args[0];
            _items.push_back(item<string>{value, str.str()});
        } else {
            _items.push_back(item<string>{other, in + " ("});
            for(size_t i = 0; i < size; ++i) {
                ostringstream str;
                str << args[i];
                _items.push_back(item{value, str.str()});
                if(i < size - 1) {
                    _items.push_back(item{other, ","});
                }
            }
            _items.push_back(item{other, ")"});
        }
        return *this;
    }

    col & and()
    {
        _items.push_back(item{other, "AND"});
        return *this;
    }

    col & or()
    {
        _items.push_back(item{other, "OR"});
        return *this;
    }

    col & quote_begin()
    {
        _items.push_back(item{other, "("});
        return *this;
    }

    col & quote_end()
    {
        _items.push_back(item{other, ")"});
        return *this;
    }

    col& operator ()(const string & any)
    {
        _items.push_back(item{other, any});
        return *this;
    }

    template<typename T>
    col & val(const T& data)
    {
        ostringstream str;
        str << data;
        _items.push_back(item{value, str.str()});

        return *this;
    }

    template<typename T>
    col & val(const vector<T> &data)
    {
        for (auto i = data.begin(); i != data.end(); ++i) {
            ostringstream str;
            str << *i;
            _items.push_back(item{value, str.str()});
        }

        return *this;
    }

    col& operator &&(col & condition)
    {
        and();
        merge(condition);

        return *this;
    }

    col& operator ||(col& condition)
    {
        or ();
        merge(condition);

        return *this;
    }

    void merge(col & condition)
    {
        for (auto i = condition._items.begin(); i != condition._items.end(); ++i) {
            _items.push_back(*i);
        }
    }

    col& operator &&(const string& condition)
    {
        quote_begin();
        _items.push_back(item{other, condition});
        quote_end();

        return *this;
    }

    col& operator ||(const string& condition) {
        quote_begin();
        _items.push_back(item{other, condition});
        quote_end();

        return *this;
    }

    col& operator &&(const char* condition) {
        return operator &&(string(condition));
    }

    col& operator ||(const char* condition) {
        return operator ||(string(condition));
    }

    col& operator [] (const char * data) {
        return with_operator(data, "LIKE");
    }

    col& operator [] (const string & data) {
        return with_operator(data, "LIKE");
    }

    col & escape(const string & data) {
        _items.push_back(item{other, "{ ESCAPE "});
        _items.push_back(item{value, data});
        _items.push_back(item{other, "}"});

        return *this;
    }

    template <typename T>
    col& operator ==(const T& data) {
        return with_operator(data, "=");
    }

    template <typename T>
    col& operator !=(const T& data) {
        return with_operator(data, "!=");
    }

    template <typename T>
    col& operator >=(const T& data) {
        return with_operator(data, ">=");
    }

    template <typename T>
    col& operator <=(const T& data) {
        return with_operator(data, "<=");
    }

    template <typename T>
    col& operator >(const T& data) {
        return with_operator(data, ">");
    }

    template <typename T>
    col& operator <(const T& data) {
        return with_operator(data, "<");
    }

    template <typename T>
    col & with_operator(const T & data, const string & oper)
    {
        ostringstream str;
        str << data;
        _items.push_back(item{other, oper});
        _items.push_back(item{value, str.str()});

        return *this;
    }

    string str(adapter * adapter) const
    {
        return str(adapter, "");
    }

    string str(adapter * adapter, vector<string> & params) const
    {   
        return str(adapter, "", params);
    }

    string str(adapter * adapter, string table_name) const {
        string ret = "";
        string pre_col = "";
        if (table_name != "") {
            pre_col = adapter->quote_field(table_name) + ".";
        } else if (_table_name != "") {
            pre_col = adapter->quote_field(_table_name) + ".";
        }
        for(auto i = _items.begin(); i != _items.end(); ++i) {
            auto it = *i;
            switch(it.type) {
            case field:
                ret += pre_col + adapter->quote_field(it.val);
                break;
            case value:
                ret += adapter->quote_value(it.val);
                break;
            case other:
                ret += " " + it.val + " ";
            }
        }

        return ret;
    }

    string str(adapter * adapter, string table_name, vector<string> & params) const
    {
        string ret = "";
        string pre_col = "";
        if (table_name != "") {
            pre_col = adapter->quote_field(table_name) + ".";
        } else if (_table_name != "") {
            pre_col = adapter->quote_field(_table_name) + ".";
        }
        for(auto i = _items.begin(); i != _items.end(); ++i) {
            auto it = *i;
            switch(it.type) {
            case field:
                ret += pre_col + adapter->quote_field(it.val);
                break;
            case value:
                ret += adapter->placeholder();
                params.push_back(it.val);
                break;
            case other:
                ret += " " + it.val + " ";
            }
        }

        return ret;
    }

    static col pre_quote(const string & c)
    {
        return col().quote_begin().name(c);
    }

    operator bool() {
        return true;
    }
private:
    vector<item> _items;
    string _table_name = "";
};

}