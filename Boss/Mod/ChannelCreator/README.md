
This module creates channels.

It reacts to `Boss::Msg::RequestChannelCreation`, then queries a
`Boss::Mod::ChannelCandidateInvestigator::Main` for candidates,
then figures out some kind of divison of available funds to those
candidates.

- `Dowser` - given a single candidate, attempts to figure out some
  recommended amount to put in a channel for this.
  This object is created at runtime for each recommended candidate.
- `Planner` - given an amount, some proposal nodes, and how many
  channels we already have, creates a plan of how to build channels.
  This object is created at runtime when we have onchain funds we
  want to use.
- `Carpenter` - actually constructs channels, and reports channel
  creation success and failure.
- `Manager` - handles the above objects, and wrangles with most
  messages received.
- `Main` - Holds the `Carpenter` and `Manager`.
