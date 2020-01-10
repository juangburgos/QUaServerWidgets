TEMPLATE = subdirs

# names
SUBDIRS = \
amalgamation \
01_server \
02_nodetree

# directories
amalgamation.subdir = $$PWD/libs/QUaServer.git/src/amalgamation
01_server.subdir    = $$PWD/examples/01_server
02_nodetree.subdir  = $$PWD/examples/02_nodetree

# dependencies
01_server.depends   = amalgamation
02_nodetree.depends = amalgamation

