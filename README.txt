I replaced a complex tree of 'if' statements with a state-machine that has clearly defined transitions.

At this moment, the code in this branch is functionally identical to the code in the main branch.
(There are some trivial differences, but they're either irrelevant or they'll be ported to the main branch.)

The approach used here is more readable and easier to maintain.
The drawback is that it consumes a lot more clock cycles.

Might be able to reduce the CPU usage quite a bit - see the giant TODO comment in UpdateState.