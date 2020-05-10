# Vic's IO Board software

## Why
A little while ago I decided that rather than buying an off the shelf home thermostat I should build my own.  Narrative with more details is here: https://www.simkus.com/projects/the-home-automation-system

## What
This repository consists of all the "high level" software.  

The hardware design is here: https://github.com/vic-simkus/VIOBOARD_V1_Schematics
Narrative for the hardware design process is here: https://www.simkus.com/projects/vics-io-board/

Firmware can be found here: https://github.com/vic-simkus/VIOBOARD_V1_Firmware

## WHat's here
In this repository are the following sub-projects:

*	HVAC_LIB -- Library containing the core functionality.  All other sub-projects here are based on it.
*	HMI_DATA_LOGGER -- Data logger that interfaces with LOGIC_CORE and writes out status information to files or a PostgreSQL database on a periodic basis.
*	HMI_SHIM -- Testing/reference implementation of the client library stuffs.
*	qtHMI_SHIM -- A GUI for debugging the LOGIC_CORE.  Also acts as a reference implementation and test bed for the communications library.
*	LOGIC_CORE -- The main logic/control component.  As with the rest of the above the core functionality is in HVAC_LIB and LOGIC_CORE is essentially a user interface skin.

For more details about the above see my website.  Relevant links:

https://www.simkus.com

https://www.simkus.com/projects/the-home-automation-system

https://www.simkus.com/projects/vics-io-board/