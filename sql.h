#pragma once

#include <vector>
#include <string>

using string = std::string;

namespace sql {

    class column;

    class Param
    {
        public:
            Param (const std::string &param) : _param(param) {}
            Param (const char *param) : _param(param) {}

        public:
            std::string operator()() const { return param(); }
            inline std::string param() const { return _param; }

        private:
            const std::string _param;
    };


//BEGIN to_value блок. --------------------------------------------------
//Нужно разобраться в семантической надобности этих функций и привести 
//их в нормальное состояние.
  
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

    template <>
    inline std::string to_value<Param>(const Param& data) {
        return data();
    }

    template <>
    inline std::string to_value<column>(const column& data);

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

//END to_value блок.  ----------------------------------------------------

    template <typename T>
    void join_vector(std::string& result, const std::vector<T>& vec, const char* sep) {
        for (auto item : vec)
            result.append(item)
                .append(sep);
        result.erase(//erase extra sep
            std::begin(result) + result.rfind(sep),
            std::end(result)
        );
    }

    // template <typename T>
    // void join_vector(std::string& result, const std::vector<T>& vec, const char* sep) {
    //     size_t size = vec.size();
    //     for(size_t i = 0; i < size; ++i) {
    //         if(i < size - 1) {
    //             result.append(vec[i]);
    //             result.append(sep);
    //         } else {
    //             result.append(vec[i]);
    //         }
    //     }
    // }

    class column
    {
    public:
        column(const std::string& column): _cond(column) {}
        virtual ~column() {}

        operator std::string() const {return _cond;}

        column& as(const std::string& s) {
            _cond.append(" as ")
                .append(s);
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
        column& add_args(const std::vector<T>& input_args, bool in_condition = true) {
            const string delimiter {", "};

            if(input_args.size() == 1) 
                _cond.append(in_condition ? " " : " !")
                    .append("= ")
                    .append(to_value(input_args.at(0)));
            else if (input_args.size() > 1) {
                _cond.append(in_condition ? " " : " not ")
                    .append("in (");
                //input args with delimiter ', '
                for (auto arg:input_args)
                    _cond.append(to_value(arg))
                        .append(delimiter);
                //remove extra delimiter
                _cond.pop_back();
                _cond.pop_back();
                _cond.append(")");
            }
            return *this;
        }

        template <typename T>
        column& in(const std::vector<T>& input_args) {
            return add_args(input_args);
        }
        template <typename T>
        column& not_in(const std::vector<T>& input_args) {
            return add_args(input_args, false);
        }

        column& operators(column& condition, std::string oper) {
            std::string str{"("+_cond+")"};
            str.append(oper)
                .append("("+condition._cond+")");
            condition._cond = std::move(str);
            return condition;
        }

        column& operator &&(column& condition) {
            return operators(condition, " and ");
        }

        column& operator ||(column& condition) {
            return operators(condition, " or ");
        }

        column& operator &&(const std::string& condition) {
            _cond.append(" and ")
                .append(condition);
            return *this;
        }

        column& operator ||(const std::string& condition) {
            _cond.append(" or ")
                .append(condition);
            return *this;
        }

        column& operator &&(const char* condition) {
            _cond.append(" and ")
                .append(condition);
            return *this;
        }

        column& operator ||(const char* condition) {
            _cond.append(" or ")
                .append(condition);
            return *this;
        }

        template <typename T>
        column& operator ==(const T& data) {
            _cond.append(" = ")
                .append(to_value(data));
            return *this;
        }

        template <typename T>
        column& operator !=(const T& data) {
            _cond.append(" != ")
                .append(to_value(data));
            return *this;
        }

        template <typename T>
        column& operator >=(const T& data) {
            _cond.append(" >= ")
                .append(to_value(data));
            return *this;
        }

        template <typename T>
        column& operator <=(const T& data) {
            _cond.append(" <= ")
                .append(to_value(data));
            return *this;
        }

        template <typename T>
        column& operator >(const T& data) {
            _cond.append(" > ")
                .append(to_value(data));
            return *this;
        }

        template <typename T>
        column& operator <(const T& data) {
            _cond.append(" < ")
                .append(to_value(data));
            return *this;
        }

        operator bool() {
            return !_cond.empty();
        }
    private:
        std::string _cond;
    };

    template <>
    inline std::string to_value<column>(const column& data) {
        return data;
    }

    template<typename Model>
    class SqlModel 
    {
    public:
        SqlModel() {}
        SqlModel(std::string table): _table_name(table) {}
        virtual ~SqlModel() {}

        virtual operator std::string() = 0;
        const std::string& last_sql() {
            return _sql;
        }
        virtual bool operator == (std::string arg){
            return arg.compare(*this);
        }
        
        template<typename T>
        Model& where(T& condition) {
            _where_condition.push_back(condition);
            return *(static_cast<Model*>(this));
        }
        
    private:
        SqlModel(const SqlModel& m) = delete;
        SqlModel& operator =(const SqlModel& data) = delete;
    protected:
        std::string                 _sql;
        std::vector<std::string>    _where_condition;
        std::string                 _table_name;
    };

    class SelectModel : public SqlModel<SelectModel>
    {
    public:
        SelectModel() : _distinct(false) {}
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

        SelectModel& distinct() {
            _distinct = true;
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

        SelectModel& join(const std::string& table_name) {
            _join_type = "join";
            _join_table = table_name;
            return *this;
        }

        SelectModel& left_join(const std::string& table_name) {
            _join_type = "left join";
            _join_table = table_name;
            return *this;
        }

        SelectModel& left_outer_join(const std::string& table_name) {
            _join_type = "left outer join";
            _join_table = table_name;
            return *this;
        }

        SelectModel& right_join(const std::string& table_name) {
            _join_type = "right join";
            _join_table = table_name;
            return *this;
        }

        SelectModel& right_outer_join(const std::string& table_name) {
            _join_type = "right outer join";
            _join_table = table_name;
            return *this;
        }

        SelectModel& full_join(const std::string& table_name) {
            _join_type = "full join";
            _join_table = table_name;
            return *this;
        }

        SelectModel& full_outer_join(const std::string& table_name) {
            _join_type = "full outer join";
            _join_table = table_name;
            return *this;
        }

        SelectModel& on(const std::string& condition) {
            _join_on_condition.push_back(condition);
            return *this;
        }

        SelectModel& on(const column& condition) {
            _join_on_condition.push_back(condition);
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

        SelectModel& having(const column& condition) {
            _having_condition.push_back(condition);
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

        operator std::string() {
            _sql.clear();
            _sql.append("select ");
            if(_distinct) {
                _sql.append("distinct ");
            }
            join_vector(_sql, _select_columns, ", ");
            _sql.append(" from ");
            _sql.append(_table_name);
            if(!_join_type.empty()) {
                _sql.append(" ");
                _sql.append(_join_type);
                _sql.append(" ");
                _sql.append(_join_table);
            }
            if(!_join_on_condition.empty()) {
                _sql.append(" on ");
                join_vector(_sql, _join_on_condition, " and ");
            }
            if(!_where_condition.empty()) {
                _sql.append(" where ");
                join_vector(_sql, _where_condition, " and ");
            }
            if(!_groupby_columns.empty()) {
                _sql.append(" group by ");
                join_vector(_sql, _groupby_columns, ", ");
            }
            if(!_having_condition.empty()) {
                _sql.append(" having ");
                join_vector(_sql, _having_condition, " and ");
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
            _select_columns.clear();
            _distinct = false;
            _groupby_columns.clear();
            _table_name.clear();
            _join_type.clear();
            _join_table.clear();
            _join_on_condition.clear();
            _where_condition.clear();
            _having_condition.clear();
            _order_by.clear();
            _limit.clear();
            _offset.clear();
            return *this;
        }
        friend inline std::ostream& operator<< (std::ostream& out, SelectModel& mod) {
            out<<mod;
            return out;
        }

    protected:
        std::vector<std::string> _select_columns;
        bool _distinct;
        std::vector<std::string> _groupby_columns;
        std::string _join_type;
        std::string _join_table;
        std::vector<std::string> _join_on_condition;
        std::vector<std::string> _having_condition;
        std::string _order_by;
        std::string _limit;
        std::string _offset;
    };


    class InsertModel : public SqlModel<InsertModel>
    {
    public:
        InsertModel() {}
        InsertModel(std::string targetTable):SqlModel(targetTable){}
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

        InsertModel& set_default(const std::string& column) {
            _columns.push_back(column);
            _values.push_back("DEFAULT");
            return *this;
        }

        InsertModel& replace(bool var) {
            _replace = var;
            return *this;
        }

        std::string vec_to_str(std::vector<std::string> vec, std::string delimiter = ", "){
            std::string result{""};
            
            for(auto element:vec)
                result.append((result.empty() ? element : (delimiter + element)));
            return result;
        }

        std::string get_values(){
            string result = std::move(vec_to_str(_values));

            if (!result.empty())
                result = (" values ("+result+")");
            return result;
        }

        std::string get_columns(){
            string result = std::move(vec_to_str(_columns));

            if (!result.empty())
                result = (" ("+result+")");
            return result;
        }

        operator std::string() {
            if (!_sql.empty())
                _sql.clear();
            if (!_values.empty() //not empty values
                && (_columns.size() == _values.size() //and (columns and values sizes eqaul)
                    || _columns.empty())){//or values empty
                _sql.append("insert")
                    .append(_replace ? " or replace " : " ")
                    .append("into ")
                    .append(_table_name)
                    .append(std::move(get_columns()))
                    .append(std::move(get_values()));
            }
            return _sql;
        }

        InsertModel& clear() {
            _table_name.clear();
            _columns.clear();
            _values.clear();
            return *this;
        }

        InsertModel& reset() {
            _columns.clear();
            _values.clear();
            return *this;
        }

        friend inline std::ostream& operator<< (std::ostream& out, InsertModel& mod) {
            out<<mod;
            return out;
        }

    protected:
        bool _replace = false;
        std::vector<std::string> _columns;
        std::vector<std::string> _values;
    };

    template <>
    inline InsertModel& InsertModel::insert(const std::string& c, const std::nullptr_t&) {
        _columns.push_back(c);
        _values.push_back("null");
        return *this;
    }


    class UpdateModel : public SqlModel<UpdateModel>
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
            std::string str{c};
            str.append(" = ")
                .append(to_value(data));
            _set_columns.push_back(str);
            return *this;
        }

        template <typename T>
        UpdateModel& operator()(const std::string& c, const T& data) {
            return set(c, data);
        }
        
        operator std::string() {
            if (!_sql.empty())
                _sql.clear();
                
            _sql.append("update ")
                .append(_table_name)
                .append(" set ");
            join_vector(_sql, _set_columns, ", ");

            if(_where_condition.size() > 0) {
                _sql.append(" where ");
                join_vector(_sql, _where_condition, " and ");
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
            out<<mod;
            return out;
        }

    protected:
        std::vector<std::string> _set_columns;//нужо разделить на _columns и _values как это сделано в InsertModel
    };

    template <>
    inline UpdateModel& UpdateModel::set(const std::string& c, const std::nullptr_t&) {
        std::string str(c);
        str.append(" = null");
        _set_columns.push_back(str);
        return *this;
    }


    class DeleteModel : public SqlModel<DeleteModel>
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

        virtual operator std::string() {
            _sql.clear();
            _sql.append("delete from ");
            _sql.append(_table_name);
            size_t size = _where_condition.size();
            if(size > 0) {
                _sql.append(" where ");
                join_vector(_sql, _where_condition, " and ");
            }
            return _sql;
        }

        DeleteModel& reset() {
            _table_name.clear();
            _where_condition.clear();
            return *this;
        }
        friend inline std::ostream& operator<< (std::ostream& out, DeleteModel& mod) {
            out<<mod;
            return out;
        }

    };

}
