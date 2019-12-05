TEMPLATE = subdirs

# names
SUBDIRS = \
amalgamation \
01_server

# directories
amalgamation.subdir = $$PWD/libs/QUaServer.git/src/amalgamation
01_server.subdir    = $$PWD/examples/01_server

# dependencies
01_server.depends   = amalgamation

