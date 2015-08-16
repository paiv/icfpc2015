#!/bin/bash

PAGES=( index.html problems.html spec.html story.html original-baa.html writeups.html )

for p in "${PAGES[@]}"
do
  curl -O "http://icfpcontest.org/$p"
done
