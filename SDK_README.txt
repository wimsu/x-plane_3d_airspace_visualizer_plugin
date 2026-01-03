X-Plane SDK Setup

This repository includes the X-Plane SDK in the repository.

You have two SDK versions available:
- SDK/        - SDK 2.1.3 (XPLM303) - Currently used in build script
- SDK 411/    - SDK 4.1.1 (XPLM400) - Newer, supports ARM64 natively

The build script currently uses SDK/ (SDK 2.1.3) with XPLM303.

To use SDK 411 instead, update build.sh:
- Change SDK_PATH to "./SDK 411"
- Change -DXPLM303 to -DXPLM400

Required structure (already present in repository):
SDK/
├── CHeaders/
│   ├── XPLM/
│   └── Widgets/
└── Libraries/
    └── Mac/
        ├── XPLM.framework
        └── XPWidgets.framework


