#include"Bitcoin/Tx.hpp"
#include"Bitcoin/le.hpp"
#include"Bitcoin/sighash.hpp"
#include"Ln/Amount.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/HasherStream.hpp"
#include"Sha256/fun.hpp"

namespace {

void feed_hash(std::ostream& hasher, Sha256::Hash const& hash) {
	std::uint8_t buf[32];
	hash.to_buffer(buf);
	for (auto i = std::size_t(0); i < sizeof(buf); ++i)
		hasher.put(buf[i]);
}

using ::Bitcoin::SighashFlags;
using ::Bitcoin::SIGHASH_ALL;
using ::Bitcoin::SIGHASH_NONE;
using ::Bitcoin::SIGHASH_SINGLE;
using ::Bitcoin::SIGHASH_ANYONECANPAY;
using ::Bitcoin::InvalidSighash;

void
sighash_core( Bitcoin::Tx const& tx
	    , SighashFlags flags
	    , std::size_t nIn
	    , Ln::Amount amount
	    , std::vector<std::uint8_t> const& scriptCode
	    , std::ostream& hasher
	    ) {

	auto loflags = flags & 0x1F;
	auto hiflags = flags & 0xE0;

	switch (loflags) {
	case SIGHASH_ALL:
	case SIGHASH_NONE:
	case SIGHASH_SINGLE:
		break;
	default:
		throw InvalidSighash("Invalid SIGHASH flag");
	}
	switch (hiflags) {
	case 0:
	case SIGHASH_ANYONECANPAY:
		break;
	default:
		throw InvalidSighash("Invalid SIGHASH flag");
	}

	if (nIn >= tx.inputs.size())
		throw InvalidSighash("nIn out of range");

	auto hashPrevouts = Sha256::Hash();
	if (!(hiflags & SIGHASH_ANYONECANPAY)) {
		Sha256::HasherStream hasher;
		for (auto const& i : tx.inputs)
			hasher << i.prevTxid
			       << Bitcoin::le(i.prevOut)
				;
		hashPrevouts = Sha256::fun(std::move(hasher).finalize());
	}

	auto hashSequence = Sha256::Hash();
	if ( !(hiflags & SIGHASH_ANYONECANPAY)
	  && (loflags != SIGHASH_SINGLE)
	  && (loflags != SIGHASH_NONE)
	   ) {
		Sha256::HasherStream hasher;
		for (auto const& i : tx.inputs)
			hasher << Bitcoin::le(i.nSequence);
		hashSequence = Sha256::fun(std::move(hasher).finalize());
	}

	auto hashOutputs = Sha256::Hash();
	if ( (loflags != SIGHASH_SINGLE)
	  && (loflags != SIGHASH_NONE)
	   ) {
		Sha256::HasherStream hasher;
		for (auto const& o : tx.outputs)
			hasher << o;
		hashOutputs = Sha256::fun(std::move(hasher).finalize());
	} else if ((loflags == SIGHASH_SINGLE) && nIn < tx.outputs.size()) {
		Sha256::HasherStream hasher;
		hasher << tx.outputs[nIn];
		hashOutputs = Sha256::fun(std::move(hasher).finalize());
	}

	/* Hashing sequence.  */
	hasher << Bitcoin::le(tx.nVersion);
	feed_hash(hasher, hashPrevouts);
	feed_hash(hasher, hashSequence);

	/* Input being signed.  */
	hasher << tx.inputs[nIn].prevTxid
	       << Bitcoin::le(tx.inputs[nIn].prevOut)
		;
	for (auto b : scriptCode)
		hasher.put(b);
	hasher << Bitcoin::le(amount)
	       << Bitcoin::le(tx.inputs[nIn].nSequence)
		;

	feed_hash(hasher, hashOutputs);
	hasher << Bitcoin::le(tx.nLockTime)
	       << Bitcoin::le(std::uint32_t(flags))
		;
}

}

namespace Bitcoin {

Sha256::Hash
sighash( Bitcoin::Tx const& tx
       , SighashFlags flags
       , std::size_t nIn
       , Ln::Amount amount
       , std::vector<std::uint8_t> const& scriptCode
       ) {
	Sha256::HasherStream hasher;
	sighash_core(tx, flags, nIn, amount, scriptCode, hasher);
	return Sha256::fun(std::move(hasher).finalize());
}

}
