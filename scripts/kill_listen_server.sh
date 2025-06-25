#!/bin/bash

pid=$(sudo ss -tunlp | grep 9981 | grep -oP 'pid=\K\d+')

if [ -z "$pid" ]; then
  echo "No process found listening on port 9981."
  exit 0
fi

echo "$pid"
sudo kill -9 "$pid"