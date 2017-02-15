# HapcanButton
This submodule can perform the same basic button operations as original [Hapcan Button module](http://hapcan.com/devices/universal/univ_3/univ_3-1-3-x/index.htm).

Note that submodule create single digital input (button). In case of multiple digital inputs You should declare multiple HapcanButton object instances.

## Library dependencies
- [Hapcanuino](https://github.com/Onixarts/Hapcanuino) - this submodule is dedicated for Hapcanuino project
- [Onixarts_IO](https://github.com/Onixarts/Onixarts_IO) - delivers basic button input capabilities


## Features

- Input debouncing (20ms default)
- 7 standard Hapcan's button events (pressed, released, released before 400ms, held for 400ms, released after 400ms, held for 4s, released after 4s)
- 2 extra button events (held for 1s, released after 1s)
- Non-blocking time measure and debouncing
- No timer used (using millis())
- Configurable active input level (LOW, HIGH)
- Configurable built-in pull-up resistor usage


## Important notes

"Release after 400ms" event is called if button is released between 400ms and 4s. Hapcanuino has 2 extra events, fired on 1s hold, and released after 1s. To keep comaptibility 
with original Hapcan module "Release after 400ms" event is also fired with "Release after 1s" event.

Every notification can be disabled. All notifications are enabled by default. In order to keep bus traffic in a low level **You should enable only required events**. It is good practice
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
Held400ms|0xFE|standard
Held4s|0xFD|standard
ReleasedBefore400ms|0xFC|standard
ReleasedAfter400ms|0xFB|standard
ReleasedAfter4s|0xFA|standard
Held1s|0xF1|extended
ReleasedAfter1s|0xF0|extended

### Status request

TODO

## Submodule control

Submodule has no control instructions.

## Settings

`HapcanButton` submodule handle many events, which can (and should) be disabled if there is no need to send it to the CAN bus.
To set up button events call `SetStandardEventNotifications(config)` and `SetExtendedEventNotifications(config)` methods in Hapcanuino's `OnReadEEPROMConfig()` method.
You can hardcode event settings by using flags defined in `Hapcan::SubModule::HapcanButton::EventNotification` namespace.

```C++
virtual void OnReadEEPROMConfig()
{
    byte config = 0;

    GetConfigByte(Hapcan::ConfigBank::NodeConfig, 0, config);
    button1.SetStandardEventNotifications(config);

    GetConfigByte(Hapcan::ConfigBank::NodeConfig, 1, config);
    button1.SetExtendedEventNotifications(config);

    // hardcoded settings for just pressed and released events
    // button1.SetStandardEventNotifications(Hapcan::SubModule::HapcanButton::EventNotification::Pressed | Hapcan::SubModule::HapcanButton::EventNotification::Released);
    // button1.SetExtendedEventNotifications(0x00);
}
```

One config byte is for standard Hapcan's events, and second byte is for extended events. Bits are set as follows:

<7>|<6>|<5>|<4>|<3>|<2>|<1>|<0>
---|---|---|---|---|---|---|---
-|Released|ReleasedAfter4s|Held4s|ReleasedAfter400ms|Held400ms|ReleasedBefore400ms|Pressed

<7>|<6>|<5>|<4>|<3>|<2>|<1>|<0>
---|---|---|---|---|---|---|---
-|-|-|-|-|-|ReleasedAfter1s|Held1s
