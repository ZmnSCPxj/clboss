#include"Sha256/Hash.hpp"
#include"Sha256/Hasher.hpp"
#include"Sha256/HasherStream.hpp"
#include"Util/make_unique.hpp"

namespace Sha256 { namespace Detail {

struct HasherStreamBuf::Impl {
	Sha256::Hasher hasher;
	char buf[64];
};

HasherStreamBuf::HasherStreamBuf() : pimpl(Util::make_unique<Impl>()) {
	Base::setp(pimpl->buf, pimpl->buf + sizeof(pimpl->buf));
}
HasherStreamBuf::HasherStreamBuf(HasherStreamBuf&&) =default;
HasherStreamBuf::~HasherStreamBuf() =default;

HasherStreamBuf::int_type
HasherStreamBuf::overflow(HasherStreamBuf::int_type ch) {
	/* Feed buffer to the hasher.  */
	pimpl->hasher.feed(pbase(), pptr() - pbase());
	/* Reset space.  */
	Base::setp(pimpl->buf, pimpl->buf + sizeof(pimpl->buf));
	/* If char is not eof, write it to the buffer.  */
	if (ch != std::char_traits<char>::eof()) {
		*pbase() = (char_type) ch;
		pbump(1);
	}
	return ch;
}

Hash HasherStreamBuf::finalize()&& {
	/* Flush.  */
	overflow(std::char_traits<char>::eof());
	/* Extract.  */
	return std::move(pimpl->hasher).finalize();
}
Hash HasherStreamBuf::get_hash() const {
	/* Copy hasher.  */
	auto tmp = pimpl->hasher;
	/* Feed it any data.  */
	tmp.feed(pbase(), pptr() - pbase());
	/* Finalize the temporary.  */
	return std::move(tmp).finalize();
}

HasherStreamBase::HasherStreamBase()
	: buf(Util::make_unique<HasherStreamBuf>()) { }

}}
