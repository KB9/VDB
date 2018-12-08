#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "vdb.hpp"

std::unique_ptr<StepMessage> stepInto(VDB& vdb, const std::string& source_file, unsigned int source_line)
{
	std::shared_ptr<DebugEngine> engine = vdb.getDebugEngine();

	// Set the breakpoint
	engine->addBreakpoint(source_file.c_str(), source_line);

	// Run the target process until it encounters the breakpoint
	engine->run();
	std::unique_ptr<DebugMessage> msg = nullptr;
	while ((msg = engine->tryPoll()) == nullptr) {}

	// Step into the function
	engine->stepInto();

	// Await the notification message that the step has been completed
	std::unique_ptr<DebugMessage> ret_val = nullptr;
	while ((ret_val = engine->tryPoll()) == nullptr) {}

	StepMessage* step_msg = dynamic_cast<StepMessage*>(ret_val.get());
	if (step_msg != nullptr)
	{
		auto step = std::make_unique<StepMessage>();
		step->line_number = step_msg->line_number;
		step->file_name = step_msg->file_name;
		return std::move(step);
	}
	else
	{
		return nullptr;
	}
}

TEST_CASE("Source step into")
{
	VDB vdb;
	vdb.init("data/functions");

	const std::string source_file = std::string(VDB_TEST_DIR) + "/data/functions.cpp";

	SECTION("Source line without function call steps to next consecutive line")
	{
		auto msg = stepInto(vdb, source_file, 23);
		REQUIRE(msg != nullptr);
		REQUIRE(msg->line_number == 24);
		REQUIRE(msg->file_name == source_file);
	}

	SECTION("Source line with function call steps into function")
	{
		auto msg = stepInto(vdb, source_file, 24);
		REQUIRE(msg != nullptr);
		REQUIRE(msg->line_number == 2);
		REQUIRE(msg->file_name == source_file);
	}

	SECTION("Source line recursively calling its parent function")
	{
		auto msg = stepInto(vdb, source_file, 17);
		REQUIRE(msg != nullptr);
		REQUIRE(msg->line_number == 15);
		REQUIRE(msg->file_name == source_file);
	}
}