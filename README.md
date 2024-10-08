# Visualizer

A visualization tool for ATLAS ITK pixel detectors.

## Installation

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

## Structure

- `core`: core functionality
- `util`: utility functions
- `datasets`: dataset loaders and adapters
