#pragma once

#include <vector>
#include <string>
#include <map>

namespace sql {

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

class column
{
public:
    column(const std::string& column) {
        _cond = column;
    }
    virtual ~column() {}

    column& as(const std::string& s) {
        _cond.append(" as ");
        _cond.append(s);
        return *this;
    }

    column& is_null() {
        _cond.append(" is null");
        return *this;
    }

    column& is_not_null() {
        _cond.append(" is not null");
        return *this;
    }

    template <typename T>
    column& in(const std::vector<T>& args) {
        size_t size = args.size();
        if(size == 1) {
            _cond.append(" = ");
            _cond.append(to_value(args[0]));
        } else {
            _cond.append(" in (");
            for(size_t i = 0; i < size; ++i) {
                if(i < size - 1) {
                    _cond.append(to_value(args[i]));
                    _cond.append(", ");
                } else {
                    _cond.append(to_value(args[i]));
                }
            }
            _cond.append(")");
        }
        return *this;
    }

    template <typename T>
    column& not_in(const std::vector<T>& args) {
        size_t size = args.size();
        if(size == 1) {
            _cond.append(" != ");
            _cond.append(to_value(args[0]));
        } else {
            _cond.append(" not in (");
            for(size_t i = 0; i < size; ++i) {
                if(i < size - 1) {
                    _cond.append(to_value(args[i]));
                    _cond.append(", ");
                } else {
                    _cond.append(to_value(args[i]));
                }
            }
            _cond.append(")");
        }
        return *this;
    }

    column& operator &&(column& condition) {
        std::string str("(");
        str.append(_cond);
        str.append(") and (");
        str.append(condition._cond);
        str.append(")");
        condition._cond = str;
        return condition;
    }

    column& operator ||(column& condition) {
        std::string str("(");
        str.append(_cond);
        str.append(") or (");
        str.append(condition._cond);
        str.append(")");
        condition._cond = str;
        return condition;
    }

    column& operator &&(const std::string& condition) {
        _cond.append(" and ");
        _cond.append(condition);
        return *this;
    }

    column& operator ||(const std::string& condition) {
        _cond.append(" or ");
        _cond.append(condition);
        return *this;
    }

    column& operator &&(const char* condition) {
        _cond.append(" and ");
        _cond.append(condition);
        return *this;
    }

    column& operator ||(const char* condition) {
        _cond.append(" or ");
        _cond.append(condition);
        return *this;
    }

    template <typename T>
    column& operator ==(const T& data) {
        _cond.append(" = ");
        _cond.append(to_value(data));
        return *this;
    }

    template <typename T>
    column& operator !=(const T& data) {
        _cond.append(" != ");
        _cond.append(to_value(data));
        return *this;
    }

    template <typename T>
    column& operator >=(const T& data) {
        _cond.append(" >= ");
        _cond.append(to_value(data));
        return *this;
    }

    template <typename T>
    column& operator <=(const T& data) {
        _cond.append(" <= ");
        _cond.append(to_value(data));
        return *this;
    }

    template <typename T>
    column& operator >(const T& data) {
        _cond.append(" > ");
        _cond.append(to_value(data));
        return *this;
    }

    template <typename T>
    column& operator <(const T& data) {
        _cond.append(" < ");
        _cond.append(to_value(data));
        return *this;
    }

    const std::string& str() const {
        return _cond;
    }

    operator bool() {
        return true;
    }
private:
    std::string _cond;
};


class SqlModel 
{
public:
    SqlModel() {}
    virtual ~SqlModel() {}

    virtual const std::string& str() = 0;
    const std::string& last_sql() {
        return _sql;
    }
private:
    SqlModel(const SqlModel& m) = delete;
    SqlModel& operator =(const SqlModel& data) = delete;
protected:
    std::string _sql;
};

class SelectModel : public SqlModel
{
public:
    SelectModel() {}
    virtual ~SelectModel() {}

    template <typename... Args>
    SelectModel& select(const std::string& str, Args&&... columns) {
        _select_columns.push_back(str);
        select(columns...);
        return *this;
    }

    // for recursion
    SelectModel& select() {
        return *this;
    }

    template <typename... Args>
    SelectModel& from(const std::string& table_name, Args&&... tables) {
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
    SelectModel& from() {
        return *this;
    }

    SelectModel& where(const std::string& condition) {
        _where_condition.push_back(condition);
        return *this;
    }

    SelectModel& where(column& condition) {
        _where_condition.push_back(condition.str());
        return *this;
    }

    template <typename... Args>
    SelectModel& group_by(const std::string& str, Args&&...columns) {
        _groupby_columns.push_back(str);
        group_by(columns...);
        return *this;
    }

    // for recursion
    SelectModel& group_by() {
        return *this;
    }

    SelectModel& having(const std::string& condition) {
        _having_condition.push_back(condition);
        return *this;
    }

    SelectModel& having(column& condition) {
        _having_condition.push_back(condition.str());
        return *this;
    }

    SelectModel& order_by(const std::string& order_by) {
        _order_by = order_by;
        return *this;
    }

    template <typename T>
    SelectModel& limit(const T& limit) {
        _limit = std::to_string(limit);
        return *this;
    }
    template <typename T>
    SelectModel& limit(const T& offset, const T& limit) {
        _offset = std::to_string(offset);
        _limit = std::to_string(limit);
        return *this;
    }
    template <typename T>
    SelectModel& offset(const T& offset) {
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

    SelectModel& reset() {
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
    friend inline std::ostream& operator<< (std::ostream& out, SelectModel& mod) {
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



class InsertModel : public SqlModel
{
public:
    InsertModel() {}
    virtual ~InsertModel() {}

    template <typename T>
    InsertModel& insert(const std::string& c, const T& data) {
        _columns.push_back(c);
        _values.push_back(to_value(data));
        return *this;
    }

    template <typename T>
    InsertModel& operator()(const std::string& c, const T& data) {
        return insert(c, data);
    }

    InsertModel& into(const std::string& table_name) {
        _table_name = table_name;
        return *this;
    }

    InsertModel& replace(bool var) {
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

    InsertModel& reset() {
        _table_name.clear();
        _columns.clear();
        _values.clear();
        return *this;
    }

    friend inline std::ostream& operator<< (std::ostream& out, InsertModel& mod) {
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
inline InsertModel& InsertModel::insert(const std::string& c, const std::nullptr_t&) {
    _columns.push_back(c);
    _values.push_back("null");
    return *this;
}


class UpdateModel : public SqlModel
{
public:
    UpdateModel() {}
    virtual ~UpdateModel() {}

    UpdateModel& update(const std::string& table_name) {
        _table_name = table_name;
        return *this;
    }

    template <typename T>
    UpdateModel& set(const std::string& c, const T& data) {
        std::string str(c);
        str.append(" = ");
        str.append(to_value(data));
        _set_columns.push_back(str);
        return *this;
    }

    template <typename T>
    UpdateModel& operator()(const std::string& c, const T& data) {
        return set(c, data);
    }

    UpdateModel& where(const std::string& condition) {
        _where_condition.push_back(condition);
        return *this;
    }

    UpdateModel& where(column& condition) {
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

    UpdateModel& reset() {
        _table_name.clear();
        _set_columns.clear();
        _where_condition.clear();
        return *this;
    }
    friend inline std::ostream& operator<< (std::ostream& out, UpdateModel& mod) {
        out<<mod.str();
        return out;
    }

protected:
    std::vector<std::string> _set_columns;
    std::string _table_name;
    std::vector<std::string> _where_condition;
};

template <>
inline UpdateModel& UpdateModel::set(const std::string& c, const std::nullptr_t&) {
    std::string str(c);
    str.append(" = null");
    _set_columns.push_back(str);
    return *this;
}


class DeleteModel : public SqlModel
{
public:
    DeleteModel() {}
    virtual ~DeleteModel() {}

    DeleteModel& _delete() {
        return *this;
    }

    template <typename... Args>
    DeleteModel& from(const std::string& table_name, Args&&... tables) {
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
    DeleteModel& from() {
        return *this;
    }

    DeleteModel& where(const std::string& condition) {
        _where_condition.push_back(condition);
        return *this;
    }

    DeleteModel& where(column& condition) {
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

    DeleteModel& reset() {
        _table_name.clear();
        _where_condition.clear();
        return *this;
    }
    friend inline std::ostream& operator<< (std::ostream& out, DeleteModel& mod) {
        out<<mod.str();
        return out;
    }

protected:
    std::string _table_name;
    std::vector<std::string> _where_condition;
};

}
