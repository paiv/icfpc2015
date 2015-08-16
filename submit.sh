#!/bin/bash

FILE=${1:-"-"}

curl --netrc-file ".netrc" -H "Content-Type: application/json" \
  -d "@$FILE" \
  https://davar.icfpcontest.org/teams/13/solutions
