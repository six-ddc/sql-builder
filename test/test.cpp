#include <iostream>
#include <cassert>
#include <sstream>

#include "sql.h"

/*

create table if not exists user (
	`id` int(10) unsigned not null auto_increment,
	`age` tinyint(8) unsigned,
	`score` int(10) unsigned not null default 0,
    `name` varchar(128) not null default '',
    `address` varchar(256),
    `create_time` datetime not null,
    primary key(`id`)
)

*/

using namespace sql;

int main()
{
    std::string example{""};
    // Insert
    InsertModel i;
    i.insert("score", 100)
            ("name", std::string("six"))
            ("age", (unsigned char)20)
            ("address", "beijing")
            ("create_time", nullptr)
        .into("user");
    example = "insert into user (score, name, age, address, create_time) values (100, 'six', 20, 'beijing', null)";
    assert(example.compare(i.querry()) == 0);

    // Insert with named parameters
    InsertModel iP;
    Param score {":score"};
    Param name {":name"};
    Param age {":age"};
    Param address {":address"};
    Param create_time {":create_time"};
    iP.insert("score", score)
            ("name", name)
            ("age", age)
            ("address", address)
            ("create_time", create_time)
        .into("user");
    example = "insert into user (score, name, age, address, create_time) values (:score, :name, :age, :address, :create_time)";
    assert(example.compare(iP.querry()) == 0);   

    // Select
    SelectModel s;
    s.select("id as user_id", "age", "name", "address")
        .distinct()
        .from("user")
        .join("score")
        .on(column("user.id") == column("score.id") && column("score.id") > 60)
        .where(column("score") > 60 && (column("age") >= 20 || column("address").is_not_null()))
        // .where(column("score") > 60 && (column("age") >= 20 || column("address").is_not_null()))
        .group_by("age")
        .having(column("age") > 10)
        .order_by("age desc")
        .limit(10)
        .offset(1);
    example = "select distinct id as user_id, age, name, address from user join score on (user.id = score.id) and (score.id > 60) where (score > 60) and ((age >= 20) or (address is not null)) group by age having age > 10 order by age desc limit 10 offset 1";
    assert(example.compare(s.querry()) == 0);

    // Update
    std::vector<int> a = {1, 2, 3};
    UpdateModel u;
    u.update("user")
        .set("name", "ddc")
            ("age", 18)
            ("score", nullptr)
            ("address", "beijing")
        .where(column("id").in(a));
    example = "update user set name = 'ddc', age = 18, score = null, address = 'beijing' where id in (1, 2, 3)";
    assert(example.compare(u.querry()) == 0);

    // Update with positional parameters
    UpdateModel uP;
    Param mark = "?";
    uP.update("user")
        .set("name", mark)
            ("age", mark)
            ("score", mark)
            ("address", mark)
        .where(column("id").in(a));
    example = "update user set name = ?, age = ?, score = ?, address = ? where id in (1, 2, 3)";
    assert(example.compare(uP.querry()) == 0);

    // Delete
    DeleteModel d;
    d._delete()
        .from("user")
        .where(column("id") == 1);
    example = "delete from user where id = 1";
    assert(example.compare(d.querry()) == 0);

    return 0;
}
