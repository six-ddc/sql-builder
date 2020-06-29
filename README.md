# sql-builder

[![Build Status](https://travis-ci.org/six-ddc/sql-builder.svg?branch=master)](https://travis-ci.org/six-ddc/sql-builder)

♥️ SQL query string builder for C++11

## Examples:

``` c++
    // Insert
    InsertModel i;
    i.insert("score", 100)
            ("name", std::string("six"))
            ("age", (unsigned char)20)
            ("address", "beijing")
            ("create_time", nullptr)
        .into("user");
    assert(i.str() ==
            "insert into user(score, name, age, address, create_time) values(100, 'six', 20, 'beijing', null)");

    // Insert with named parameters
    InsertModel iP;
    Param score = ":score";
    Param name = ":name";
    Param age = ":age";
    Param address = ":address";
    Param create_time = ":create_time";
    iP.insert("score", score)
            ("name", name)
            ("age", age)
            ("address", address)
            ("create_time", create_time)
        .into("user");
    assert(iP.str() ==
            "insert into user(score, name, age, address, create_time) values(:score, :name, :age, :address, :create_time)");

    // Select
    SelectModel s;
    s.select("id as user_id", "age", "name", "address")
        .distinct()
        .from("user")
        .join("score")
        .on(column("user.id") == column("score.id") and column("score.id") > 60)
        .where(column("score") > 60 and (column("age") >= 20 or column("address").is_not_null()))
        // .where(column("score") > 60 && (column("age") >= 20 || column("address").is_not_null()))
        .group_by("age")
        .having(column("age") > 10)
        .order_by("age desc")
        .limit(10)
        .offset(1);
    std::cout << s.str() << std::endl;
    assert(s.str() ==
            "select distinct id as user_id, age, name, address from user join score on (user.id = score.id) and (score.id > 60) where (score > 60) and ((age >= 20) or (address is not null)) group by age having age > 10 order by age desc limit 10 offset 1");

    // Update
    std::vector<int> a = {1, 2, 3};
    UpdateModel u;
    u.update("user")
        .set("name", "ddc")
            ("age", 18)
            ("score", nullptr)
            ("address", "beijing")
        .where(column("id").in(a));
    assert(u.str() ==
            "update user set name = 'ddc', age = 18, score = null, address = 'beijing' where id in (1, 2, 3)");

    // Update with positional parameters
    UpdateModel uP;
    Param mark = "?";
    uP.update("user")
        .set("name", mark)
            ("age", mark)
            ("score", mark)
            ("address", mark)
        .where(column("id").in(a));
    assert(uP.str() ==
            "update user set name = ?, age = ?, score = ?, address = ? where id in (1, 2, 3)");

    // Delete
    DeleteModel d;
    d._delete()
        .from("user")
        .where(column("id") == 1);
    assert(d.str() ==
            "delete from user where id = 1");
```
