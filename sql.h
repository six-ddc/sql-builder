#ifndef _SQL_H_
#define _SQL_H_

#include <vector>
#include <string>
// #include <ctime>
#include <map>

class column;

class SqlHelper
{
public:
    template <typename T>
    static std::string to_string(const T& data) {
        return std::to_string(data);
    }

    template <size_t N>
    static std::string to_string(char const(&data)[N]) {
        std::string str("'");
        str.append(data);
        str.append("'");
        return str;
    }
};

template <>
std::string SqlHelper::to_string<std::string>(const std::string& data);

template <>
std::string SqlHelper::to_string<const char*>(const char* const& data);

/*
template <>
std::string SqlHelper::to_string<time_t>(const time_t& data);
*/

class SqlValue
{
    friend class DataModel;
public:
    SqlValue() {}
    virtual ~SqlValue() {}

    template <typename T>
    SqlValue(const T& v) {
        _value = SqlHelper::to_string(v);
    }

    template <typename T>
    SqlValue& operator=(const T& v) {
        _value = SqlHelper::to_string(v);
        return *this;
    }

    std::string str() const {
        if(*_value.begin() == '\'' && *_value.rbegin() == '\'') {
            return std::string(_value.c_str() + 1, _value.size() - 2);
        } else {
            return _value;
        }
    }

private:
    std::string _value;
};

class DataModel : public std::map<std::string, SqlValue>, public SqlHelper
{
public:
    virtual ~DataModel() {}
    void insert_format(std::vector<std::string> &c, std::vector<std::string> &v) const {
        for(const auto& value : *this) {
            c.push_back(value.first);
            v.push_back(value.second._value);
        }
    }

    void update_format(std::vector<std::string> &v) const {
        for(const auto& value : *this) {
            std::string str(value.first);
            str.append(" = ");
            str.append(value.second._value);
            v.push_back(str);
        }
    }

    template <typename T, typename... Args>
    DataModel& set(const std::string& k, const T& v, Args&&... more) {
        (*this)[k] = SqlValue(v);
        set(more...);
        return *this;
    }
    
    // for recursion
    DataModel& set() {
        return *this;
    }
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

    SelectModel& where(column& condition); 

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

    SelectModel& having(column& condition); 

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

    virtual const std::string& str() override;

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

    template <typename T, typename... Args>
    InsertModel& insert(const std::string& c, const T& data, Args&&... more) {
        _columns.push_back(c);
        _values.push_back(SqlHelper::to_string(data));
        insert(more...);
        return *this;
    }

    // for recursion
    InsertModel& insert() {
        return *this;
    }

    InsertModel& insert(const DataModel& mod) {
        mod.insert_format(_columns, _values);
        return *this;
    }

    inline void into(const std::string& table_name) {
        _table_name = table_name;
    }

    virtual const std::string& str() override;

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
    std::string _table_name;
    std::vector<std::string> _columns;
    std::vector<std::string> _values;
};

class UpdateModel : public SqlModel
{
public:
    UpdateModel() {}
    virtual ~UpdateModel() {}

    UpdateModel& update(const std::string& table_name) {
        _table_name = table_name;
        return *this;
    }

    template <typename T, typename... Args>
    UpdateModel& set(const std::string& c, const T& data, Args&&... more) {
        std::string str(c);
        str.append(" = ");
        str.append(SqlHelper::to_string(data));
        _set_columns.push_back(str);
        set(more...);
        return *this;
    }

    // for recursion
    UpdateModel& set() {
        return *this;
    }

    UpdateModel& set(const DataModel& mod) {
        mod.update_format(_set_columns);
        return *this;
    }

    UpdateModel& where(const std::string& condition) {
        _where_condition.push_back(condition);
        return *this;
    }

    UpdateModel& where(column& condition); 

    virtual const std::string& str() override;

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

    DeleteModel& where(column& condition); 

    virtual const std::string& str() override;

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
            _cond.append(SqlHelper::to_string(args[0]));
        } else {
            _cond.append(" in (");
            for(size_t i = 0; i < size; ++i) {
                if(i < size - 1) {
                    _cond.append(SqlHelper::to_string(args[i]));
                    _cond.append(", ");
                } else {
                    _cond.append(SqlHelper::to_string(args[i]));
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
            _cond.append(SqlHelper::to_string(args[0]));
        } else {
            _cond.append(" not in (");
            for(size_t i = 0; i < size; ++i) {
                if(i < size - 1) {
                    _cond.append(SqlHelper::to_string(args[i]));
                    _cond.append(", ");
                } else {
                    _cond.append(SqlHelper::to_string(args[i]));
                }
            }
            _cond.append(")");
        }
        return *this;
    }

    column& operator &&(column& condition);
    column& operator ||(column& condition);
    column& operator &&(const std::string& condition);
    column& operator ||(const std::string& condition);
    column& operator &&(const char* condition);
    column& operator ||(const char* condition);

    template <typename T>
    column& operator ==(const T& data) {
        _cond.append(" = ");
        _cond.append(SqlHelper::to_string(data));
        return *this;
    }

    template <typename T>
    column& operator !=(const T& data) {
        _cond.append(" != ");
        _cond.append(SqlHelper::to_string(data));
        return *this;
    }

    template <typename T>
    column& operator >=(const T& data) {
        _cond.append(" >= ");
        _cond.append(SqlHelper::to_string(data));
        return *this;
    }

    template <typename T>
    column& operator <=(const T& data) {
        _cond.append(" <= ");
        _cond.append(SqlHelper::to_string(data));
        return *this;
    }

    template <typename T>
    column& operator >(const T& data) {
        _cond.append(" > ");
        _cond.append(SqlHelper::to_string(data));
        return *this;
    }

    template <typename T>
    column& operator <(const T& data) {
        _cond.append(" < ");
        _cond.append(SqlHelper::to_string(data));
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

#endif
