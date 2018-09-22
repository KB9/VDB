#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <memory>

#include "vdb.hpp"

std::string valueOf(const std::string& variable_name, std::shared_ptr<DebugEngine> engine)
{
	std::unique_ptr<GetValueMessage> get_val = std::unique_ptr<GetValueMessage>(new GetValueMessage());
	get_val->variable_name = variable_name;
	engine->sendMessage(std::move(get_val));

	std::unique_ptr<DebugMessage> ret_val = nullptr;
	while ((ret_val = engine->tryPoll()) == nullptr) {}

	GetValueMessage *value_msg = dynamic_cast<GetValueMessage *>(ret_val.get());
	if (value_msg != nullptr)
	{
		return value_msg->value;
	}
	else
	{
		return "";
	}
}

TEST_CASE("Fundamental type value deduction")
{
	VDB vdb;
	vdb.init("data/variables");

	std::shared_ptr<DebugEngine> engine = vdb.getDebugEngine();
	std::shared_ptr<BreakpointTable> table = engine->getBreakpoints();

	// Set the breakpoint location on the return statement
	const std::string source_file = std::string(VDB_TEST_DIR) + "/data/variables.cpp";
	const unsigned int source_line = 69;

	// Set the breakpoint
	table->addBreakpoint(source_file.c_str(), source_line);

	// Run the target process until it encounters the breakpoint
	engine->run();
	std::unique_ptr<DebugMessage> msg = nullptr;
	while ((msg = engine->tryPoll()) == nullptr) {}

	SECTION("Boolean type")
	{
		REQUIRE(valueOf("b", engine) == "true");
	}

	SECTION("Character types")
	{
		REQUIRE(valueOf("s_c", engine) == "2");
		REQUIRE(valueOf("u_c", engine) == "51");
		REQUIRE(valueOf("c", engine) == "4");
		REQUIRE(valueOf("wc", engine) == "53");
	}

	SECTION("Integer types")
	{
		REQUIRE(valueOf("sh", engine) == "6");
		REQUIRE(valueOf("sh_i", engine) == "7");
		REQUIRE(valueOf("s_sh", engine) == "8");
		REQUIRE(valueOf("s_sh_i", engine) == "9");
		REQUIRE(valueOf("u_sh", engine) == "10");
		REQUIRE(valueOf("u_sh_i", engine) == "11");
		REQUIRE(valueOf("i", engine) == "12");
		REQUIRE(valueOf("s", engine) == "13");
		REQUIRE(valueOf("s_i", engine) == "14");
		REQUIRE(valueOf("u", engine) == "15");
		REQUIRE(valueOf("u_i", engine) == "16");
		REQUIRE(valueOf("l", engine) == "17");
		REQUIRE(valueOf("l_i", engine) == "18");
		REQUIRE(valueOf("s_l", engine) == "19");
		REQUIRE(valueOf("s_l_i", engine) == "20");
		REQUIRE(valueOf("u_l", engine) == "21");
		REQUIRE(valueOf("u_l_i", engine) == "22");
		REQUIRE(valueOf("l_l", engine) == "23");
		REQUIRE(valueOf("l_l_i", engine) == "24");
		REQUIRE(valueOf("s_l_l", engine) == "25");
		REQUIRE(valueOf("s_l_l_i", engine) == "26");
		REQUIRE(valueOf("u_l_l", engine) == "27");
		REQUIRE(valueOf("u_l_l_i", engine) == "28");
	}

	SECTION("Floating point types")
	{
		REQUIRE(std::stof(valueOf("f", engine)) == 29.0f);
		REQUIRE(std::stod(valueOf("d", engine)) == 30.0);
		// REQUIRE(std::stold(valueOf("l_d", engine)) == 31.0);
	}
}