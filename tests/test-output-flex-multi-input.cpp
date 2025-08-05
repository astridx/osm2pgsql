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

char const *const CONF_FILE = "test_output_flex.lua";
char const *const POINT_TABLE = "osm2pgsql_test_point";
char const *const LINE_TABLE = "osm2pgsql_test_line";

} // anonymous namespace

TEST_CASE("with three input files")
{
    options_t options = testing::opt_t().slim().flex(CONF_FILE);

    REQUIRE_NOTHROW(
        db.run_import(options, {"n10 v1 dV x10.0 y10.0\n"
                                "n11 v1 dV x10.0 y10.2\n"
                                "w20 v1 dV Thighway=primary Nn10,n11,n12\n",
                                "n12 v1 dV x10.2 y10.2\n"
                                "w21 v1 dV Thighway=secondary Nn12,n10\n",
                                "n13 v1 dV x11.0 y11.0 Tamenity=postbox\n"}));

    auto conn = db.db().connect();

    CHECK(1 == conn.get_count(POINT_TABLE));
    CHECK(2 == conn.get_count(LINE_TABLE));
    CHECK(1 == conn.get_count(LINE_TABLE, "tags->'highway' = 'primary'"));
    CHECK(1 == conn.get_count(LINE_TABLE, "tags->'highway' = 'secondary'"));
    CHECK(1 == conn.get_count(LINE_TABLE, "ST_NumPoints(geom) = 3"));
    CHECK(1 == conn.get_count(LINE_TABLE, "ST_NumPoints(geom) = 2"));

    options.append = true;

    REQUIRE_NOTHROW(db.run_import(options, "n10 v2 dV x11.0 y11.0\n"));

    CHECK(1 == conn.get_count(POINT_TABLE));
    CHECK(2 == conn.get_count(LINE_TABLE));
    CHECK(1 == conn.get_count(LINE_TABLE, "ST_NumPoints(geom) = 3"));
    CHECK(1 == conn.get_count(LINE_TABLE, "ST_NumPoints(geom) = 2"));
}

TEST_CASE("should use newest version of any object")
{
    options_t const options = testing::opt_t().slim().flex(CONF_FILE);

    REQUIRE_NOTHROW(
        db.run_import(options, {"n10 v1 dV x10.0 y10.0 Ta=10.1\n"
                                "n11 v1 dV x10.1 y10.1 Ta=11.1\n"
                                "n12 v1 dV x10.2 y10.2 Ta=12.1\n",
                                "n13 v2 dV x10.3 y10.3 Ta=13.2\n",
                                "n10 v1 dV x10.0 y10.0 Ta=10.1\n"
                                "n11 v2 dV x10.1 y10.2 Ta=11.2\n"
                                "n13 v1 dV x10.3 y10.3 Ta=13.1\n"}));

    auto conn = db.db().connect();

    CHECK(4 == conn.get_count(POINT_TABLE));
    CHECK(1 == conn.get_count(POINT_TABLE, "tags->'a' = '10.1'")); // both the same
    CHECK(1 == conn.get_count(POINT_TABLE, "tags->'a' = '11.2'"));
    CHECK(1 == conn.get_count(POINT_TABLE, "tags->'a' = '12.1'")); // only one
    CHECK(1 == conn.get_count(POINT_TABLE, "tags->'a' = '13.2'"));
}

