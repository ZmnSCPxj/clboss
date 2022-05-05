#undef NDEBUG
#include"Jsmn/Object.hpp"
#include"Jsmn/Parser.hpp"
#include<assert.h>

int main() {
	Jsmn::Parser p;

	/* Should work across feeds.  */
	{
		auto res = p.feed("[");
		assert(res.size() == 0);
	}
	{
		auto res = p.feed("]");
		assert(res.size() == 1);
		assert(res[0].is_array());
		assert(res[0].size() == 0);
	}

	/* Whitespace trickery.  */
	{
		auto res = p.feed(R"JSON(
			"""" """ "
		)JSON");
		assert(res.size() == 4);
		assert(res[0].is_string());
		assert(res[1].is_string());
		assert(res[2].is_string());
		assert(res[3].is_string());
		assert(std::string(res[0]) == "");
		assert(std::string(res[1]) == "");
		assert(std::string(res[2]) == "");
		assert(std::string(res[3]) == " ");
	}

	/* Really nasty whitespace trickery.  */
	{
		auto res = p.feed("\"\"");
		assert(res.size() == 1);
		assert(res[0].is_string());
		assert(std::string(res[0]) == "");
	}
	{
		auto res = p.feed(" ");
		assert(res.size() == 0);
	}
	{
		auto res = p.feed("9");
		assert(res.size() == 0);
	}
	{
		auto res = p.feed(" 42");
		assert(res.size() == 1);
		assert(res[0].is_number());
	}
	{
		auto res = p.feed(".1\n");
		assert(res.size() == 1);
		assert(res[0].is_number());
	}

	/* Crazy nested object.  */
	{
		auto res = p.feed(" { ");
		assert(res.size() == 0);
	}
	{
		auto res = p.feed(" \"foo\": { ");
		assert(res.size() == 0);
	}
	{
		auto res = p.feed(" \"foo\": { ");
		assert(res.size() == 0);
	}
	{
		auto res = p.feed("}");
		assert(res.size() == 0);
	}
	{
		auto res = p.feed("}}\n\n\n");
		assert(res.size() == 1);
		assert(res[0].is_object());
		assert(res[0].has("foo"));
		assert(res[0]["foo"].is_object());
		assert(res[0]["foo"].has("foo"));
		assert(res[0]["foo"]["foo"].is_object());
		assert(res[0]["foo"]["foo"].keys().size() == 0);
	}

	/* Simple array.  */
	{
		auto res = p.feed("[1]");
		assert(res.size() == 1);
		assert(res[0].is_array());
		assert(res[0].size() == 1);
		assert(res[0][0].is_number());
	}

	/* JSON text inside JSON string.  */
	{
		auto res = p.feed(R"JSON(
			" { \"foo\": \"bar quux\")JSON");
		assert(res.size() == 0);
	}
	{
		auto res = p.feed("}\"  ");
		assert(res.size() == 1);
		assert(res[0].is_string());
		assert(std::string(res[0]) == " { \"foo\": \"bar quux\"}");
	}

	/* Mismatched JSON text inside JSON string inside object.  */
	{
		auto res = p.feed(R"JSON(
			{
				"array": [
					"}\\[[\""
				]
			}
		)JSON");
		assert(res.size() == 1);
		assert(res[0].is_object());
		assert(res[0].has("array"));
		assert(res[0]["array"].is_array());
		assert(res[0]["array"].size() == 1);
		assert(res[0]["array"][0].is_string());
		assert(std::string(res[0]["array"][0]) == "}\\[[\"");
	}

	return 0;
}
