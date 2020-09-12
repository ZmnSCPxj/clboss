
The `ChannelCandidateInvestigator` is a module which waits for
`ProposeChannelCandidates` and puts them in persistent storage.

Every random-hour, we investigate up to some number of candidates.
The gumshoe tries to connect to the candidates.

If the candidate is not reachable, we decrement its score by 1.
If the candidate is reachable, we increment its score by 1.
If the score drops too low, we stop investigating it and remove it from
our table.

New candidates start with a score of 0.

When there are few positive/zero-scored candidates, we
`SolicitChannelCandidates` to trigger other modules.

The sub-modules are:

* `Main` - contains the other sub-modules.
* `Secretary` - handles queries and updates to the database.
* `Gumshoe` - handles checking the candidates.
* `Manager` - operates the `Secretary` and `Gumshoe`, and does most of
  the bus talking.
