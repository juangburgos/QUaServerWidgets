# QUaServerWidgets

Generic widgets to view and edit instances of `QUaServer` types.

## Build

```bash
# windows
qmake "CONFIG+=ua_alarms_conditions ua_historizing ua_encryption" -r -tp vc examples.pro
# linux
qmake "CONFIG+=ua_alarms_conditions ua_historizing ua_encryption" -r examples.pro
```