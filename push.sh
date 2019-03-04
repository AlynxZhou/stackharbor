#!/bin/bash

hikaru clean --debug
hikaru build --debug
git add .
git commit -m "Updated site."
git push origin master

