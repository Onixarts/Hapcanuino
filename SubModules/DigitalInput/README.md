# DigitalInput
This submodule is a simple digital input based on button operations as original [Hapcan Button module](http://hapcan.com/devices/universal/univ_3/univ_3-1-3-x/index.htm). It can
handle just two events Pressed and Released. It is designed to connect sensors with digital output signal.
If You want full Hapcan button events support see [HapcanButton](https://github.com/Onixarts/Hapcanuino/tree/master/SubModules/HapcanButton) SubModule.

Note that submodule create single digital input. In case of multiple digital inputs You should declare multiple DigitalInput object instances.

## Library dependencies
- [Hapcanuino](https://github.com/Onixarts/Hapcanuino) - this submodule is dedicated for Hapcanuino project
- [Onixarts_IO](https://github.com/Onixarts/Onixarts_IO) - delivers basic input capabilities


## Features

- 2 standard Hapcan's button events (pressed, released)
- Configurable active input level (LOW, HIGH)
- Configurable built-in pull-up resistor usage


## Important notes

Each notification can be disabled (enabled by default). In order to keep bus traffic in a low level **You should enable only required events**. It is good practice
to store this settings in EEPROM memory, so it can be set up from external programmer.

## Communication frames

### Button message
This message is send by submodule on input events.

Frame type|Flags|Module|Group|D0|D1|D2|D3|D4|D5|D6|D7
---|---|---|---|---|---|---|---|---|---|---|---
0x302|3:2:1:0|Node|Group|0xFF|0xFF|CHANNEL|BUTTON|0xFF|0xFF|0xFF|0xFF

**0x302** - universal module frame, relay

**3:2:1:RE** - response flag, 1 = answer for status request, 0 = status info on output changed

**Node** - message sender node number

**Group** - message sender group number

**CHANNEL** - input channel. Each defined `HapcanButton` submodule should have unique channel defined in object constructor.

**BUTTON** - current button status or event

#### BUTTON status values

Standard Hapcan button statuses:

Event|Value|Type
---|---|---
Released(open)|0x00|standard
Pressed(close)|0xFF|standard

### Status request

TODO

## Submodule control

Submodule has no control instructions.

## Settings

`DigitalInput` submodule handle two events, which can (and should) be disabled if there is no need to send it to the CAN bus.
To set up button events call `SetStandardEventNotifications(config)` method in Hapcanuino's `OnReadEEPROMConfig()` method.
You can hardcode event settings by using flags defined in `Hapcan::SubModule::DigitalInput::EventNotification` namespace.

```C++
virtual void OnReadEEPROMConfig()
{
    byte config = 0;

    GetConfigByte(Hapcan::ConfigBank::NodeConfig, 0, config);
    input1.SetStandardEventNotifications(config);

    // hardcoded settings for just pressed event
    // input1.SetStandardEventNotifications(Hapcan::SubModule::DigitalInput::EventNotification::Pressed);
}
```

Config byte is as follows:

<7>|<6>|<5>|<4>|<3>|<2>|<1>|<0>
---|---|---|---|---|---|---|---
-|Released|-|-|-|-|-|Pressed

It is compatible with [HapcanButton](https://github.com/Onixarts/Hapcanuino/tree/master/SubModules/HapcanButton) standard config.