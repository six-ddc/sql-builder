# sql-builder

[![Build Status](https://travis-ci.org/six-ddc/sql-builder.svg?branch=master)](https://travis-ci.org/six-ddc/sql-builder)

♥️ SQL query string builder for C++11

## Examples:

``` c++
    InsertModel i;
    i.insert("score", 100)
        .insert("name", std::string("six"))
        .insert("age", (unsigned char)20)
        .insert("address", "beijing")
        .insert("create_time", time(NULL))
        .into("user");
    std::cout<<i.str()<<std::endl;
    // insert into user(score, name, age, address, create_time) values(100, 'six', 20, 'beijing', '2016-03-25 10:15:59')

    SelectModel s;
    s.select("age")
        .from("user")
        .where(column("score") > 60 and (column("age") >= 20 or column("address").is_not_null()))
        .group_by("age")
        .having(column("age") > 10)
        .order_by("age desc")
        .limit(10)
        .offset(1);
    std::cout<<s<<std::endl;
    // select age from user where (score > 60) and ((age >= 20) or (address is not null)) group by age having age > 10 order by age desc limit 10 offset 1

    std::vector<int> a = {1, 2, 3};
    UpdateModel u;
    u.update("user")
        .set("name", "ddc")
        .set("age", 18)
        .where(column("id").in(a));
    std::cout<<u<<std::endl;
    // update user set name = 'ddc', age = 18 where id in (1, 2, 3)

    DeleteModel d;
    d._delete()
        .from("user")
        .where(column("id") > 1);
    std::cout<<d<<std::endl;
    // delete from user where id > 1

    SqlModel m;
    m["address"] = "chengdu";
    m["score"] = 80;
    m["create_time"] = time(NULL);

    u.reset();
    u.update("user")
        .set(m);
    std::cout<<u<<std::endl;
    // update user set address = 'chengdu', create_time = '2016-03-25 10:33:32', score = 80

    i.reset();
    i.insert(m)
        .into("user");
    std::cout<<i<<std::endl;
    // insert into user(address, create_time, score) values('chengdu', '2016-03-25 10:33:32', 80)
```


