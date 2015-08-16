#!/bin/bash

pushd code
tar -czf paiv_icfpc2015.tar.gz README Makefile src
popd
mv code/paiv_icfpc2015.tar.gz ./
