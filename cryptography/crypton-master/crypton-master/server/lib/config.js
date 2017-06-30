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

var fs = require('fs');
var path = require('path');

/**!
 * Attempt to load a provided `config.json` file, falling back to the example file
 */

var data;
var configFile;

if (process.configFile) {
  configFile = path.resolve(process.env.PWD, process.configFile);
} else {
  configFile = __dirname + '/../config/config.' + process.env.NODE_ENV + '.json';
}

try {
  var file = fs.readFileSync(configFile).toString();
  data = JSON.parse(file);
} catch (e) {
  var app = require('../app');
  logger.error('could not parse config file');
  throw e;
}

if (process.docker) {
  data.redis.host = process.env.REDIS_PORT_6379_TCP_ADDR;
  data.redis.port = process.env.REDIS_PORT_6379_TCP_PORT;

  data.database.host = process.env.DB_PORT_5432_TCP_ADDR;
  data.database.port = process.env.DB_PORT_5432_TCP_PORT;
}

module.exports = data;
