project('ish','c')
b_sanitize='address,leak,undefined'
buildtype='debug'
libedit = dependency('libedit')
executable('ish','main.c','strprocessing.c','parsing.c','util.c', dependencies: libedit)
