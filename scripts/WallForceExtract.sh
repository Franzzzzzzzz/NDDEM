#!/bin/bash

ls LogWallForce-* | sort -k 1.13 -nr | xargs -n 1 sed "$1!d"

