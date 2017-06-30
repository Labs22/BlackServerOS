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

var pg = require('pg').native;
var datastore = require('./');

datastore.util = {};

/**!
 * ### util.camelize(str)
 * Convert from under_scores to camelCase
 *
 * @param {String} str
 * @return {String}
 */
datastore.util.camelize = function (str) {
  return str.replace(/\_(.)/g, function (x, chr) {
     return chr.toUpperCase();
  });
};

/**!
 * ### util.camelizeObject(obj)
 * Camelize every key in an object
 *
 * @param {Object} obj
 * @return {Object}
 */
datastore.util.camelizeObject = function (obj) {
  var newObj = {};

  for (var i in obj) {
    newObj[datastore.util.camelize(i)] = obj[i];
  }

  return newObj;
};

/**!
 * ### connect(callback)
 * Create a connection to the database
 *
 * __Note:__ always call the `done` function when finished
 * with the client to return capacity to the connection pool
 *
 * Calls back with client object and done function without error if successful.
 *
 * __Throws__ if unsuccessful
 *
 * @param {Function} callback
 */
var connect = datastore.connect = function connect(callback) {
  var config = process.app.config.database;

  pg.connect(config, function (err, client, done) {
    if (err) {
      // TODO: retry a few times with delays, so we can survive a quick
      // database hiccup. crash the whole app only if the DB's really
      // unavailable.
      logger.error('Could not connect to database:');
      logger.error(err);
      process.exit(1);
    }

    callback(client, done);
  });
};

/**!
 * ### listTables(callback)
 * List the tables in the configured database
 *
 * Calls back with an array of table names without error if successful.
 *
 * Calls back with error if unsuccessful
 *
 * @param {Function} callback
 */
datastore.listTables = function listTables(callback) {
  connect(function (client) {
    client.query('select * from pg_tables', function (err, result) {
      if (err) {
        return callback(err);
      }

      var tables = [];
      var rows = result.rows.length;

      for (var i = 0; i < rows; i++) {
        tables.push(result.rows[i].tablename);
      }

      callback(null, tables);
    });
  });
};
