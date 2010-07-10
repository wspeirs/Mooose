#!/bin/bash
kill `ps aux | grep bochs | awk '{ print $2 }' | head -n 1`
