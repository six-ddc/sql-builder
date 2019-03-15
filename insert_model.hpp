#pragma once

#include "model.hpp"

#include <map>

namespace boosql
{

class insert_model : public model
{
    class row_interface {
    public:
        virtual const string & fields(adapter *) = 0;
        virtual const string & values(adapter *) = 0;
        virtual const string & values(adapter *, vector<string> &) = 0;
    };

    class row : public row_interface {
    public:
        row(insert_model & model): _model(model) {}

        template <typename T>
        row& insert(const std::string& c, const T& data) {
            ostringstream str;
            str << data;
            _data[c] = str.str();
            return *this;
        }

        template <typename T>
        row& operator()(const std::string& c, const T& data) {
            return insert(c, data);
        }

        insert_model & next_row() {
            return _model;
        }

        const string & fields(adapter * adapter) override
        {
            if (_fields != "") {
                return _fields;
            }
            _fields = "(";
            auto size = _data.size();
            int count = 0;
            for (auto i = _data.begin(); i != _data.end(); ++i) {
                count++;
                _fields.append(col(i->first).str(adapter));
                if (count < size) {
                    _fields.append(", ");
                }
            }

            _fields.append(")");

            return _fields;
        }

        const string & values(adapter * adapter) override
        {
            if (_values != "") {
                return _values;
            }
            _values = "(";
            auto size = _data.size();
            int count = 0;
            for (auto i = _data.begin(); i != _data.end(); ++i) {
                count++;
                _values.append(adapter->quote_value(i->second));
                if (count < size) {
                    _values.append(", ");
                }
            }

            _values.append(")");

            return _values;
        }

        const string & values(adapter * adapter, vector<string> & params) override
        {
            if (_values != "") {
                return _values;
            }
            _values = "(";
            auto size = _data.size();
            int count = 0;
            for (auto i = _data.begin(); i != _data.end(); ++i) {
                count++;
                _values.append(adapter->placeholder());
                params.push_back(i->second);
                if (count < size) {
                    _values.append(", ");
                }
            }

            _values.append(")");

            return _values;
        }

    private:    
        insert_model & _model;
        map<string, string> _data;
        string _fields;
        string _values;
    };

public:
    insert_model(shared_ptr<adapter> adapter) : model(adapter) {}
    virtual ~insert_model()
    {
        auto i = _rows.begin();
        while (i != _rows.end()) {
            delete *i;
            _rows.erase(i);
            i = _rows.begin();
        }
    }

    row & next_row() {
        auto r = new row(*this);
        _rows.push_back(r);
        return *r;
    }

    template <typename T>
    row& operator()(const std::string& c, const T& data) {
        return next_row()(c, data);
    }

    insert_model& into(const std::string& table_name) {
        _table_name = table_name;
        return *this;
    }

    insert_model& replace(bool var) {
        _replace = var;
        return *this;
    }

    const string & table_name() override
    {
        return _table_name;
    }

    const string& str() override {
        _sql.clear();
        if (_replace) {
            _sql.append("INSERT INTO OR REPLACE INFO ");
        }else {
            _sql.append("INSERT INTO ");
        }
        _sql.append(_adapter->quote_field(_table_name));
        auto size = _rows.size();
        int count = 0;
        for (auto i = _rows.begin(); i != _rows.end(); ++i) {
            count++;
            if (count == 1) {
                _sql.append((*i)->fields(_adapter.get()));
                _sql.append(" VALUES");
            }
            _sql.append((*i)->values(_adapter.get()));
            if (count < size) {
                _sql.append(", ");
            }
        }

        return _sql;
    }

    const string &str(vector<string> &params) override
    {
        _sql.clear();
        if (_replace) {
            _sql.append("INSERT INTO OR REPLACE INFO ");
        }else {
            _sql.append("INSERT INTO ");
        }
        _sql.append(_adapter->quote_field(_table_name));
        auto size = _rows.size();
        int count = 0;
        for (auto i = _rows.begin(); i != _rows.end(); ++i) {
            count++;
            if (count == 1) {
                _sql.append((*i)->fields(_adapter.get()));
                _sql.append(" VALUES");
            }
            _sql.append((*i)->values(_adapter.get(), params));
            if (count < size) {
                _sql.append(", ");
            }
        }

        return _sql;
    }

    insert_model& reset() {
        _table_name.clear();
        _rows.clear();
        return *this;
    }

    friend inline std::ostream& operator << (std::ostream& out, insert_model& mod) {
        out << mod.str();
        return out;
    }

protected:
    bool _replace = false;
    string _table_name;
    
    vector<row_interface *> _rows;
};

}