
The `Boss::Mod::BoltzSwapper` provides offchain-to-onchain
swap services.

- `ServiceModule` - a thin wrapper that converts from Boss
  messages to calls to a `Boltz::Service`.
- `ServiceCreator` - at `Boss::Msg::Init`, constructs the
  various `ServiceModule`s according to hardcoded known
  Boltz instances.
- `Env` - an implementation of `Boltz::EnvIF` for Boss.
- `Main` - holds the `ServiceCreator` and `Env`.
