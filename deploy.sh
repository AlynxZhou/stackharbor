#!/bin/bash

npx hikaru clean --debug
npx hikaru build --debug

git add --all
if [[ -n "${*}" ]]; then
    git commit --message "${*}"
else
    git commit --message "Updated site."
fi
git push --set-upstream origin master
