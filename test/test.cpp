#include <iostream>
#include <sstream>

#include "col.hpp"
#include "select_model.hpp"
#include "update_model.hpp"
#include "delete_model.hpp"
#include "insert_model.hpp"
#include "adapter.hpp"

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

using namespace boosql;

int main() 
{
    std::shared_ptr<sqlite_adapter> a = std::make_shared<sqlite_adapter>();

    select_model selector(a);
    selector.from("users")
        .select(col("*"))
        .where(boosql::col("hello")["%hello"])  // like
        .quote([](select_model & model) {
            model.where(col("id") != 1).or_where(col("id") != 2);
        });
    select_model group(a);

    group.from("group").select(col("a"), col("b"), col("c")).where(col("a") == 2);

    selector.left_join(group).on("hello")("=", col("a")).or_on("id")("=", col("b")).end();

    selector.group_by(col("hello")).order_by(col("hello")("DESC"));
    std::cout << selector.str() << std::endl;

    update_model updater(a);
    updater.update("users")("hello", "hello")("world", "world").where(col("id") == 2);
    std::cout << updater.str() << std::endl;

    delete_model deleter(a);
    deleter.from("users").where(col("id") == 1).or_where(col("name")["%hello"]);
    std::cout << deleter.str() << std::endl;

    insert_model insert(a);
    insert.into("users")
        ("id", 1)
        ("name", "hello")
    .next_row()
        ("id", 2)
        ("name", "world");

    std::cout << insert.str() << std::endl;

    return 0;
}
