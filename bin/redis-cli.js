#!/usr/local/bin/silkjs

var Redis = require('Redis').Redis,
    ReadLine = require('ReadLine'),
    console = require('console');

var db;

function main() {
    db = new Redis();
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
            console.log(print_r(db.command(command)));
        }
        catch (e) {
            console.dir(e);
        }
    }
}
