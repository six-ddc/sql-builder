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

int main() 
{
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
    s.select("id", "age", "name")
        .from("user")
        .where(column("score") > 60 and (column("age") >= 20 or column("address").is_not_null()));
    std::cout<<s<<std::endl;
    // select id, age, name from user where (score > 60) and ((age >= 20) or (address is not null))

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

    return 0;
}
