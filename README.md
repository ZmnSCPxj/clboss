CLBOSS The C-Lightning Node Manager
===================================

CLBOSS is an automated manager for C-Lightning forwarding nodes.

Heuristics
----------

### Initial Peer Discovery

At initial setup of a node, it has no knowledge of the network, no
peers, no channels, nothing to get information about how best to
connect ourselves to the network.

At initial peer discovery we look at lseed.bitcoinstats.com via
DNS `SRV` requests.
If that fails, we fall back to a hardcoded list of nodes.

These initial peers are only used to get access to the network,
and are not otherwise privileged.

### Initial Channel Proposal

At setup a node has no channels yet, and no decent data to determine
how best to connect to the network.

However, if we are connected to at least one node, the C-lightning
node should start downloading the network topology.
Once there are some minimum number of known nodes, we analyze each
node and count the number of channels they have.
We perform a reservoir sampling with the number of channels as score,
selecting one node at random.

We then randomly select a *peer* of that selected high-connectivity
node, making the selected node the "patron" and the peer of the
high-connectivity node the "proposal".
The "proposal" is what we will actually make a channel to.

The logic is that high-connectivity nodes are already popular and
already have connections, so adding more to them is superfluous, but
we need to have easy access to the network as well.

### Proposal Investigation

We maintain a set of proposal nodes and their patrons.

While maintaining this, we connect to the proposal nodes.
Every 5 minutes, we `connect` (which should be idempotent if we are
alrady connected) and `ping` them.
If both commands succeed, we increment its investigation score.
If either command fails, we decrement its investigation score.
If the investigation store becomes sufficiently negative, we blacklist
the node for 1 week and drop it from investigation.

When we have onchain funds that we decide we want to put into channels,
we sort the proposed nodes according to their investigation score.
The proposed nodes with highest score are the ones we prefer to make
channels to.

### Channel Size Heuristic

Our logic is that the "patron" of a proposed node is the reason why
we want to connect to the proposed node.

So, we estimate the capacity of the proposed node to deliver funds
to the patron.
We do this by performing `getroute` from the proposed to the patron.

We keep a running sum, and an initial exclusion set (that initially
contains only our node).
We `getroute` a 1-millisatoshi payment, and find the lowest-capacity
channel of that route, subtract -0.5% (the default `maxfeepercent`),
as the route capacity.
We divide the route capacity by (2 + length) where length is the
number of hops in the route, and add the result to the running sum.
Then we add the first hop node (or if the route is single-hop, the
the short-channel-id with the patron) to the exclusion set, and
`getroute` again, repeating this loop until `getroute` fails.

This running sum amount serves as the recommended maximum size of
the channel we build to the proposal node.

### Channel Construction

Channel construction is triggered when we notice we have onchain
funds that have been confirmed for say 3 blocks.

If the node has no active or awaiting-lockin channels at all, we get
2 initial proposal nodes and find the recommended maximum size of
both.
If the onchain amount we have is less than the maximum size of the
larger one, we just split the onchain amount in two and make channels
to both.

If we already have at least one active/awaiting-lockin channel, we
just pick one initial proposal node.

If there are no proposal nodes left, we trigger channel proposal
heuristics to get more proposals.

If there are remaining onchain funds beyond the recommended maximum
size, we keep getting more nodes from the proposal list until we
run out of onchain funds.
Then we `multifundchannel` to all those nodes with the selected
amounts, with a `best_effort` flag.

Channel construction is a liability.
We must record the onchain fees paid to create the channels.

### Node-level Balance

If there are at least two active channels, and if total incoming
capacity on active channels is less than half the total outgoing
capacity, we trigger node-level rebalancing.

This is done by performing offchain-to-onchain swaps, sometimes
variously called "reverse submarine swaps" or "loop out".
We can support a number of services, such as boltz.exchange, to
swap with.
Every module that enables such support can query the service to
get a quotation on such a swap.
We prefer quotations with smaller losses, but we do this selection
via a reservoir sampling so that more expensive services still have
a (reduced) chance of being selected.

We swap out until we have equal incoming and outgoing capacity.
Specifically, we swap out (outgoing - incoming) / 2.

Once we have a target amount to swap out, we batch it in tranches
of 20mBTC.
Then we pay the needed invoices to the swap service, getting the
funds as onchain funds, which should trigger channel construction
heuristic.
The losses involved in the offchain-to-onchain swap, plus the channel
construction heuristics, should lead to a state where the node
incoming is slightly larger than half the node outgoing capacity.

Purchasing incoming capacity is an expense and we should record it
in our liabilities.

### Market Extension Proposal

Periodically we look at the direct peer which has been the most
lucrative, i.e. we have earned a lot from forwards *to* this peer.

We then select a peer of that peer that we do not have a direct
channel to.
That new peer is a proposed node, and its patron is the lucrative
peer.

If we are lucky, we selected the node that is the cause of most
of our earnings from the patron, meaning we successfully undercut
the patron and get its fee earnings.

If we are unlucky, we can still use the new channel to that node
to rebalance the channel with the patron, and since it is just one
hop away from the patron, rebalancing from that channel to the
high-income channel is relatively cheap.

### New Market Proposal

Periodically we perform a Dijkstra algorithm starting on our local
node.

This Dijkstra results in a shortest-path tree.
The leaves on that tree are those nodes which are distant from
our node, and therefore it is unlikely that we will route to or
from those nodes.

We select the node with the highest cost to transfer funds from
our own node, as the proposal node.
This "find highest" skips nodes that are direct peers of our
local node, in the unlikely event that a direct peer is somehow
on a leaf of the shortest-path tree.
We select its parent in that tree as the patron.

Since those nodes are distant, it increases the probability that
we are able to undercut other nodes by forming a channel to those
nodes.

As the number of channels we have increases, we lower the rate
at which we run this heuristic.

### Active Channel Rebalance

If a channel has more than 90% of the funds owned by us, we
should consider rebalancing it to transfer funds over to another
channel we have.

We thus seek to balance by moving about 25% of the funds to
another channel we control, if there is one available that is not
a dead-end.

Rebalances are liabilities.

### JIT Rebalance

When we receive an `htlc_accepted` hook, we check if the outgoing
channel:

* Has sufficient funds to send the HTLC plus fees.
* Has sufficient free HTLCs sample.

If the channel has insufficient funds, we check if the other side
has enough funds to receive the amount plus fees.
This implies that it would be a good idea to rebalance.
If so, we then seek to get funds from channels with higher outgoing
capacity.
We delay the `htlc_accepted` response for up to 5 seconds, checking
the outgoing channel every quarter-second so we can release it "early"
if it turns out we can now send that capacity.
We also start the rebalance flow to move funds to this channel.

We seek to fill up the outgoing channel up to 80% of its total capacity,
or the minimum needed to send the HTLC, whichever is larger.
(fewer rebalances are better)

If the channel has insufficient number of HTLCs, we delay the response
for up to 1 second, again re-checking every quarter-second to release
it early, in the hope that one of the HTLCs will get resolved in the
meantime.

