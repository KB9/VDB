#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>

#include "vdb.hpp"

TEST_CASE("Breakpoint testing")
{
	VDB vdb;
	vdb.init("data/hello_world");

	std::shared_ptr<DebugEngine> engine = vdb.getDebugEngine();
	std::shared_ptr<BreakpointTable> table = engine->getBreakpoints();

	const std::string source_file = std::string(VDB_TEST_DIR) + "/data/hello_world.cpp";
	const unsigned int source_line = 6;

	SECTION("Check for a non-existant breakpoint")
	{
		REQUIRE(table->isBreakpoint(source_file, source_line) == false);
	}

	SECTION("Setting a breakpoint")
	{
		table->addBreakpoint(source_file.c_str(), source_line);
		REQUIRE(table->isBreakpoint(source_file, source_line) == true);
	}

	SECTION("Removing an existing breakpoint")
	{
		table->removeBreakpoint(source_file.c_str(), source_line);
		REQUIRE(table->isBreakpoint(source_file, source_line) == false);
	}

	SECTION("Debuggee halting at an enabled breakpoint")
	{
		// Set the breakpoint
		table->addBreakpoint(source_file.c_str(), source_line);

		// Run the tatget process until it encounters the breakpoint
		engine->run();
		std::unique_ptr<DebugMessage> msg = nullptr;
		while ((msg = engine->tryPoll()) == nullptr) {}

		// Ensure the breakpoint that has been hit matches the breakpoint that
		// was added
		BreakpointHitMessage *bph_msg = dynamic_cast<BreakpointHitMessage *>(msg.get());
		REQUIRE(bph_msg != nullptr);
		REQUIRE(bph_msg->file_name == source_file);
		REQUIRE(bph_msg->line_number == source_line);
	}
}