#pragma once

#include <vector>
#include <string>
#include <functional>

#include "col.hpp"
#include "model.hpp"

namespace boosql {


class select_model : public model
{
    typedef enum {left, right, inner} join_type;

    class join_t {
    public:
        join_t(select_model & selector, select_model & model) : _selector(selector), model(model) {}

        join_t & on(std::string main)
        {
            return on(main, _selector.table_ref());
        }

        join_t & on(std::string main, std::string table_name)
        {
            if (ons.size() > 0) {
                ons.push_back(col().o_and());
            }

            ons.push_back(col(main, table_name));

            return *this;
        }

        join_t & operator () (std::string oper, col second)
        {
            ons.push_back(col()(oper));
            ons.push_back(second.table_name(model.table_ref()));

            return *this;
        }

        join_t & or_on(std::string main)
        {
            return or_on(main, _selector.table_ref());
        }

        join_t & or_on(std::string main, std::string table_name)
        {
            ons.push_back(col().o_or());
            ons.push_back(col(main, table_name));

            return *this;
        }

        select_model & end()
        {
            return _selector;
        }

    public:
        join_type type;
        select_model & model;
        std::vector<col> ons;
    private:
        select_model & _selector;
    };
public:
    select_model() {}
    select_model(adapter * adapter): model(adapter) {}
    select_model(const select_model & m) : model(m)
    {
        for (auto i = m._joins.begin(); i != m._joins.end(); ++i) {
            join_t join(*this, (*i).model);
            join.ons = (*i).ons;
            _joins.push_back(join);
        }
        _select = m._select;
        _groupby_columns = m._groupby_columns;
        _having_condition = m._having_condition;
        _order_by = m._order_by;
        _limit = m._limit;
        _offset = m._offset;
    }
    select_model(adapter * adapter, const std::string & table_name) 
    : model(adapter, table_name)
    {}

    select_model(const std::string & table_name) : model(table_name)
    {}

    virtual ~select_model() {}

    void copy_select(const select_model & m)
    {
    }

    template <typename... Args>
    select_model& select(const col& c, Args&&... columns) {
        _select.push_back(c);
        select(columns...);
        return *this;
    }

    // for recursion
    select_model& select() {
        return *this;
    }

    select_model& from(const std::string& table_name)
    {
        _table_name = table_name;
        return *this;
    }
    
    select_model & from(select_model & m)
    {
        _from_model = &m;
        _is_from_model = true;
        return *this;
    }

    select_model& and_where(const std::string & condition)
    {
        model::and_where(condition);
        return *this;
    }

    select_model& and_where(const col & condition)
    {
        model::and_where(condition);
        return *this;
    }

    select_model& or_where(const std::string & condition)
    {
        model::or_where(condition);
        return *this;
    }

    select_model& or_where(const col & condition)
    {
        model::or_where(condition);
        return *this;
    }

    select_model& quote(std::function<void(select_model& model)> callback)
    {
        model::quote<select_model>(callback, *this);
        return *this;
    }

    select_model& where(const std::string& condition) {
        model::where(condition);
        return *this;
    }

    select_model& where(const col& condition) {
        model::where(condition);
        return *this;
    }

    join_t & left_join(select_model & m) {
        return join(m, left);
    }

    join_t & right_join(select_model &m) {
        return join(m, right);
    }

    join_t & inner_join(select_model &m) {
        return join(m, inner);
    }

    template <typename... Args>
    select_model& group_by(const col& c, Args&&...columns) {
        _groupby_columns.push_back(c);
        group_by(columns...);
        return *this;
    }

    // for recursion
    select_model& group_by() {
        return *this;
    }

    select_model& having(const std::string& condition) {
        _having_condition.push_back(col("")(condition));
        return *this;
    }

    select_model& having(const col& condition) {
        _having_condition.push_back(condition);
        return *this;
    }

    select_model& order_by(const col& order_by) {
        _order_by.push_back(order_by);
        return *this;
    }

    template <typename T>
    select_model& limit(const T& limit) {
        _limit = std::to_string(limit);

        return *this;
    }

    select_model& page(const int& page, const int& page_size) {
        offset((page - 1) * page_size);
        limit(page_size);
        return *this;
    }

    template <typename T>
    select_model& offset(const T& offset) {
        _offset = std::to_string(offset);
        return *this;
    }

    std::string table_str()
    {
        if (_is_from_model) {
            return "(" + _from_model->str() + ") __m__";
        }
        return _table_name;
    }

    std::string table_ref()
    {
        if (_is_from_model) {
            return "__m__";
        }

        return _table_name;
    }

    std::string table_str(std::vector<std::string> & p)
    {
        if (_is_from_model) {
            return "(" + _from_model->str(p) + ") __m__";
        }
        return _table_name;
    }

    const std::string& str() override
    {
        _sql.clear();
        _sql.append("SELECT ");
        _sql.append(select_str());
        _sql.append(" FROM ");
        _sql.append(table_str());
        _sql.append(join_str());
        append_where();
        _sql.append(group_by_str());
        _sql.append(having_str());
        _sql.append(order_by_str());
        if(!_limit.empty()) {
            _sql.append(" LIMIT ");
            _sql.append(_limit);
        }
        if(!_offset.empty()) {
            _sql.append(" OFFSET ");
            _sql.append(_offset);
        }
        return _sql;
    }

    const std::string & str(std::vector<std::string> &params) override
    {
        _sql.clear();
        _sql.append("SELECT ");
        _sql.append(select_str());
        _sql.append(" FROM ");
        _sql.append(table_str(params));
        _sql.append(join_str());
        append_where(params);
        _sql.append(group_by_str());
        _sql.append(having_str());
        _sql.append(order_by_str());
        if(!_limit.empty()) {
            _sql.append(" LIMIT ");
            _sql.append(_limit);
        }
        if(!_offset.empty()) {
            _sql.append(" OFFSET ");
            _sql.append(_offset);
        }
        return _sql;
    }

    std::string order_by_str()
    {
        std::string ret;
        auto size = _order_by.size();
        if (size > 0) {
            ret.append(" ORDER BY ");
            for (size_t i = 0; i < size; ++i) {
                ret.append(_order_by[i].str(_adapter, table_ref()));
                if(i < size - 1) {
                    ret.append(", ");
                }
            }
        }

        return ret;
    }

    std::string join_str()
    {
        std::string ret;
        for (auto i = _joins.begin(); i != _joins.end(); ++i) {
            switch ((*i).type) {
            case left:
                ret.append(" LEFT");
                break;
            case right:
                ret.append(" RIGHT");
                break;
            case inner:
                ret.append(" INNER");
                break;
            }
            ret.append(" JOIN " + _adapter->quote_field((*i).model.table_name()));
            ret.append(" ON ");
            auto ons = (*i).ons;
            for (auto j = ons.begin(); j != ons.end(); ++j) {
                ret.append((*j).str(_adapter));
            }
        }

        return ret;
    }

    std::string where_str()
    {
        std::string ret = model::where_str();
        for (auto i = _joins.begin(); i != _joins.end(); ++i) {
            auto s = (*i).model.where_str();
            if (ret.length() > 0 && s.length() > 0) {
                ret.append(" AND (" + s + ")");
            } else if (s.length() > 0) {
                ret.append("(" + s + ")");
            }
        }

        return ret;
    }

    std::string where_str(std::vector<std::string> & params)
    {
        std::string ret = model::where_str(params);
        for (auto i = _joins.begin(); i != _joins.end(); ++i) {
            if (ret.length() > 0) {
                ret.append(" AND ");
            }
            auto s = (*i).model.where_str(params);
            if (s.length() > 0) {
                ret.append("(" + s + ")");
            }
        }

        return ret;
    }

    std::string having_str()
    {
        std::string ret;
        auto size = _having_condition.size();
        if(size > 0) {
            ret.append(" HAVING ");
            for(size_t i = 0; i < size; ++i) {
                ret.append(_having_condition[i].str(_adapter, table_ref()));
                if(i < size - 1) {
                    _sql.append(" ");
                }
            }
        }

        return ret;
    }

    std::string group_by_str()
    {
        std::string ret;
        auto size = _groupby_columns.size();
        if(size > 0) {
            ret.append(" GROUP BY ");
            for(size_t i = 0; i < size; ++i) {
                ret.append(_groupby_columns[i].str(_adapter, table_ref()));
                if(i < size - 1) {
                    ret.append(", ");
                }
            }
        }

        return ret;
    }


    std::string select_str()
    {
        std::string ret = "";
        unsigned count = 0;
        for (auto i = _select.begin(); i != _select.end(); ++i) {
            count++;
            ret.append((*i).str(_adapter, table_ref()));
            if (count < _select.size()) {
                ret.append(", ");
            }
        }
        for (auto i = _joins.begin(); i != _joins.end(); ++i) {
            if (ret.length() > 0) {
                ret.append(", ");
            }
            ret.append((*i).model.select_str());
        }

        return ret;
    }

    select_model& reset() {
        model::reset();
        _select.clear();
        _groupby_columns.clear();
        _having_condition.clear();
        _order_by.clear();
        _limit.clear();
        _offset.clear();
        return *this;
    }

    friend inline std::ostream& operator<< (std::ostream& out, select_model& mod) {
        out << mod.str();
        return out;
    }

 private:

    join_t & join(select_model & m, join_type type) {
        join_t j(*this, m);
        j.type = type;
        _joins.push_back(j);

        return _joins.back();
    }

private:
    std::vector<join_t> _joins;
    std::vector<col> _select;
    std::vector<col> _groupby_columns;
    std::vector<col> _having_condition;
    std::vector<col> _order_by;
    std::string _limit;
    std::string _offset;
    bool _is_from_model = false;

    select_model * _from_model;
};

}
