#pragma once

#include "model.hpp"

#include <map>
#include <functional>

namespace boosql
{

class insert_model : public model
{
    class row_interface {
    public:
        virtual void each(std::function<void(std::string, std::string)>) = 0;
        virtual const std::string & fields(adapter *) = 0;
        virtual const std::string & values(adapter *) = 0;
        virtual const std::string & values(adapter *, std::vector<std::string> &) = 0;
    };

    class row : public row_interface {
    public:
        row(insert_model & model): _model(model) {}

        template <typename T>
        row& insert(const std::string& c, const T& data) {
            std::ostringstream str;
            str << data;
            _data[c] = str.str();
            return *this;
        }

        template <typename T>
        row& operator()(const std::string& c, const T& data) {
            return insert(c, data);
        }

        void each(std::function<void(std::string, std::string)> handle)
        {
            for (auto i = _data.begin(); i != _data.end(); ++i) {
                handle(i->first, i->second);
            }
        }

        insert_model & next_row() {
            return _model;
        }

        const std::string & fields(adapter * adapter) override
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

        const std::string & values(adapter * adapter) override
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

        const std::string & values(adapter * adapter, std::vector<std::string> & params) override
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
        std::map<std::string, std::string> _data;
        std::string _fields;
        std::string _values;
    };

public:
    insert_model() {}
    insert_model(adapter * a) : model(a) {}
    insert_model(const insert_model & m) : model(m)
    {
        _replace = m._replace;
        _table_name = m._table_name;
        for (auto i = m._rows.begin(); i != m._rows.end(); ++i) {
            row * r = new row(*this);
            (*i)->each([&r](std::string field, std::string val) {
                r->insert(field, val);
            });
            _rows.push_back(r);
        }
    }
    insert_model(adapter * a, const std::string & table_name)
        : model(a, table_name)
    {}

    insert_model(const std::string & table_name)
    : model(table_name)
    {}

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

    const std::string& str() override {
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
                _sql.append((*i)->fields(_adapter));
                _sql.append(" VALUES");
            }
            _sql.append((*i)->values(_adapter));
            if (count < size) {
                _sql.append(", ");
            }
        }

        return _sql;
    }

    const std::string &str(std::vector<std::string> &params) override
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
                _sql.append((*i)->fields(_adapter));
                _sql.append(" VALUES");
            }
            _sql.append((*i)->values(_adapter, params));
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
    std::vector<row_interface *> _rows;
};

}