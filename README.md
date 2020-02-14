# lolz [![Build Status](https://travis-ci.org/geekprojects/lolz.svg?branch=master)](https://travis-ci.org/geekprojects/lolz)

Oh hai! I'm in ur logz, findin ur bugz!

## Installation

### Requirements
  * [libgeek](https://github.com/geekprojects/libgeek)
  * [fswatch](https://github.com/emcrisostomo/fswatch)
  * [yaml-cpp](https://github.com/jbeder/yaml-cpp)
  * sqlite3

### Building
  * mkdir build
  * cd build
  * cmake ..
  * make

## Usage

### Configuration

Create a lolz.yml file, by default $HOME/.lolz.yml will be used. e.g.:
```YAML
database: /home/me/stuff/lolz.db

directories:
    - path: home/me/projects
      include:
      - *.log
```

### Ceiling Cat - The Daemon
```bash
lolzcat --config=/path/to/config
```

### lolzfindr
```bash
./lolzfindr ERROR
```

#### Linux
```bash
lolzfindr --tail --exec='notify-send "{line}"' ERROR
```

#### MacOS
Using [terminal-notifier](https://github.com/julienXX/terminal-notifier) to provide a notification that links to the log file:
````bash
lolzfindr --tail --exec='terminal-notifier -message "{line}" -title "Oh noes!" -open "file://{file}"' ERROR
````

## License

lolz is available under the GPL3 license.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the [GNU General Public License] (COPYING) for more details.


