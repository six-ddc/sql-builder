#define BOOST_TEST_MODULE sqltest

#include <iostream>
#include <sstream>

#include <boost/test/unit_test.hpp>
#include "sql.h"

BOOST_AUTO_TEST_SUITE(sqltest)

BOOST_AUTO_TEST_CASE(insert)
{
    std::vector<int> a = {1, 2, 3};

    InsertModel i;
    i.insert("id", 32312)
        .insert("name", std::string("six"))
        .insert("address", "beijing")
        .insert("time", time(NULL))
        .insert("age", (int64_t)15323892489203488)
        .into("info");
    std::cout<<i.str()<<std::endl;

    SelectModel s;
    s.select("aaa", "bbb", "ccc")
        .from("table")
        .where(column("aaa") >= 0 and column("bbb").is_null() or column("ddd") != 1 and column("eee").in(a));
    std::cout<<s<<std::endl;

    UpdateModel u;
    u.update("table")
        .set("id", 2)
        .set("name", "six")
        .where(column("aaa") >= 0 and column("bbb").is_null() or column("ddd") != 1 and column("eee").in(a));
    std::cout<<u<<std::endl;

    DeleteModel d;
    d._delete()
        .from("table")
        .where(column("aaa") >= 0 and column("bbb").is_null() or column("ddd") != 1 and column("eee").in(a));
    std::cout<<d<<std::endl;

    SqlModel m;
    m["id"] = 1;
    m["name"] = "six-ddc";

    u.reset();
    u.update("table")
        .set(m);
    std::cout<<u<<std::endl;

    i.reset();
    i.insert(m)
        .into("table");
    std::cout<<i<<std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
