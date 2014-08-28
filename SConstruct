###
#
# Basic Settings
#
###

env = Environment()



env.Append( CPPPATH=['include'] )
#env.Append( CFLAGS=['-Weverything','-g'] )
#env.Append( CFLAGS=['-Weverything','-Wno-documentation','-g'] )
#env.Append( CFLAGS=['-Wall','-g','--coverage'] )
#env.Append( LINKFLAGS=['-g','--coverage'] )
env.Append( CFLAGS=['-Wall','-Wmissing-field-initializers','-Wmissing-variable-declarations','-g'] )
env.Append( LINKFLAGS=['-g', '-lsqlite3'] )
src = ( Glob('src/anbproto/*.c'), Glob('src/anbutil/*.c') )

# Simple Magic
env.Append( CPPPATH=['../simplemagic/include'] )
env.Append( LINKFLAGS=['-L../simplemagic/', '-lsimplemagic'] )

# SQLite
env.Append( LINKFLAGS=['-g', '-lsqlite3'] )

###
#
# Utils Library
#
###
utils_lib_src = [ Glob('src/anbutil/*.c') ]
utils_lib_env = env.Clone()
utils_lib = utils_lib_env.Library('anbutil',utils_lib_src) 


###
#
# Core Library
#
###
core_lib_src = [ Glob('src/anbproto/*.c') ]
core_lib_env = env.Clone()
core_lib = core_lib_env.Library('anbproto',core_lib_src) 


###
#
# Core Application
#
###
core_app_src = [ core_lib, utils_lib ]
core_app_env = env.Clone()
core_app = core_app_env.Program("anbproto", core_app_src)


###
#
# Tests
#
###

tests_env = env.Clone()
tests_env.Append( CPPPATH=['tests'] )
clar_suite = tests_env.Command('tests/clar.suite', Glob('src/core/*.c'), "python ./tests/clar/generate.py tests")
AlwaysBuild(clar_suite)
clar_obj = tests_env.Object('tests/clar/clar.c')
Depends(clar_obj,File('tests/clar.suite'))

tests_src = [ utils_lib, clar_obj ] + Glob('tests/*.c') + Glob('tests/core/*.c')
tests_env.Program('run_tests', tests_src)
 
