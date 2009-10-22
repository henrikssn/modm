#!/usr/bin/env python
#
# $Id$

from misc.scons.build_properties import PropertyParser, PropertyException

# create a new commandline-option to specify a property-file
AddOption('--properties',
			dest='properties',
			default='properties.yaml',
			metavar='FILE',
			help='configuration file')

# parse the global property-file
verbose = ARGUMENTS.get('verbose') == '1'
parser = PropertyParser(GetOption('properties'), verbose)

# create a build-environment for the specific target
build = parser.getGlobalProperties()
if build.target == 'avr':
	env = Environment(
			ARCHITECTURE = 'avr',
			AVRDUDE = build.avr.avrdude,
			AVR_DEVICE = build.avr.device,
			AVR_CLOCK = build.avr.clock,
			tools = ['avr', 'doxygen'],
			toolpath = ['misc/scons/'],
			LIBS=[],
			LIBPATH=[])

elif build.target == 'pc':
	env = Environment(
			ARCHITECTURE = 'pc',
			tools = ['pc', 'doxygen'],
			toolpath = ['misc/scons/'],
			LIBS=['boost_thread'],
			LIBPATH=['/usr/lib/'])

else:
	print "Unknown build target '%s'!" % build.target
	Exit(1)

# finally build the library from the src-directory
sourceFiles = parser.parseDirectory('src/', 'library')
library = SConscript('src/SConscript',
			src='src',
			variant_dir='build', 
			exports=['env', 'sourceFiles'], 
			duplicate=False)
env.Alias('lib', 'build/librobot.a')

# build the tests
sourceFiles = parser.parseDirectory('tests/', 'tests')
SConscript('tests/SConscript',
			src='tests',
			variant_dir='build/tests',
			exports=['env', 'sourceFiles', 'library'], 
			duplicate=False)
env.Alias('tests', 'build/tests')


env.Doxygen('doc/doxyfile')
env.Alias('doc', 'apidoc/html')
