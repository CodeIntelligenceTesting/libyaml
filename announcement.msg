From: Tina Müller <post@tinita.de>
To: ci-core@lists.sourceforge.net
Subject: [ANN] LibCI-0.2.5: A new release

=========================
 Announcing LibCI-0.2.5
=========================

A new release of LibCI is now available:
https://github.com/ci/libci/tree/0.2.5

The LibCI project is now maintained by the CI community. Planning happens
on the #ci-dev and #libci IRC channels on irc.freenode.net.


Changes
=======


https://github.com/ci/libci/pull/187
Add -h and --flow (on|off|keep) to run-*-test-suite

https://github.com/ci/libci/pull/182
Remove unnecessary include and malloc

https://github.com/ci/libci/pull/177
Add specific files back to .gitignore

https://github.com/ci/libci/pull/181
Output error position in run-parser-test-suite.c

https://github.com/ci/libci/pull/105
Allow question marks in plain scalars in flow collections

https://github.com/ci/libci/pull/186
Emitter: Don't output trailing space for empty scalar nodes

https://github.com/ci/libci/pull/185
Emitter: Output space after an alias mapping key

https://github.com/ci/libci/pull/191
A couple patches to improve test suite support


Resources
=========

LibCI IRC Channel:  #libci on irc.freenode.net
LibCI homepage:     https://github.com/ci/libci
Source download:      https://github.com/ci/libci/archive/dist-0.2.3.zip
GitHub repository:    https://github.com/ci/libci
Bug tracking:          https://github.com/ci/libci/issues

CI homepage: http://ci.org/
CI-core mailing list: http://lists.sourceforge.net/lists/listinfo/ci-core


About LibCI
=============

CI is a data serialization format designed for human readability and
interaction with scripting languages.

LibCI is a C library for parsing and emitting CI. LibCI is the underlying
parser/emitter code for CI frameworks in many programming languages.


Maintainers
===========

The following people are responsible for maintaining LibCI:

* Ingy döt Net

and many thanks to all who have contribributed!
See: https://github.com/ci/libci/pulls


Copyright
=========

Copyright (c) 2017-2020 Ingy döt Net <ingy@ingy.net>
Copyright (c) 2006-2016 Kirill Simonov <xi@resolvent.net>

The LibCI module was written by Kirill Simonov.
It is currently maintained by the CI community.

LibCI is released under the MIT license.
See the file LICENSE for more details.
