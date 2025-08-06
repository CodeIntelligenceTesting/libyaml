#!/usr/bin/env bash

set -ex

cp -r /output/libci.git /tmp/
cd /tmp/libci.git
./bootstrap
./configure
make dist

# get the tarball filename
tarballs=(ci-*.tar.gz)
tarball=${tarballs[0]:?}
dirname=${tarball/.tar.gz/}

# Copy to output dir
cp "$tarball" /output

# Create zip archive
cd /tmp
cp "/output/$tarball" .
tar xvf "$tarball"
zip -r "/output/$dirname.zip" "$dirname"
