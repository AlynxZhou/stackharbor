#!/bin/bash

hikaru clean --debug
hikaru build --debug
git add --all
git commit --message "Updated site."
git push --set-upstream origin master
