#ifndef JSON_OUT_HPP
#define JSON_OUT_HPP

#include"Jsmn/Detail/Str.hpp"
#include"Jsmn/Object.hpp"
#include<memory>
#include<sstream>

namespace Json { class Out; }

namespace Json { namespace Detail {

namespace {
char begin_obj = '{';
char end_obj = '}';
char begin_arr = '[';
char end_arr = ']';
}

/* Pre-declare these.  */
typedef std::stringstream Content;
template<typename Up> class Array;
template<typename Up> class Detail;

/* Simple type serialization.  */
template<typename t>
struct Serializer;
template<>
struct Serializer<double> {
	static std::string serialize(double v) {
		return Jsmn::Detail::Str::from_double(v);
	}
};
template<>
struct Serializer<int> {
	static std::string serialize(int v) {
		return Jsmn::Detail::Str::from_double(v);
	}
};
template<>
struct Serializer<bool> {
	static std::string serialize(bool v) {
		return v ? "true" : "false";
	}
};
template<>
struct Serializer<std::string> {
	static std::string serialize(std::string const& v) {
		return "\"" + Jsmn::Detail::Str::to_escaped(v) + "\"";
	}
};

template<typename Up>
class Object {
private:
	Up& up;
	Content& content;
	bool started;

	void encomma() {
		if (started)
			content << ", ";
		else
			started = true;
	}

public:
	Object(Up& up_, Content& content_) : up(up_), content(content_) {
		started = false;
		content << begin_obj;
	}

	template<typename a>
	Object<Up>& field(std::string const& name, a const& value) {
		encomma();
		content << Serializer<std::string>::serialize(name)
			<< ": "
			<< Serializer<a>::serialize(value)
			;
		return *this;
	}

	/* Declared later when all types are completed.  */
	Array<Object<Up>> start_array(std::string const& field);
	Object<Object<Up>> start_object(std::string const& field);

	Up& end_object() {
		content << end_obj;
		return up;
	}
};

template<typename Up>
class Array {
private:
	Up& up;
	Content& content;
	bool started;

	void encomma() {
		if (started)
			content << ", ";
		else
			started = true;
	}

public:
	Array(Up& up_, Content& content_) : up(up_), content(content_) {
		started = false;
		content << begin_arr;
	}

	template<typename a>
	Array<Up>& entry(a const& value) {
		encomma();
		content << Serializer<a>::serialize(value);
		return *this;
	}

	/* Declared later when all types are completed.  */
	Array<Array<Up>> start_array();
	Object<Array<Up>> start_object();

	Up& end_array() {
		content << end_arr;
		return up;
	}
};

} /* namespace Detail */

class Out {
private:
	std::shared_ptr<Json::Detail::Content> content;

public:
	Out() : content(std::make_shared<Json::Detail::Content>()) { }
	explicit
	Out(Jsmn::Object js
	   ) : content(std::make_shared<Json::Detail::Content>()){
		*content << js;
	}

	std::string output() const {
		return content->str();
	}

	Json::Detail::Object<Json::Out> start_object() {
		return Json::Detail::Object<Json::Out>(*this, *content);
	}
	Json::Detail::Array<Json::Out> start_array() {
		return Json::Detail::Array<Json::Out>(*this, *content);
	}

	static
	Json::Out empty_object() {
		return Json::Out().start_object().end_object();
	}
};

namespace Detail {

/* JSON data.  */
template<>
struct Serializer<Json::Out> {
	static std::string serialize(Json::Out const& v) {
		return v.output();
	}
};
template<>
struct Serializer<Jsmn::Object> {
	static std::string serialize(Jsmn::Object const& v) {
		auto os = std::ostringstream();
		os << v;
		return os.str();
	}
};

/* Sub-objects and sub-arrays.  */
template<typename Up>
Array<Object<Up>> Object<Up>::start_array(std::string const& name) {
	encomma();
	content << Serializer<std::string>::serialize(name)
		<< ": "
		;
	return Array<Object<Up>>(*this, content);
}
template<typename Up>
Object<Object<Up>> Object<Up>::start_object(std::string const& name) {
	encomma();
	content << Serializer<std::string>::serialize(name)
		<< ": "
		;
	return Object<Object<Up>>(*this, content);
}
template<typename Up>
Array<Array<Up>> Array<Up>::start_array() {
	encomma();
	return Array<Array<Up>>(*this, content);
}
template<typename Up>
Object<Array<Up>> Array<Up>::start_object() {
	encomma();
	return Object<Array<Up>>(*this, content);
}

} /* namespace Detail */

} /* namespace Json */

#endif /* !defined(JSON_OUT_HPP) */
