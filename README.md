# VolumetricAmbientDemo

## Setup

### Prerequisites

* Unreal Engine 4.25
* FMOD Studio 2.01
* FMOD Plugin 2.01

### Adding The FMOD Plugin

1. Close your Unreal Editor.
1. Download the latest FMOD Plugin from [GitHub](https://github.com/fmod/ue4integration) or [Official Website](https://www.fmod.com/download).
1. Copy the `FMODStudio` folder to `{UE4ProjectFolder}/Plugins` folder.
1. Start the Unreal Editor.

## Getting Started

1. [Download](https://www.fmod.com/download) and install latest FMOD Studio
1. Create FMOD Project
1. Setup FMOD Project:
 1. _Edit -> Preferences -> Build_
 1. Set _Built banks output directory_ to `{UE4ProjectFolder}/Content/FMOD`
 1. Create some FMOD 3D event and build it
1. Open the `AmbientSoundSystem.uproject`
1. In _Project Settings_ find _FMOD Studio_ plugin settings and set _Bank Output Directory_ to `FMOD`
1. Set the created FMOD 3D Event to **BP_VolumetricEmitter**
1. ???
1. PROFIT!!!