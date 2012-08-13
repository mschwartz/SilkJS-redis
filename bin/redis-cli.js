#!/usr/local/bin/silkjs

var Redis = require('Redis').Redis,
    ReadLine = require('ReadLine'),
    console = require('console');

var db;

function main(host) {
    host = host || 'localhost:6379';
    var parts = host.split(':');
    host = parts[0];
    var port = parts[1] || 6379;
    db = new Redis(host, port);
    var stdin = new ReadLine('redis-cli');
    stdin.prompt('redis-cli.js> ');
    while (true) {
        var command;
        try {
            command = stdin.gets();
        }
        catch (e) {
            if (e === 'SIGQUIT' || e === 'SIGTERM') {
                break;
            }
            console.dir(e);
            continue;
        }
        try {
            console.log(print_r(db.command(command)).replace(/\n$/igm, ''));
        }
        catch (e) {
            console.log(e.message);
            // console.dir(e.stack);
        }
    }
}
