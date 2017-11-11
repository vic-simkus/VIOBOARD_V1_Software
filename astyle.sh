#!/bin/sh

astyle -n  --indent-col1-comments  --break-blocks  --indent-preproc-block  --indent-namespaces --indent-switches  --attach-inlines --indent=tab --style=allman  --delete-empty-lines  --unpad-paren --align-pointer=type -R '*.cpp' '*.hpp'


