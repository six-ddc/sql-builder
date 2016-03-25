#include "sql.h"

template <>
std::string SqlHelper::to_string<std::string>(const std::string& data) {
    std::string str("'");
    str.append(data);
    str.append("'");
    return str;
}

template <>
std::string SqlHelper::to_string<const char*>(const char* const& data) {
    std::string str("'");
    str.append(data);
    str.append("'");
    return str;
}

template <>
std::string SqlHelper::to_string<time_t>(const time_t& data) {
    char buff[128] = {0};
    struct tm* ttime = localtime(&data);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", ttime);
    std::string str("'");
    str.append(buff);
    str.append("'");
    return str;
}

const std::string& InsertModel::str() {
    _sql.clear();
    std::string v_ss;
    _sql.append("insert into ");
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

const std::string& SelectModel::str() {
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

SelectModel& SelectModel::where(column& condition) {
    _where_condition.push_back(condition.str());
    return *this;
}

SelectModel& SelectModel::having(column& condition) {
    _having_condition.push_back(condition.str());
    return *this;
}

UpdateModel& UpdateModel::where(column& condition) {
    _where_condition.push_back(condition.str());
    return *this;
}

const std::string& UpdateModel::str() {
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

DeleteModel& DeleteModel::where(column& condition) {
    _where_condition.push_back(condition.str());
    return *this;
}

const std::string& DeleteModel::str() {
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

column& column::operator &&(column& condition) {
    std::string str("(");
    str.append(_cond);
    str.append(") and (");
    str.append(condition._cond);
    str.append(")");
    condition._cond = str;
    return condition;
}

column& column::operator ||(column& condition) {
    std::string str("(");
    str.append(_cond);
    str.append(") or (");
    str.append(condition._cond);
    str.append(")");
    condition._cond = str;
    return condition;
}

column& column::operator &&(const std::string& condition) {
    _cond.append(" and ");
    _cond.append(condition);
    return *this;
}

column& column::operator ||(const std::string& condition) {
    _cond.append(" or ");
    _cond.append(condition);
    return *this;
}

column& column::operator &&(const char* condition) {
    _cond.append(" and ");
    _cond.append(condition);
    return *this;
}

column& column::operator ||(const char* condition) {
    _cond.append(" or ");
    _cond.append(condition);
    return *this;
}
