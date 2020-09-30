This module checks the offchain channel fees of peers-of-peers.
It checks all the incoming directions of every channel of every
peer for their base and proportional channel feerates, and
computes the weighted median (with channel size as the weight).

The logic here is that this is a reasonable baseline for our
own feerate with that peer.

This module only performs this measuring and broadcasts the
extracted information.
Other modules should handle the actual changing of our own
feerate.

- `Surveyor` - Temporary object that measures the median
  competitor feerate for a particular node.
- `Main` - Handles events and creates `Surveyor` as needed.
