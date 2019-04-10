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

void test_select_join()
{
    sqlite_adapter a;
    select_model selector(&a, "users");
    selector .select(col("*"))
        .where(boosql::col("hello")["%hello"])  // like
        .quote([](select_model & model) {
            model.where(col("id") != 1).or_where(col("id") != 2);
        });
    select_model group(&a, "group");

    group.select(col("a"), col("b"), col("c")).where(col("a") == 2);

    selector.left_join(group).on("hello")("=", col("a")).or_on("id")("=", col("b")).end();

    selector.group_by(col("hello")).order_by(col("hello")("DESC"));

    select_model another(selector);

    another.order_by(col("hello")("ASC"));

    std::cout << selector.str() << std::endl;
    std::cout << another.str() << std::endl;
}

void test_select_where()
{
    sqlite_adapter a;
    select_model s(&a, "users");
    s.select(col()("*"))
        .where(col("hello") == "hello")
        .where(col("world") == "world")
        .or_where(col("hw") == "hw")
        .quote([](select_model & s) {
            s.where(col("a") != "1")
                .or_where(col("b") == "1");
        });
    std::cout << s.str() << std::endl;
}

void test_update()
{
    sqlite_adapter a;
    update_model updater(&a, "users");
    updater("hello", "hello")("world", "world").where(col("id") == 2);
    update_model au(updater);
    au("helloworld", "helloworld");

    std::cout << updater.str() << std::endl;
    std::cout << au.str() << std::endl;
}

void test_delete()
{
    sqlite_adapter a;
    delete_model deleter(&a, "users");
    // delete_model deleter(a);
    deleter.where(col("id") == 1).or_where(col("name")["%hello"]);
    std::cout << deleter.str() << std::endl;
}

void test_insert()
{
    sqlite_adapter a;
    insert_model insert(&a, "users");
    insert
        ("id", 1)
        ("name", "hello")
    .next_row()
        ("id", 2)
        ("name", "world");

    std::cout << insert.str() << std::endl;
}

int main() 
{
    test_select_where();
    return 0;
}
