# Flecs Rewind Debugger

The Flecs Rewind Debugger integration is a passive, opt-in historical inspector.
Scrubbing changes only the state displayed by the debugger; it does not restore
or mutate the live Flecs world.

## Enable capture

1. Enable **Flecs Per-Project Editor Settings > Rewind Debugger > Enable Flecs
   Rewind Debugger**.
2. Add `FFlecsRewindDebuggerTag` to each entity that should be inspected:

```cpp
#include "Debugging/FlecsRewindDebuggerTag.h"

Entity.Add<FFlecsRewindDebuggerTag>();
```

3. Start a new Rewind Debugger recording.

The setting is read when a recording begins. Changing it during an active
recording has no effect until the next recording.

Capture occurs only while the setting is enabled, a Rewind Debugger recording
is active, and the entity owns the tag. Untagged entities are not serialized.

## Recorded data

Each changed full snapshot contains the complete 64-bit Flecs entity ID,
generation, historical name and path, and owned archetype membership. Tags,
pairs, and component names remain visible even when a component does not have a
Flecs metadata serializer; in that case its value is shown as unavailable.

The **Flecs Entities** track is attached to each recorded `UFlecsWorld`. This
keeps PIE server and client worlds separate. Child tracks show entity lifetime
spans and state-change markers, and their details views resolve state at the
current scrub time.
