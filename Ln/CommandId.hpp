#ifndef LN_COMMANDID_HPP
#define LN_COMMANDID_HPP

#include"Util/Either.hpp"
#include<cstdint>
#include<memory>
#include<string>

namespace Jsmn { class Object; }

namespace Ln {

typedef Util::Either<std::uint64_t, std::string> CommandId;

std::unique_ptr<CommandId>
command_id_from_jsmn_object(Jsmn::Object const&);

}

#endif /* !defined(LN_COMMANDID_HPP) */
