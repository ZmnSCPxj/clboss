
This module creates channels.

It reacts to `Boss::Msg::RequestChannelCreation`, then queries a
`Boss::Mod::ChannelCandidateInvestigator::Main` for candidates,
then figures out some kind of divison of available funds to those
candidates.

- `Dowser` - given a single candidate, attempts to figure out some
  recommended amount to put in a channel for this.
  This object is created at runtime for each recommended candidate.
