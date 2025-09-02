# Visualizer

A visualization tool for ATLAS ITK pixel detectors.

[![Demo Video](https://github.com/user-attachments/assets/2933b2e9-0c11-4dd9-ba3e-3b9bb5f7c0f5)](https://www.youtube.com/watch?v=K4evDcjJaR0)

## Installation

GLFW is a dependency of the software. **UNIX systems such as LINUX and FreeBSD require additional dependency installations.** 

If you are running either system, scroll to the "Dependencies for Wayland and X11" section in the following [link](https://www.glfw.org/docs/latest/compile_guide.html#compile_deps_wayland) and install the necessary dependencies for your system.

```bash
git clone https://github.com/reitori/visualizer.git
# OPTIONAL: for LBL pc installs
source /opt/rh/devtoolset-9/enable
cd visualizer
mkdir build/
cd build/
cmake3 ../
make
cd ../
```

Installed binaries will be available in `visualizer/bin`, and can be executed with e.g.
```
./bin/<executable> <args>
```

## Troubleshooting!


### GUI rendering errors
**Problem**: you cannot render GUI windows over SSH and see an error like this:

```
libGL error: No matching fbConfigs or visuals found
libGL error: failed to load driver: swrast
X Error of failed request:  GLXBadContext
  Major opcode of failed request:  152 (GLX)
  Minor opcode of failed request:  6 (X_GLXIsDirect)
  Serial number of failed request:  628
  Current serial number in output stream:  627
```

**Possible solution for MACOS:**
try (on macos - probably an analog exists for windows/linux) adding this line to your `.bashrc` or equivalent:
```
defaults write org.xquartz.X11 enable_iglx -bool YES
```

Then, quit and restart your X11 server.


## Structure

- `core`: core functionality
- `util`: utility functions
- `datasets`: dataset loaders and adapters

