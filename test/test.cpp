#include <iostream>
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
    InsertModel i;
    i.insert("score", 100)
        ("name", std::string("six"))
        ("age", (unsigned char)20)
        ("address", "beijing")
        ("create_time", nullptr)
        .into("user");
    std::cout<<i.str()<<std::endl;
    // insert into user(score, name, age, address, create_time) values(100, 'six', 20, 'beijing', '2016-03-25 10:15:59')

    SelectModel s;
    s.select("id", "age", "name", "address")
        .from("user")
        .where(column("score") > 60 and (column("age") >= 20 or column("address").is_not_null()))
        .group_by("age")
        .having(column("age") > 10)
        .order_by("age desc")
        .limit(10)
        .offset(1);
    std::cout<<s<<std::endl;
    // select id, age, name, address from user where (score > 60) and ((age >= 20) or (address is not null)) group by age having age > 10 order by age desc limit 10 offset 1

    std::vector<int> a = {1, 2, 3};
    UpdateModel u;
    u.update("user")
        .set("name", "ddc")
        ("age", 18)
        ("score", nullptr)
        ("address", "beijing")
        .where(column("id").in(a));
    std::cout<<u<<std::endl;
    // update user set name = 'ddc', age = 18, score = 18, address = 'beijing' where id in (1, 2, 3)

    DeleteModel d;
    d._delete()
        .from("user")
        .where(column("id") == 1);
    std::cout<<d<<std::endl;
    // delete from user where id = 1

    return 0;
}
