/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This file is part of osm2pgsql (https://osm2pgsql.org/).
 *
 * Copyright (C) 2006-2025 by the osm2pgsql developer community.
 * For a full list of authors see the git log.
 */

#include <catch.hpp>

#include "common-import.hpp"
#include "common-options.hpp"

namespace {

testing::db::import_t db;

char const *const CONF_FILE = "test_output_flex_schema.lua";
char const *const DATA_FILE = "liechtenstein-2013-08-03.osm.pbf";

} // anonymous namespace

TEST_CASE("config with schema should work")
{
    options_t const options = testing::opt_t().slim().flex(CONF_FILE);

    auto conn = db.db().connect();
    conn.exec("CREATE SCHEMA IF NOT EXISTS myschema;");
    init_database_capabilities(conn);

    REQUIRE_NOTHROW(db.run_file(options, DATA_FILE));

    REQUIRE(1 ==
            conn.get_count("pg_catalog.pg_namespace", "nspname = 'myschema'"));
    REQUIRE(1 ==
            conn.get_count("pg_catalog.pg_tables", "schemaname = 'myschema'"));

    REQUIRE(7103 == conn.get_count("myschema.osm2pgsql_test_line"));

    REQUIRE(1 ==
            conn.get_count("pg_catalog.pg_proc",
                           "proname = 'osm2pgsql_test_line_osm2pgsql_valid'"));

    REQUIRE(1 == conn.get_count("pg_catalog.pg_trigger"));
    REQUIRE(1 ==
            conn.get_count("pg_catalog.pg_trigger",
                           "tgname = 'osm2pgsql_test_line_osm2pgsql_valid'"));
}
