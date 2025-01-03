#!/usr/bin/env python3

import os, sys, shutil, subprocess

global_options = ['--werror']

flavors = {
    'debug': {
        'options': [
        ],
    },

    'asan': {
        'options': [
            '-Db_sanitize=address',
            '-Ddocumentation=false',
        ],
    },

    'release': {
        'options': [
            '--buildtype', 'release',
            '-Db_ndebug=true',
            '-Ddocumentation=false',
        ],
        'env': {
            'CFLAGS': '-ffunction-sections -fdata-sections',
            'CXXFLAGS': '-ffunction-sections -fdata-sections',
            'LDFLAGS': '-fuse-ld=gold -Wl,--gc-sections,--icf=all',
        },
    },

    'lto': {
        'options': [
            '--buildtype', 'release',
            '-Db_ndebug=true',
            '-Db_lto=true',
            '-Ddocumentation=false',
        ],
    },

    'clang': {
        'options': [
            '-Ddocumentation=false',
        ],
        'env': {
            'CC': 'clang',
            'CXX': 'clang++',
        },
    },
}

source_root = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]) or '.', '..'))
output_path = os.path.join(source_root, 'output')
prefix_root = '/usr/local/stow'

for name, data in flavors.items():
    print(name)
    build_root = os.path.join(output_path, name)

    env = os.environ.copy()
    if 'env' in data:
        env.update(data['env'])

    cmdline = [
        'meson', source_root, build_root,
    ] + global_options

    if 'options' in data:
        cmdline.extend(data['options'])

    prefix = os.path.join(prefix_root, 'cm4all-beng-proxy-' + name)

    cmdline += ('--prefix', prefix)

    try:
        shutil.rmtree(build_root)
    except:
        pass

    subprocess.check_call(cmdline, env=env)
