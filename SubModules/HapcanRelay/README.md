# HapcanRelay
This submodule can perform the same basic operations as original [Hapcan Relay module](http://hapcan.com/devices/universal/univ_3/univ_3-2-x-x.htm).

Note that submodule create single digital output. In case of multiple digital outputs (relays) You should declare multiple HapcanRelay object instances.

## Library dependencies
- [Hapcanuino](https://github.com/Onixarts/Hapcanuino) - this submodule is dedicated for Hapcanuino project
- [Onixarts_IO](https://github.com/Onixarts/Onixarts_IO) - delivers basic output capabilities



## Features
- digital output for relay control
- 3 Hapcan's control instructions (On, Off, Toggle)
- 1 additional control instruction (Blink)
- timer for instruction execution delay (1s-24h)
- up to 8 channels (8 different `HapcanRelay` submodules)


## Communication frames

### Relay message
This message is send by submodule on output change.

Frame type|Flags|Module|Group|D0|D1|D2|D3|D4|D5|D6|D7
---|---|---|---|---|---|---|---|---|---|---|---
0x302|3:2:1:0|Node|Group|0xFF|0xFF|CHANNEL|STATUS|0xFF|INSTR1|INSTR2|TIMER

**0x302** - universal module frame, relay

**3:2:1:RE** - response flag, 1 = answer for status request, 0 = status info on output changed

**Node** - message sender node number

**Group** - message sender group number

**CHANNEL** - output channel. Each defined `HapcanRelay` submodule should have unique channel defined in object constructor.

**STATUS** - current output status (after change), 0x00 = Off, 0xFF = On

**INSTR1** - currently not supported

**INSTR2** - currently not supported

**TIMER** - currently not supported

### Status request

TODO

## Submodule control

Submodule can be controlled directly by sending control frame `0x10A` from PC or indirectly by executing instructions when received message match one deffined in configuration boxes.

### Control instruction

Instruction|INSTR1|INSTR2|INSTR3|INSTR5|INSTR6|INSTR6|INSTR7|INSTR8|Description
---|---|---|---|---|---|---|---|---|---
Turn OFF output|0x00|CHANNEL|TIMER|0xXX|0xXX|0xXX|0xXX|0xXX|0xXX|It will turn OFF the output if CHANNEL match channel assigned to this submodule
Turn ON output|0x01|CHANNEL|TIMER|0xXX|0xXX|0xXX|0xXX|0xXX|0xXX|It will turn ON the output if CHANNEL match channel assigned to this submodule
Toggle output|0x02|CHANNEL|TIMER|0xXX|0xXX|0xXX|0xXX|0xXX|0xXX|It will turn Toggle the output if CHANNEL match channel assigned to this submodule
Blink|0x05|CHANNEL|TIMER|COUNT|DURATION|0xXX|0xXX|0xXX|0xXX|It will blink the output if CHANNEL match channel assigned to this submodule

**CHANNEL** is a flag type, so sending 0x03 = 0000 0011B will match channels 1 and 2. In this case You can control up to 8 channels sending one command.

**TIMER** instruction execute delay value. For 0x00 it will execute immediately.

**COUNT** defines how many times the output will blink. Minimum value is 1.

**DURATION** defines the blink duration. TODO

### Timer
TODO

### Direct control

Frame type|Flags|Module|Group|D0|D1|D2|D3|D4|D5|D6|D7
---|---|---|---|---|---|---|---|---|---|---|---
0x10A|0x0|COMP ID1|COMP ID2|INSTR1|INSTR2|Node|Group|INSTR3|INSTR4|INSTR5|INSTR6

**0x10A** - direct control frame (see Hapcan reference for details)

**COMP ID1** sender (computer) ID 1

**COMP ID2** sender (computer) ID 2

**Node** - node number of requested module

**Group** - group number of requested module

**INSTR1-6** instruction (INSTR1) with params to be executed.