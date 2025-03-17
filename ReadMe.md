## LibYAML - A C library for parsing and emitting YAML.

To build and install the library, run:

    $ ./configure
    $ make
    # make install

Required packages:

- gcc
- libtool
- make

If you checked the source code from the Git repository, run

    $ ./bootstrap
    $ ./configure
    $ make
    # make install

Required packages:

- autoconf
- libtool
- make

For more information, check the [LibYAML
homepage](https://github.com/yaml/libyaml).

Discuss LibYAML with the maintainers in IRC #libyaml irc.freenode.net.

You may also use the [YAML-Core mailing
list](http://lists.sourceforge.net/lists/listinfo/yaml-core).

Submit bug reports and feature requests to the [LibYAML bug
tracker](https://github.com/yaml/libyaml/issues/new).

This project was developed for Python Software Foundation as a part of Google
Summer of Code under the mentorship of Clark Evans.

The LibYAML module was written by Kirill Simonov <xi@resolvent.net>.
It is currently maintained by the YAML community.

LibYAML is released under the MIT license.
See the file LICENSE for more details.

## Code Intelligence Changes:
For demo purposes was a Buffer Overflow added in [api.c](src/api.c#L300).

To make use of the devcontainer setup please run ```git submodule update --init --recursive``` and have a look at the [submodule readme](.devcontainer/README.md#L1). The second README will only be available after running the command git submodule update.

To execute the spark automation, add the necessary environment variables as described [here](https://docs.code-intelligence.com/ai-test-agent/llm-setup) and run CI Fuzz either locally or in the devcontainer via the command:
```sh
cifuzz spark --target-coverage 75
```