#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace boosql {

 using namespace std;

template <typename T>
inline std::string to_value(const T& data) {
    return std::to_string(data);
}

template <size_t N>
inline std::string to_value(char const(&data)[N]) {
    std::string str("'");
    str.append(data);
    str.append("'");
    return str;
}

template <>
inline std::string to_value<std::string>(const std::string& data) {
    std::string str("'");
    str.append(data);
    str.append("'");
    return str;
}

template <>
inline std::string to_value<const char*>(const char* const& data) {
    std::string str("'");
    str.append(data);
    str.append("'");
    return str;
}

/*
template <>
static std::string sql::to_value<time_t>(const time_t& data) {
    char buff[128] = {0};
    struct tm* ttime = localtime(&data);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", ttime);
    std::string str("'");
    str.append(buff);
    str.append("'");
    return str;
}
*/

class sql_model 
{
public:
    sql_model() {
        auto adapter = make_shared<sqlite_adapter>();
        _adapter = adapter->shared_from_this();
    }
    sql_model(shared_ptr<adapter> adapter) : _adapter(adapter->shared_from_this()) {}

    column col(const string & col)
    {
        return column(_adapter->quote_field(col));
    }

    virtual ~sql_model() {}

    virtual const std::string& str() = 0;
    const std::string& last_sql() {
        return _sql;
    }
private:
    sql_model(const sql_model& m) = delete;
    sql_model& operator =(const sql_model& data) = delete;
    shared_ptr<adapter> _adapter;

protected:
    std::string _sql;
};

class select_model : public sql_model
{
public:
    select_model() {}
    virtual ~select_model() {}

    template <typename... Args>
    select_model& select(const std::string& str, Args&&... columns) {
        _select_columns.push_back(str);
        select(columns...);
        return *this;
    }

    // for recursion
    select_model& select() {
        return *this;
    }

    template <typename... Args>
    select_model& from(const std::string& table_name, Args&&... tables) {
        if(_table_name.empty()) {
            _table_name = table_name;
        } else {
            _table_name.append(", ");
            _table_name.append(table_name);
        }
        from(tables...);
        return *this;
    }
    
    // for recursion
    select_model& from() {
        return *this;
    }

    select_model& where(const std::string& condition) {
        _where_condition.push_back(condition);
        return *this;
    }

    select_model& where(column& condition) {
        _where_condition.push_back(condition.str());
        return *this;
    }

    template <typename... Args>
    select_model& group_by(const std::string& str, Args&&...columns) {
        _groupby_columns.push_back(str);
        group_by(columns...);
        return *this;
    }

    // for recursion
    select_model& group_by() {
        return *this;
    }

    select_model& having(const std::string& condition) {
        _having_condition.push_back(condition);
        return *this;
    }

    select_model& having(column& condition) {
        _having_condition.push_back(condition.str());
        return *this;
    }

    select_model& order_by(const std::string& order_by) {
        _order_by = order_by;
        return *this;
    }

    template <typename T>
    select_model& limit(const T& limit) {
        _limit = std::to_string(limit);
        return *this;
    }
    template <typename T>
    select_model& limit(const T& offset, const T& limit) {
        _offset = std::to_string(offset);
        _limit = std::to_string(limit);
        return *this;
    }
    template <typename T>
    select_model& offset(const T& offset) {
        _offset = std::to_string(offset);
        return *this;
    }

    virtual const std::string& str() override {
        _sql.clear();
        _sql.append("select ");
        size_t size = _select_columns.size();
        for(size_t i = 0; i < size; ++i) {
            if(i < size - 1) {
                _sql.append(_select_columns[i]);
                _sql.append(", ");
            } else {
                _sql.append(_select_columns[i]);
            }
        }
        _sql.append(" from ");
        _sql.append(_table_name);
        size = _where_condition.size();
        if(size > 0) {
            _sql.append(" where ");
            for(size_t i = 0; i < size; ++i) {
                if(i < size - 1) {
                    _sql.append(_where_condition[i]);
                    _sql.append(" ");
                } else {
                    _sql.append(_where_condition[i]);
                }
            }
        }
        size = _groupby_columns.size();
        if(!_groupby_columns.empty()) {
            _sql.append(" group by ");
            for(size_t i = 0; i < size; ++i) {
                if(i < size - 1) {
                    _sql.append(_groupby_columns[i]);
                    _sql.append(", ");
                } else {
                    _sql.append(_groupby_columns[i]);
                }
            }
        }
        size = _having_condition.size();
        if(size > 0) {
            _sql.append(" having ");
            for(size_t i = 0; i < size; ++i) {
                if(i < size - 1) {
                    _sql.append(_having_condition[i]);
                    _sql.append(" ");
                } else {
                    _sql.append(_having_condition[i]);
                }
            }
        }
        if(!_order_by.empty()) {
            _sql.append(" order by ");
            _sql.append(_order_by);
        }
        if(!_limit.empty()) {
            _sql.append(" limit ");
            _sql.append(_limit);
        }
        if(!_offset.empty()) {
            _sql.append(" offset ");
            _sql.append(_offset);
        }
        return _sql;
    }

    select_model& reset() {
        _table_name.clear();
        _select_columns.clear();
        _groupby_columns.clear();
        _where_condition.clear();
        _having_condition.clear();
        _order_by.clear();
        _limit.clear();
        _offset.clear();
        return *this;
    }
    friend inline std::ostream& operator<< (std::ostream& out, select_model& mod) {
        out<<mod.str();
        return out;
    }

protected:
    std::vector<std::string> _select_columns;
    std::vector<std::string> _groupby_columns;
    std::string _table_name;
    std::vector<std::string> _where_condition;
    std::vector<std::string> _having_condition;
    std::string _order_by;
    std::string _limit;
    std::string _offset;
};



class insert_model : public sql_model
{
public:
    insert_model() {}
    virtual ~insert_model() {}

    template <typename T>
    insert_model& insert(const std::string& c, const T& data) {
        _columns.push_back(c);
        _values.push_back(to_value(data));
        return *this;
    }

    template <typename T>
    insert_model& operator()(const std::string& c, const T& data) {
        return insert(c, data);
    }

    insert_model& into(const std::string& table_name) {
        _table_name = table_name;
        return *this;
    }

    insert_model& replace(bool var) {
        _replace = var;
        return *this;
    }

    virtual const std::string& str() override {
        _sql.clear();
        std::string v_ss;

        if (_replace) {
            _sql.append("insert or replace into ");
        }else {
            _sql.append("insert into ");
        }

        _sql.append(_table_name);
        _sql.append("(");
        v_ss.append(" values(");
        size_t size = _columns.size();
        for(size_t i = 0; i < size; ++i) {
            if(i < size - 1) {
                _sql.append(_columns[i]);
                _sql.append(", ");
                v_ss.append(_values[i]);
                v_ss.append(", ");
            } else {
                _sql.append(_columns[i]);
                _sql.append(")");
                v_ss.append(_values[i]);
                v_ss.append(")");
            }
        }
        _sql.append(v_ss);
        return _sql;
    }

    insert_model& reset() {
        _table_name.clear();
        _columns.clear();
        _values.clear();
        return *this;
    }

    friend inline std::ostream& operator<< (std::ostream& out, insert_model& mod) {
        out<<mod.str();
        return out;
    }

protected:
    bool _replace = false;
    std::string _table_name;
    std::vector<std::string> _columns;
    std::vector<std::string> _values;
};

template <>
inline insert_model& insert_model::insert(const std::string& c, const std::nullptr_t&) {
    _columns.push_back(c);
    _values.push_back("null");
    return *this;
}


class update_model : public sql_model
{
public:
    update_model() {}
    virtual ~update_model() {}

    update_model& update(const std::string& table_name) {
        _table_name = table_name;
        return *this;
    }

    template <typename T>
    update_model& set(const std::string& c, const T& data) {
        std::string str(c);
        str.append(" = ");
        str.append(to_value(data));
        _set_columns.push_back(str);
        return *this;
    }

    template <typename T>
    update_model& operator()(const std::string& c, const T& data) {
        return set(c, data);
    }

    update_model& where(const std::string& condition) {
        _where_condition.push_back(condition);
        return *this;
    }

    update_model& where(column& condition) {
        _where_condition.push_back(condition.str());
        return *this;
    }

    virtual const std::string& str() override {
        _sql.clear();
        _sql.append("update ");
        _sql.append(_table_name);
        _sql.append(" set ");
        size_t size = _set_columns.size();
        for(size_t i = 0; i < size; ++i) {
            if(i < size - 1) {
                _sql.append(_set_columns[i]);
                _sql.append(", ");
            } else {
                _sql.append(_set_columns[i]);
            }
        }
        size = _where_condition.size();
        if(size > 0) {
            _sql.append(" where ");
            for(size_t i = 0; i < size; ++i) {
                if(i < size - 1) {
                    _sql.append(_where_condition[i]);
                    _sql.append(" ");
                } else {
                    _sql.append(_where_condition[i]);
                }
            }
        }
        return _sql;
    }

    update_model& reset() {
        _table_name.clear();
        _set_columns.clear();
        _where_condition.clear();
        return *this;
    }
    friend inline std::ostream& operator<< (std::ostream& out, update_model& mod) {
        out<<mod.str();
        return out;
    }

protected:
    std::vector<std::string> _set_columns;
    std::string _table_name;
    std::vector<std::string> _where_condition;
};

template <>
inline update_model& update_model::set(const std::string& c, const std::nullptr_t&) {
    std::string str(c);
    str.append(" = null");
    _set_columns.push_back(str);
    return *this;
}


class delete_model : public sql_model
{
public:
    delete_model() {}
    virtual ~delete_model() {}

    delete_model& _delete() {
        return *this;
    }

    template <typename... Args>
    delete_model& from(const std::string& table_name, Args&&... tables) {
        if(_table_name.empty()) {
            _table_name = table_name;
        } else {
            _table_name.append(", ");
            _table_name.append(table_name);
        }
        from(tables...);
        return *this;
    }
    
    // for recursion
    delete_model& from() {
        return *this;
    }

    delete_model& where(const std::string& condition) {
        _where_condition.push_back(condition);
        return *this;
    }

    delete_model& where(column& condition) {
        _where_condition.push_back(condition.str());
        return *this;
    }

    virtual const std::string& str() override {
        _sql.clear();
        _sql.append("delete from ");
        _sql.append(_table_name);
        size_t size = _where_condition.size();
        if(size > 0) {
            _sql.append(" where ");
            for(size_t i = 0; i < size; ++i) {
                if(i < size - 1) {
                    _sql.append(_where_condition[i]);
                    _sql.append(" ");
                } else {
                    _sql.append(_where_condition[i]);
                }
            }
        }
        return _sql;
    }

    delete_model& reset() {
        _table_name.clear();
        _where_condition.clear();
        return *this;
    }
    friend inline std::ostream& operator<< (std::ostream& out, delete_model& mod) {
        out<<mod.str();
        return out;
    }

protected:
    std::string _table_name;
    std::vector<std::string> _where_condition;
};

}
