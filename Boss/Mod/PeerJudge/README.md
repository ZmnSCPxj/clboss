This module handles peer judgment and whether to close on a peer or not.

Specifically, the algorithm implemented in `Boss::Mod::PeerJudge::Algo`
goes this way:

* First, we determine a "reasonable average expected rate of earnings".
  * Average: Specifically, we use a weighted median.
    We weight it according to channel size.
  * Reasonable: We get this data from the channels we currently have.
    We sample from a specific recent past timeframe.
  * Rate of earnings: We get all earned fees (incoming and outgoing)
    and divide by the size of the channel.
  * More specifically, we get the rate-of-earnings of all channels,
    sort the channels according to rate-of-earnings, then get the
    weighted median of the channels.
* All channels whose rate of earnings is lower than the median are
  subject to possible closure.
  These are put in a set of candidates-to-close.
* Next, we determine the cost-of-reopen.
  * The cost of reopening channels is equal to the cost of a close
    plus the cost of an open plus the cost of getting incoming
    capacity.
* For each candidate:
  * Determine the expected earnings.
    * Multiply the *median* rate-of-earnings we got above, with the
      size of the channel.
  * If the expected earnings is less than or equal to the
    cost-of-reopen, do *not* close the channel.
  * Else close the channel.
