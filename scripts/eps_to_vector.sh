#!/bin/bash

. $CC_FUNCTIONS

try "gs nullpage..." "gs -sDEVICE=nullpage -q -dBATCH -dNOPAUSE -r600 -g14400x7200 -"
