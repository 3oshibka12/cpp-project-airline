#!/bin/bash
sudo apt-get update
sudo apt-get install -y g++ make

cd /workspaces/$(basename $GITHUB_REPOSITORY)
make build

echo "Для запуска введите: make run"