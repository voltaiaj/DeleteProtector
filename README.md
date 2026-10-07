# DeleteProtector

Project for learning and exploration of File System Mini-Filters. 

### Goal
Protect user-defined extensions from being deleted. User is able to set and clear the protected extensions list. 

### Architecture
- Client application
- Kernel Driver

### Design
- Ability to Load/Unload Driver using sc.exe
  - Requires setting required mini-filter registry values
