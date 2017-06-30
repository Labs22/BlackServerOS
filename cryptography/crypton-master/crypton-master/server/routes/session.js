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

var semver = require('semver');

var app = process.app;

/**!
 * ### GET /
 * Get '/' - just needs to return any kind of data for certain mobile (cordova) uses like testing if a user is still connected
*/
app.get('/', function (req, res) {
  res.send({
    success: true,
    data: { server: 'crypton'}
  });
});

/**!
 * ### GET /versioncheck/
 * Get '/versioncheck/' - see if the current client has the same version as the server
*/
app.get('/versioncheck', function (req, res) {
  logger.info('server version: ' + app.SERVER_VERSION);
  if (!semver.satisfies(app.SERVER_VERSION, '~'+req.query.v)) {
    return res.send({
      success: false,
      version: app.SERVER_VERSION,
      error: 'version mismatch detected',
      data: { server: 'crypton'}
    });
  }

  return res.send({
    success: true,
    version: app.SERVER_VERSION,
    data: { server: 'crypton'}
  });
});
