#ifndef BOSS_SIGNER_HPP
#define BOSS_SIGNER_HPP

#include<memory>
#include<string>

namespace Ev { template<typename a> class Io; }
namespace Secp256k1 { class Random; }
namespace Secp256k1 { class SignerIF; }
namespace Sqlite3 { class Db; }

namespace Boss {

/** class Boss::Signer
 *
 * @brief constructs a signer and checks that
 * the given database matches its public key.
 *
 * @desc maintains a 32-byte file containing the
 * private key for the CLBOSS instance.
 * If the file does not exist, a random key is
 * generated.
 *
 * This also maintains a table in the given
 * database, containing the public key.
 * If the privkey does not match the in-database
 * public key, then constructing the signer will
 * throw an exception within the `Ev::Io` system.
 */
class Signer {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Signer() =delete;

	Signer(Signer&&);
	Signer& operator=(Signer&&);
	~Signer();

	/* The filename is opened later, during
	 * the `construct` function.  */
	Signer( std::string const& privkey_filename
	      , Secp256k1::Random& random
	      , Sqlite3::Db db
	      );

	/* Call to open the filename.
	 * Note that after this function call, the
	 * factory is invalidated, i.e. you should
	 * use `std::move`.
	 */
	Ev::Io<std::unique_ptr<Secp256k1::SignerIF>>
	construct()&&;
};

}

#endif /* !defined(BOSS_SIGNER_HPP) */
