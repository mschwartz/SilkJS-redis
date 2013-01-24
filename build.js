#!/usr/local/bin/silkjs

var process = require('builtin/process'),
    fs = require('fs'),
    console = require('console');

var cwd = fs.getcwd();

var env = process.env();
var OSX = env.OS === 'OSX';

var CCFLAGS=[
    '-fPIC',
    '-I/usr/local/silkjs/src',
    '-I/usr/local/silkjs/src/v8/include'
];

var LIBS=[
    'hiredis/libhiredis.a',
    '-L/usr/local/silkjs/src/v8 -lv8'
];

var LDFLAGS = OSX ?
    '-shared -Wl,-install_name,redis_module' :
    '-shared -Wl,-soname,redis_module.so';

function exec(cmd) {
    console.log(cmd);
    process.exec(cmd);
    if (process.exec_result() !== 0) {
        console.log('exiting due to errors');
        process.exit(1);
    }
}

function server() {
    fs.chdir('src');
    if (!fs.exists('redis')) {
        exec('git clone https://github.com/redis/redis.git');
        fs.chdir('redis');
    }
    else {
        fs.chdir('redis');
        exec('git pull');
    }
    exec('make');
    exec('sudo make install');
    if (!OSX) {
        fs.chdir('utils');
        exec('sudo ./install_server.sh');
    }
    else {
        console.log('redis installed in /usr/local/bin.');
    }
    fs.chdir(cwd);
}
function client() {
    fs.chdir('src');
    if (!fs.exists('hiredis')) {
        exec('git clone https://github.com/redis/hiredis.git');
        fs.chdir('hiredis');
    }
    else {
        fs.chdir('hiredis');
        exec('git pull');
    }
    exec('make');
    fs.chdir(cwd);
}
function module() {
    client();
    fs.chdir('src');
    exec('g++ -c ' + CCFLAGS.join(' ') + ' -o redis.o redis.cpp');
    exec('g++ ' + LDFLAGS + ' -o redis_module.so redis.o ' + LIBS.join(' '));
    fs.chdir(cwd);
    exec('cp src/redis_module.so lib');
    exec('mkdir -p /usr/local/silkjs/contrib/Redis');
    exec('cp -rp index.js lib bin /usr/local/silkjs/contrib/Redis');
    exec('ln -sf /usr/local/silkjs/contrib/Redis/bin/redis-cli.js /usr/local/bin');
}

function usage() {
    console.log('Usage:');
    console.log('   ./build.js server');
    console.log('       Build redis server and install it on your system');
    console.log('   ./build.js module');
    console.log('       Build and install SilkJS redis client');
    process.exit(1);
}

function main(rule) {
    if (!rule) {
        usage();
    }
    if (global[rule]) {
        global[rule]();
    }
    else {
        console.log('No rule for ' + rule);
    }
}
