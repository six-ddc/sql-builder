#include "sql.h"
#include <iostream>
#include <sstream>

template <>
std::string SqlHelper::to_string<std::string>(const std::string& data) {
    return "'" + data + "'";
}

template <>
std::string SqlHelper::to_string<const char*>(const char* const& data) {
    return "'" + std::string(data) + "'";
}

template <>
std::string SqlHelper::to_string<time_t>(const time_t& data) {
    char buff[128] = {0};
    struct tm* ttime = localtime(&data);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", ttime);
    return "'" + std::string(buff) + "'";
}

std::string InsertModel::str() {
    std::stringstream c_ss, v_ss;
    c_ss<<"insert into "<<_table_name<<"(";
    v_ss<<" values(";
    size_t size = _columns.size();
    for(int i = 0; i < size; ++i) {
        if(i < size - 1) {
            c_ss<<_columns[i]<<", ";
            v_ss<<_values[i]<<", ";
        } else {
            c_ss<<_columns[i]<<")";
            v_ss<<_values[i]<<")";
        }
    }
    c_ss<<v_ss.str();
    _sql = c_ss.str();
    return _sql;
}

std::string SelectModel::str() {
    std::stringstream ss_w;
    ss_w<<"select ";
    size_t size = _select_columns.size();
    for(size_t i = 0; i < size; ++i) {
        if(i < size - 1) {
            ss_w<<_select_columns[i]<<", ";
        } else {
            ss_w<<_select_columns[i];
        }
    }
    ss_w<<" from "<<_table_name;
    size = _where_condition.size();
    if(size > 0) {
        ss_w<<" where ";
        for(size_t i = 0; i < size; ++i) {
            if(i < size - 1) {
                ss_w<<_where_condition[i]<<" ";
            } else {
                ss_w<<_where_condition[i];
            }
        }
    }
    _sql = ss_w.str();
    return _sql;
}

SelectModel& SelectModel::where(column& condition) {
    _where_condition.push_back(condition.str());
    return *this;
}

UpdateModel& UpdateModel::where(column& condition) {
    _where_condition.push_back(condition.str());
    return *this;
}

std::string UpdateModel::str() {
    std::stringstream ss_w;
    ss_w<<"update "<<_table_name<<" set ";
    size_t size = _set_columns.size();
    for(size_t i = 0; i < size; ++i) {
        if(i < size - 1) {
            ss_w<<_set_columns[i]<<", ";
        } else {
            ss_w<<_set_columns[i];
        }
    }
    size = _where_condition.size();
    if(size > 0) {
        ss_w<<" where ";
        for(size_t i = 0; i < size; ++i) {
            if(i < size - 1) {
                ss_w<<_where_condition[i]<<" ";
            } else {
                ss_w<<_where_condition[i];
            }
        }
    }
    _sql = ss_w.str();
    return _sql;
}

DeleteModel& DeleteModel::where(column& condition) {
    _where_condition.push_back(condition.str());
    return *this;
}

std::string DeleteModel::str() {
    std::stringstream ss_w;
    ss_w<<"delete from "<<_table_name;
    size_t size = _where_condition.size();
    if(size > 0) {
        ss_w<<" where ";
        for(size_t i = 0; i < size; ++i) {
            if(i < size - 1) {
                ss_w<<_where_condition[i]<<" ";
            } else {
                ss_w<<_where_condition[i];
            }
        }
    }
    _sql = ss_w.str();
    return _sql;
}

column& column::operator &&(column& condition) {
    condition._cond = "(" + _cond + ") and (" + condition._cond + ")";
    return condition;
}

column& column::operator ||(column& condition) {
    condition._cond = "(" + _cond + ") or (" + condition._cond + ")";
    return condition;
}

column& column::operator &&(const std::string& condition) {
    _cond = _cond + " and " + condition;
    return *this;
}

column& column::operator ||(const std::string& condition) {
    _cond = _cond + " or " + condition;
    return *this;
}

column& column::operator &&(const char* condition) {
    _cond = _cond + " and " + std::string(condition);
    return *this;
}

column& column::operator ||(const char* condition) {
    _cond = _cond + " or " + std::string(condition);
    return *this;
}
