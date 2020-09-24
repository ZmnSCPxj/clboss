#ifndef BOLTZ_SERVICEFACTORY_HPP
#define BOLTZ_SERVICEFACTORY_HPP

#include<functional>
#include<memory>

namespace Bitcoin { class Tx; }
namespace Boltz { class EnvIF; }
namespace Boltz { class Service; }
namespace Ev { template<typename a> class Io; }
namespace Ev { class ThreadPool; }
namespace Secp256k1 { class SignerIF; }
namespace Sqlite3 { class Db; }

namespace Boltz {

/** class Boltz::ServiceFactory
 *
 * @brief Creates services.
 *
 * @desc Each service needs to store its data into a
 * database table.
 * A single database table is used for all services
 * created from this factory.
 * The service API endpoint is used to differentiate
 * each service, and is stored in the table.
 */
class ServiceFactory {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	ServiceFactory() =delete;

	ServiceFactory(ServiceFactory&&);
	ServiceFactory& operator=(ServiceFactory&&);
	~ServiceFactory();

		      /* Threadpool to use.  */
	ServiceFactory( Ev::ThreadPool& threadpool
		      /* Database to store data in.  */
		      , Sqlite3::Db db
		      /* Signer for reverse submarine swap claims.  */
		      , Secp256k1::SignerIF& signer
		      /* Environment we are running in.  */
		      , Boltz::EnvIF& env
		      /* SOCKS5 proxy to use.  Empty string means no proxy.  */
		      , std::string proxy = ""
		      );

	/** Boltz::ServiceFactory::create_service
	 *
	 * @brief creates a service from a particular
	 * API endpoint.
	 */
	Ev::Io<std::unique_ptr<Service>>
	create_service(std::string const& api_endpoint);
};

}

#endif /* !defined(BOLTZ_SERVICEFACTORY_HPP) */
