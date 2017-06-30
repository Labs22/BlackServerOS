#!/usr/bin/env node

/* Crypton Server, Copyright 2013 SpiderOak, Inc.
 *
 * This file is part of Crypton Server.
 *
 * Crypton Server is free software: you can redistribute it and/or modify it
 * under the terms of the Affero GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Crypton Server is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Affero GNU General Public
 * License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with Crypton Server.  If not, see <http://www.gnu.org/licenses/>.
 */

'use strict';

/**!
 * A CLI to manage running and daemonizing the Crypton server
 *
 * crypton-server -h
 *
 */

var fs = require('fs');
var program = require('commander');
var forever = require('forever');
var version = require('../package.json').version;

function getServer(callback) {
  forever.list(false, function(err, data) {
    if (err) {
      console.log(err);
      process.exit(1);
    }

    for (var i in data) {
      if (~data[i].file.indexOf('crypton/server/bin/cli.js')) {
        data[i].index = i;
        return callback(data[i]);
      }
    }

    callback(null);
  });
}

program
  .version(version)
  .option('-c, --config [file]', 'Specify a custom configuration file [default config]')
  .option('-d, --docker', 'Enable Docker mode to use environment variables instead of a config file [default false]');

program.command('run')
  .description('Run the server')
  .action(function() {
    process.configFile = program.config;
    process.docker = program.docker;
    var app = require('../app');
    app.start();
  });

program.command('db:init')
  .description('Initialize the database')
  .action(function() {
    process.configFile = program.config;
    process.docker = program.docker;
    require('./init')();
  });

program.command('db:drop')
  .description('Drop the database')
  .action(function() {
    process.configFile = program.config;
    process.docker = program.docker;
    require('./drop')();
  });

program.command('status')
  .description('Print the status of the daemonized server')
  .action(function() {
    getServer(function(data) {
      if (!data) {
        console.log('No server daemon is running');
      } else {
        console.log(data);
      }
    });
  });

program.command('start')
  .description('Daemonize the server')
  .action(function() {
    getServer(function(data) {
      if (data) {
        console.log('Server daemon already running');
        process.exit(1);
      }

      var monitor = forever.startDaemon(__filename, {
        options: ['run'],
      });

      console.log('Server started');
    });
  });

program.command('stop')
  .description('Stop the daemonized server')
  .action(function() {
    getServer(function(data) {
      if (!data) {
        console.log('Server daemon was not running');
        process.exit(1);
      }

      forever.stop(data.index);
      console.log('Server stopped');
    });
  });

program.command('restart')
  .description('Restart the daemonized server')
  .action(function() {
    getServer(function(data) {
      if (!data) {
        console.log('Server daemon was not running');
        process.exit(1);
      }

      forever.restart(data.index);
      console.log('Server daemon restarted');
    });
  });

program.command('logs')
  .description('Print the latest server logs')
  .action(function() {
    getServer(function(data) {
      if (!data) {
        console.log('Server daemon is not running');
        process.exit(1);
      }

      forever.tail(data.index, {}, function(err, log) {
        console.log(log.line);
      });
    });
  });

program.command('tail')
  .description('Tail the server logs to STDOUT')
  .action(function() {
    getServer(function(data) {
      if (!data) {
        console.log('Server daemon was not running');
        process.exit(1);
      }

      forever.tail(data.index, {
        stream: true,
      }, function(err, log) {
        console.log(log.line);
      });
    });
  });

program.command('cleanlogs')
  .description('Remove the server logs')
  .action(function() {
    forever.cleanUp(true);
  });

program.parse(process.argv);

if (!program.args[0]) {
  program.help();
}
