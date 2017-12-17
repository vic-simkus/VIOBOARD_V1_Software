#!/bin/sh

astyle \
	-n \
	-R \
	--indent-col1-comments  \
	--break-blocks  \
	--indent-preproc-block  \
	--indent-namespaces \
	--indent-switches  \
	--indent-preproc-define \
	--attach-inlines \
	--indent=force-tab \
	--style=allman  \
	--delete-empty-lines  \
	--unpad-paren \
	--align-pointer=type \
	-Xv \
	--pad-oper \
	--pad-paren-in \
	-j \
	'*.cpp' '*.hpp'


