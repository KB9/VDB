#define CATCH_CONFIG_MAIN
#include "../catch.hpp"

#include "vdb.hpp"

TEST_CASE("VDB initialization handling")
{
	VDB vdb;

	REQUIRE(vdb.isInitialized() == false);

	SECTION("VDB initialization handles an invalid file path")
	{
		bool success = vdb.init("invalid_file_path");

		REQUIRE(success == false);
		REQUIRE(vdb.isInitialized() == false);
		REQUIRE(vdb.getDebugInfo() == nullptr);
		REQUIRE(vdb.getDebugEngine() == nullptr);
	}

	SECTION("VDB initialization initializes successfully with valid file path")
	{
		bool success = vdb.init("data/hello_world");

		REQUIRE(success == true);
		REQUIRE(vdb.isInitialized() == true);
		REQUIRE(vdb.getDebugInfo() != nullptr);
		REQUIRE(vdb.getDebugEngine() != nullptr);
	}
}