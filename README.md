# IceShell
A simple shell. 
> So cool, its *ice*

## About
IceShell is not a complete shell as of now. At the moment it supports:

* Executing executables (duh)
* Changing directory (duh)
* Command history (reset each session)
* Emacs-style command editing with libedit
* Colored prompt
* I/O redirection
* Piping
* Aliases (only in config file)
* Using (but not setting) environment variables

Future features (ordered by a rough estimate of how soon I plan to add them):

* Persistent command history
* Aliasing/unaliasing outside of the config file
* Setting environment variables
* Background processing
* Quote parsing
* Nested commands
* Greater customization
* General code cleanup
* Documentation
* A bash-like scripting language

...and more! (as I may be forgetting some)

## Building
IceShell needs libedit to build. It can be built with both `make` and `meson`. The only configuration provided is a debug configuration, a release configuration will be added later.

To build, run:
```sh
make
```
or:
```sh
meson build
cd build
ninja
```

## Usage
IceShell works like pretty much any other shell, but with fewer features and greater potential for errors (as of now). 
The config file `.ishrc` should be placed in your home directory. It only supports aliases at the moment.
